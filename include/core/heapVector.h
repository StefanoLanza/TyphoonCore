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
requires((std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>))
#endif

    class HeapVector {
	static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>, "HeapVector requires move or copy constructible types");

public:
	using value_type = T;
	using size_type = std::size_t;
	using iterator = T*;
	using const_iterator = const T*;

	explicit HeapVector(HeapAllocator& allocator)
	    : _allocator { &allocator }
	    , _data(nullptr)
	    , _size(0)
	    , _cap(0) {
	}
	explicit HeapVector(HeapAllocator& allocator, size_t n)
	    : _allocator { &allocator }
	    , _data(nullptr)
	    , _size(0)
	    , _cap(0) {
		resize(n);
	}

	HeapVector(HeapAllocator& allocator, std::initializer_list<T> list) requires(std::is_copy_constructible_v<T>)
	    : _allocator { &allocator }
	    , _data { nullptr }
	    , _size { 0 }
	    , _cap(0) {
		reserve(list.size());
		std::uninitialized_copy(list.begin(), list.end(), _data);
		_size = list.size();
	}

	// not copyable
	HeapVector(const HeapVector&) = delete;
	HeapVector& operator=(const HeapVector&) = delete;

	HeapVector(HeapVector&& o)
	    : _allocator { o._allocator }
	    , _data(o._data)
	    , _size(o._size)
	    , _cap(o._cap) {
		o._data = nullptr;
		o._size = 0;
		o._cap = 0;
	}

	HeapVector& operator=(HeapVector&& o) {
		if (this != &o) {
			assert(_allocator == o._allocator && "Move-assigning between vectors with different allocators");
			clear();
			freeStorage();
			_data = o._data;
			_size = o._size;
			_cap = o._cap;
			o._data = nullptr;
			o._size = 0;
			o._cap = 0;
		}
		return *this;
	}

	~HeapVector() {
		clear();
		freeStorage();
	}

	T& push_back(const T& v) {
		return emplace_back(v);
	}

	T& push_back(T&& v) {
		return emplace_back(std::move(v));
	}

	void pop_back() {
		assert(_size > 0);
		if constexpr (! std::is_trivially_destructible_v<T>) {
			std::destroy_at(_data + _size - 1);
		}
		--_size;
	}

	template <typename... Args>
	T& emplace_back(Args&&... args) {
		if (_size == _cap) {
			reallocate(grow_to(_size + 1));
		}
		T* slot = ::new (_data + _size) T(std::forward<Args>(args)...);
		++_size;
		return *slot;
	}

	T& operator[](size_t i) {
		assert(i < _size);
		return _data[i];
	}
	const T& operator[](size_t i) const {
		assert(i < _size);
		return _data[i];
	}

	T& front() {
		assert(_size > 0);
		return _data[0];
	}
	const T& front() const {
		assert(_size > 0);
		return _data[0];
	}

	T& back() {
		assert(_size > 0);
		return _data[_size - 1];
	}

	const T& back() const {
		assert(_size > 0);
		return _data[_size - 1];
	}

	T* data() {
		return _data;
	}
	const T* data() const {
		return _data;
	}
	size_t size() const {
		return _size;
	}
	size_t capacity() const {
		return _cap;
	}

	bool empty() const {
		return _size == 0;
	}

	void reserve(size_t new_cap) {
		if (new_cap <= _cap) {
			return;
		}
		reallocate(new_cap);
	}

	void resize(size_t new_size) {
		if (new_size > _cap) {
			reserve(grow_to(new_size));
		}
		if (new_size > _size) {
			std::uninitialized_value_construct(_data + _size, _data + new_size);
		}
		else if (new_size < _size) {
			std::destroy(_data + new_size, _data + _size);
		}
		_size = new_size;
	}

	void resize(size_t new_size, const T& value) {
		if (new_size > _cap) {
			reserve(grow_to(new_size));
		}
		if (new_size > _size) {
			std::uninitialized_fill(_data + _size, _data + new_size, value);
		}
		else if (new_size < _size) {
			std::destroy(_data + new_size, _data + _size);
		}
		_size = new_size;
	}

	void erase(iterator it) {
		erase(it, it + 1);
	}

	// erase range [first, last)
	void erase(iterator first, iterator last) {
		assert(first <= last && first >= _data && last <= _data + _size);

		const size_t count = static_cast<size_t>(last - first);
		if (count == 0) {
			return;
		}

		iterator dst = first;
		iterator src = last;
		iterator end_it = _data + _size;

		if constexpr (std::is_trivially_copyable_v<T>) {
			std::memmove(dst, src, static_cast<size_t>(end_it - src) * sizeof(T));
		}
		else {
			for (; src != end_it; ++dst, ++src) {
				if constexpr (std::is_move_assignable_v<T>) {
					*dst = std::move(*src);
				}
				else if constexpr (std::is_copy_assignable_v<T>) {
					*dst = *src;
				}
				else {
					std::destroy_at(dst);
					::new (dst) T(std::move(*src));
				}
			}
			std::destroy(end_it - count, end_it);
		}

		_size -= count;
	}

	void clear() {
		std::destroy(_data, _data + _size);
		_size = 0;
	}

	template <class _Iter>
	void assign(_Iter first, _Iter last) {
		assert(last >= first);
		clear();
		const size_t newSize = static_cast<size_t>(std::distance(first, last));
		if (newSize > _cap) {
			reserve(grow_to(newSize));
		}
		std::uninitialized_copy(first, last, _data);
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
	void freeStorage() {
		if (_data) {
			_allocator->free(_data, _cap * sizeof(T));
			_data = nullptr;
			_cap = 0;
		}
	}

	size_t grow_to(size_t min_needed) const {
		size_t n = (_cap ? _cap * 2 : 1);
		while (n < min_needed) {
			n *= 2;
		}
		return n;
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
			std::uninitialized_move_n(_data, _size, new_data);
			std::destroy(_data, _data + _size);
			freeStorage();
			_data = new_data;
		}

		_cap = new_cap;
	}

private:
	HeapAllocator* _allocator;
	T*             _data;
	size_t         _size;
	size_t         _cap;
};

template <class T, class Pred>
typename HeapVector<T>::size_type erase_if(HeapVector<T>& c, Pred pred) {
	auto it = std::remove_if(c.begin(), c.end(), pred);
	auto count = static_cast<typename HeapVector<T>::size_type>(std::distance(it, c.end()));
	c.erase(it, c.end());
	return count;
}

} // namespace Typhoon
