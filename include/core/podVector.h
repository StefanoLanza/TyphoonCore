#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <version>
#include <memory>

#include <core/allocator.h>

namespace Typhoon {

class Allocator;

template <typename T>
#ifdef __cpp_lib_concepts
requires std::is_trivially_copyable_v<T>
#endif

    class podVector {
	static_assert(std::is_trivially_copyable_v<T>, "pod_vector requires trivially copyable T");

public:
	using value_type = T;
	using size_type = std::size_t;
	using iterator = T*;
	using const_iterator = const T*;

	explicit podVector(Allocator& allocator) noexcept
	    : _allocator { &allocator }
	    , _data(nullptr)
	    , _size(0)
	    , _cap(0) {
	}
	explicit podVector(Allocator& allocator, size_t n) noexcept
	    : _allocator { &allocator }
	    , _data(nullptr)
	    , _size(0)
	    , _cap(0) {
		reserve(n);
		_size = n;
		// optionally zero-init POD bytes
		std::memset(_data, 0, _size * sizeof(T));
	}

	~podVector() {
		if (_data) {
			_allocator->free(_data, _cap * sizeof(T));
		}
	}

	// not copyable
	podVector(const podVector&) = delete;
	podVector& operator=(const podVector&) = delete;

#if 0
    pod_vector(const pod_vector& other) {
        _size = other._size;
        _cap = other._size;
        if (_size) {
            _data = static_cast<T*>(std::malloc(_cap * sizeof(T)));
            if (!_data) throw std::bad_alloc();
            std::memcpy(_data, other._data, _size * sizeof(T));
        } else {
            _data = nullptr;
        }
    }

    pod_vector& operator=(const pod_vector& other) {
        if (this == &other) return *this;
        // Stronger strategy: allocate new block first
        T* new_data = nullptr;
        if (other._size) {
            new_data = static_cast<T*>(std::malloc(other._size * sizeof(T)));
            if (!new_data) throw std::bad_alloc();
            std::memcpy(new_data, other._data, other._size * sizeof(T));
        }
        std::free(_data);
        _data = new_data;
        _size = other._size;
        _cap = other._size;
        return *this;
    }
#endif
	podVector(podVector&& o) noexcept
	    : _allocator { o._allocator }
	    , _data(o._data)
	    , _size(o._size)
	    , _cap(o._cap) {
		o._allocator = nullptr;
		o._data = nullptr;
		o._size = 0;
		o._cap = 0;
	}

	podVector& operator=(podVector&& o) noexcept {
		if (this != &o) {
			if (_data) {
				_allocator->free(_data, _cap * sizeof(T));
			}
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

	void push_back(const T& v) {
		if (_size == _cap) {
			grow();
		}
		_data[_size++] = v;
	}

	void pop_back() noexcept {
		if (_size) {
			--_size;
		}
	}

	T& operator[](size_t i) noexcept {
		return _data[i];
	}
	const T& operator[](size_t i) const noexcept {
		return _data[i];
	}

	const T& back() const noexcept {
		assert(_size > 0);
		return _data[_size - 1];
	}

	T* data() noexcept {
		return _data;
	}
	const T* data() const noexcept {
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
		void* p = _allocator->realloc(_data, _cap * sizeof(T), new_cap * sizeof(T), alignof(T));
		assert(p);
		_data = static_cast<T*>(p);
		_cap = new_cap;
	}

	void resize(size_t new_size) noexcept {
		if (new_size > _cap) {
			reserve(grow_to(new_size));
		}
		if (new_size > _size) {
			// zero-init new POD elements
			std::memset(_data + _size, 0, (new_size - _size) * sizeof(T));
		}
		_size = new_size;
	}

	void resize(size_t new_size, const T& value) noexcept {
		if (new_size > _cap) {
			reserve(grow_to(new_size));
		}
		if (new_size > _size) {
			for (size_t i = _size; i < new_size; ++i) {
				_data[i] = value;
			}
		}
		_size = new_size;
	}

	void erase(iterator it) noexcept {
		size_t index = it - _data;
		assert(index < _size);
		if (index + 1 < _size) {
			std::memmove(it, it + 1, (_size - index - 1) * sizeof(T));
		}
		_size--;
	}

	// erase range [first, last)
	void erase(iterator first, iterator last) {
		size_t firstIndex = first - _data;
		size_t lastIndex = last - _data;
		assert(first <= last && lastIndex <= _size);
		size_t count = lastIndex - firstIndex;
		if (count > 0 && lastIndex < _size) {
			std::memmove(first, last, (_size - lastIndex) * sizeof(T));
		}
		_size -= count;
	}

	void clear() noexcept {
		_size = 0;
	}

	template <class _Iter>
	void assign(_Iter first, _Iter last) {
		assert(last > first);
		size_t newSize = std::distance(first, last);
		if (newSize > _cap) {
			reserve(grow_to(newSize));
		}
		std::memcpy(_data, std::to_address(first), newSize * sizeof(T));
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
		size_t new_cap = next_capacity(_cap);
		void*  p = _allocator->realloc(_data, _cap * sizeof(T), new_cap * sizeof(T), alignof(T));
		assert(p);
		_data = static_cast<T*>(p);
		_cap = new_cap;
	}

private:
	Allocator* _allocator;
	T*         _data;
	size_t     _size;
	size_t     _cap;
};

} // namespace Typhoon
