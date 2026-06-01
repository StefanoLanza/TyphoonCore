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
class FixedArray {
public:
	using value_type = T;
	using size_type = std::size_t;
	using iterator = T*;
	using const_iterator = const T*;

	FixedArray(Allocator& allocator, std::initializer_list<T> list) 
		requires(std::is_copy_constructible_v<T>)
		: _allocator { &allocator }
		, _data { nullptr }
		, _size { 0 } {
		allocate(list.size());
		std::uninitialized_copy(list.begin(), list.end(), _data);
	}

	FixedArray(Allocator& allocator, size_t n) noexcept
		requires(std::is_default_constructible_v<T>)
	    : _allocator { &allocator }
	    , _data { nullptr }
	    , _size { 0 } {
		allocate(n);
		std::uninitialized_value_construct(_data, _data + n);
	}

	template <typename U>
	FixedArray(Allocator& allocator, size_t n, const U& value) noexcept
		requires(std::is_copy_constructible_v<T> && std::is_constructible_v<T, U>)
	    : _allocator { &allocator }
	    , _data { nullptr }
	    , _size { 0 } {
		allocate(n);
		std::uninitialized_fill(_data, _data + _size, value);
	}

	// not copyable
	FixedArray(const FixedArray&) = delete;
	FixedArray& operator=(const FixedArray&) = delete;

	FixedArray(FixedArray&& o) noexcept
	    : _allocator { o._allocator }
	    , _data(o._data)
	    , _size(o._size) {
		o._data = nullptr;
		o._size = 0;
#ifdef _DEBUG
		_epoch = o._epoch;
		o._epoch = 0;
#endif
	}

	FixedArray& operator=(FixedArray&& o) {
		if (this != &o) {
			assert(_allocator == o._allocator && "Move-assigning between arrays with different allocators");
			freeData();
			_data = o._data;
			_size = o._size;
			o._data = nullptr;
			o._size = 0;
#ifdef _DEBUG
			_epoch = o._epoch;
			o._epoch = 0;
#endif
		}
		return *this;
	}

	~FixedArray() {
		freeData();
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

	[[nodiscard]] T* data() {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	[[nodiscard]] const T* data() const {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	[[nodiscard]] size_t size() const {
		return _size;
	}
	[[nodiscard]] bool empty() const {
		return _size == 0;
	}

	// ---- iterators ----
	iterator begin() {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	iterator end() {
#ifdef _DEBUG
		debug();
#endif
		return _data + _size;
	}
	const_iterator begin() const {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	const_iterator end() const {
#ifdef _DEBUG
		debug();
#endif
		return _data + _size;
	}
	const_iterator cbegin() const {
#ifdef _DEBUG
		debug();
#endif
		return _data;
	}
	const_iterator cend() const {
#ifdef _DEBUG
		debug();
#endif
		return _data + _size;
	}

private:
	void freeData() {
#ifdef _DEBUG
		debug();
#endif
		if (_data) {
			std::destroy(_data, _data + _size);
			_allocator->free(_data, _size * sizeof(T));
			_data = nullptr;
			_size = 0;
		}
	}

	void allocate(size_t size) {
		void* raw = _allocator->alloc(size * sizeof(T), alignof(T));
		assert(raw);
		_data = static_cast<T*>(raw);
		_size = size;
#ifdef _DEBUG
		_epoch = _allocator->getEpoch();
#endif
	}

#ifdef _DEBUG
	void debug() const {
		if (_data) {
			_allocator->check(_data, _epoch);
		}
	}
#endif

private:
	Allocator* _allocator;
	T*         _data;
	size_t     _size;
#ifdef _DEBUG
	uint32_t _epoch = 0;
#endif
};

} // namespace Typhoon
