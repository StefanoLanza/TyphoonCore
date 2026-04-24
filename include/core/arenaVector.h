#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>
#include <version>

#include <core/allocator.h>

namespace Typhoon {

template <typename T>
#ifdef __cpp_lib_concepts
requires(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
#endif

    class ArenaVector {
	static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>, "ArenaVector requires T to be move- or copy-constructible");

public:
	using value_type = T;
	using size_type = std::size_t;
	using iterator = T*;
	using const_iterator = const T*;

	explicit ArenaVector(LinearAllocator& allocator) noexcept
	    : _allocator { &allocator }
	    , _data(nullptr)
	    , _size(0)
	    , _cap(0) {
	}
	explicit ArenaVector(LinearAllocator& allocator, size_t n) noexcept
	    : _allocator { &allocator }
	    , _data(nullptr)
	    , _size(0)
	    , _cap(0) {
		reserve(n);
		if constexpr (std::is_trivially_constructible_v<T>) {
			std::memset(_data, 0, n * sizeof(T));
		}
		else {
			std::uninitialized_default_construct(_data, _data + n);
		}
		_size = n;
	}

	// not copyable
	ArenaVector(const ArenaVector&) = delete;
	ArenaVector& operator=(const ArenaVector&) = delete;

	ArenaVector(ArenaVector&& o) noexcept
	    : _allocator { o._allocator }
	    , _data(o._data)
	    , _size(o._size)
	    , _cap(o._cap) {
		o._allocator = nullptr;
		o._data = nullptr;
		o._size = 0;
		o._cap = 0;
	}

	ArenaVector& operator=(ArenaVector&& o) noexcept {
		if (this != &o) {
			_allocator = o._allocator;
			_data = o._data;
			_size = o._size;
			_cap = o._cap;
			o._allocator = nullptr;
			o._data = nullptr;
			o._size = 0;
			o._cap = 0;
		}
		return *this;
	}

	~ArenaVector() {
		destroyAll();
	}

	T& push_back(const T& v) {
		if (_size == _cap) {
			grow();
		}
#ifdef _DEBUG
		debug();
#endif
		T* slot;
		if constexpr (std::is_trivially_constructible_v<T>) {
			_data[_size] = v;
			slot = &_data[_size];
		}
		else {
			slot = ::new (_data + _size) T(v);
		}
		++_size;
		return *slot;
	}

	T& push_back(T&& v) noexcept {
		return emplace_back(std::move(v));
	}

	void pop_back() noexcept {
		if (_size) {
			if constexpr (! std::is_trivially_destructible_v<T>) {
				_data[_size - 1].~T();
			}
			--_size;
		}
	}

	template <typename... Args>
	T& emplace_back(Args&&... args) noexcept {
		if (_size == _cap) {
			grow();
		}
#ifdef _DEBUG
		debug();
#endif
		T* slot = ::new (_data + _size) T(std::forward<Args>(args)...);
		++_size;
		return *slot;
	}

	T& operator[](size_t i) noexcept {
#ifdef _DEBUG
		debug();
#endif
		return _data[i];
	}
	const T& operator[](size_t i) const noexcept {
#ifdef _DEBUG
		debug();
#endif
		return _data[i];
	}

	T& front() noexcept {
#ifdef _DEBUG
		debug();
#endif
		assert(_size);
		return _data[0];
	}
	const T& front() const noexcept {
#ifdef _DEBUG
		debug();
#endif
		assert(_size);
		return _data[0];
	}

	T& back() noexcept {
#ifdef _DEBUG
		debug();
#endif
		assert(_size > 0);
		return _data[_size - 1];
	}

	const T& back() const noexcept {
#ifdef _DEBUG
		debug();
#endif
		assert(_size > 0);
		return _data[_size - 1];
	}

	T* data() noexcept {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	const T* data() const noexcept {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	size_t size() const noexcept {
		return _size;
	}
	size_t capacity() const noexcept {
		return _cap;
	}

	bool empty() const noexcept {
		return _size == 0;
	}

	void reserve(size_t new_cap) noexcept {
		if (new_cap <= _cap) {
			return;
		}
		reallocate(new_cap);
	}

	void resize(size_t new_size) noexcept {
		if (new_size > _cap) {
			reserve(grow_to(new_size));
		}
#ifdef _DEBUG
		debug();
#endif
		if (new_size > _size) {
			if constexpr (std::is_trivially_constructible_v<T>) {
				// zero-init new POD elements
				std::memset(_data + _size, 0, (new_size - _size) * sizeof(T));
			}
			else {
				std::uninitialized_default_construct(_data + _size, _data + new_size);
			}
		}
		else if (new_size < _size) {
			if constexpr (! std::is_trivially_destructible_v<T>) {
				for (size_t i = new_size; i < _size; ++i) {
					_data[i].~T();
				}
			}
		}
		_size = new_size;
	}

	void resize(size_t new_size, const T& value) noexcept {
		if (new_size > _cap) {
			reserve(grow_to(new_size));
		}
#ifdef _DEBUG
		debug();
#endif
		if (new_size > _size) {
			if constexpr (std::is_trivially_copyable_v<T>) {
				std::fill(_data + _size, _data + new_size, value);
			}
			else {
				for (size_t i = _size; i < new_size; ++i) {
					_data[i] = value;
				}
			}
		}
		else if (new_size < _size) {
			if constexpr (! std::is_trivially_destructible_v<T>) {
				for (size_t i = new_size; i < _size; ++i) {
					_data[i].~T();
				}
			}
		}
		_size = new_size;
	}

	void erase(iterator it) noexcept {
		assert(it >= _data && it < _data + _size);
#ifdef _DEBUG
		debug();
#endif
		if constexpr (! std::is_trivially_destructible_v<T>) {
			it->~T();
		}
		shiftLeft(it + 1, _data + _size, it);
		_size--;
	}

	// erase range [first, last)
	void erase(iterator first, iterator last) {
		assert(first <= last && first >= _data && last <= _data + _size);
		const size_t count = static_cast<size_t>(last - first);
		if (count == 0) {
			return;
		}
#ifdef _DEBUG
		debug();
#endif
		if constexpr (! std::is_trivially_destructible_v<T>) {
			for (iterator it = first; it != last; ++it) {
				it->~T();
			}
		}
		shiftLeft(last, _data + _size, first);
		_size -= count;
	}

	void clear() {
		destroyAll();
		_data = nullptr;
		_size = 0;
		_cap = 0;
#ifdef _DEBUG
		_epoch = 0;
#endif
	}

	template <class _Iter>
	void assign(_Iter first, _Iter last) {
		assert(last > first);
		const size_t newSize = std::distance(first, last);
		if constexpr (! std::is_trivially_destructible_v<T>) {
			for (size_t i = 0; i < _size; ++i) {
				_data[i].~T();
			}
		}
		if (newSize > _cap) {
			reserve(grow_to(newSize));
		}
		if constexpr (std::is_trivially_copyable_v<T>) {
			std::memcpy(_data, std::to_address(first), newSize * sizeof(T));
		}
		else {
			for (size_t i = 0; i < newSize; ++i, ++first) {
				::new (_data + i) T(*first);
			}
		}
		_size = newSize;
	}

	// ---- iterators ----
	iterator begin() {
		return _data;
	}
	iterator end() {
		return _data + _size;
	}
	const_iterator begin() const {
		return _data;
	}
	const_iterator end() const {
		return _data + _size;
	}
	const_iterator cbegin() const {
		return _data;
	}
	const_iterator cend() const {
		return _data + _size;
	}

private:
	static size_t next_capacity(size_t c) {
		return c ? c * 2 : 1;
	}

	size_t grow_to(size_t min_needed) const {
		size_t n = (_cap ? _cap * 2 : 1);
		while (n < min_needed) {
			n *= 2;
		}
		return n;
	}

	void grow() {
		reallocate(next_capacity(_cap));
	}

	void reallocate(size_t new_cap) {
		if constexpr (std::is_trivially_copyable_v<T>) {
			void* p = _allocator->realloc(_data, _cap * sizeof(T), new_cap * sizeof(T), alignof(T));
			assert(p);
			_data = static_cast<T*>(p);
		}
		else {
			// Non-trivial: must allocate fresh and move. realloc is unsafe
			// since it may memcpy objects that own resources
			void* raw = _allocator->alloc(new_cap * sizeof(T), alignof(T));
			assert(raw);
			T* new_data = static_cast<T*>(raw);
			for (size_t i = 0; i < _size; ++i) {
				::new (new_data + i) T(std::move(_data[i]));
				_data[i].~T();
			}
			_data = new_data; // old block returns to arena on arena reset
		}

		_cap = new_cap;
#ifdef _DEBUG
		_epoch = _allocator->getEpoch();
#endif
	}
	void destroyAll() {
#ifdef _DEBUG
		debug();
#endif
		if constexpr (! std::is_trivially_destructible_v<T>) {
			for (size_t i = 0; i < _size; ++i) {
				_data[i].~T();
			}
		}
	}

	// Move [src_first, src_last) to [dst, ...) � handles overlapping ranges
	void shiftLeft(iterator src_first, iterator src_last, iterator dst) noexcept {
		if constexpr (std::is_trivially_copyable_v<T>) {
			std::memmove(dst, src_first, static_cast<size_t>(src_last - src_first) * sizeof(T));
		}
		else {
			while (src_first != src_last) {
				// Move-assign if possible, otherwise copy-assign
				if constexpr (std::is_nothrow_move_assignable_v<T>)
					*dst = std::move(*src_first);
				else
					*dst = *src_first;
				src_first->~T(); // destroy the now-vacated slot
				++dst;
				++src_first;
			}
		}
	}

#ifdef _DEBUG
	void debug() const {
		assert(_data == nullptr || (_epoch == _allocator->getEpoch()));
	}
#endif

private:
	LinearAllocator* _allocator;
	T*               _data;
	size_t           _size;
	size_t           _cap;
#ifdef _DEBUG
	uint32_t _epoch = 0;
#endif
};

} // namespace Typhoon
