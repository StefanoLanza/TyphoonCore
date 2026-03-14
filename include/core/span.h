#pragma once

#include <array>
#include <cassert>
#include <cstddef> // size_t
#include <vector>

namespace Typhoon {

// Temporary replacement for std::span in C++ 20
template <class T>
class span {
public:
	using size_type = std::size_t;

	span()
	    : data_(nullptr)
	    , size_(0) {
	}
	template <class It>
	span(It first, It last)
	    : data_(first == last ? nullptr : &*first)
	    , size_(last - first) {
	}
	template <class I>
	span(T* data, I size)
	    : data_(data)
	    , size_(static_cast<size_t>(size)) {
	}
	template <class U, std::size_t N>
	span(const std::array<U, N>& array)
	    : data_(array.data())
	    , size_(array.size()) {
	}
	template <class U, std::size_t N>
	span(const U (&array)[N])
	    : data_(array)
	    , size_(N) {
	}
	span(std::initializer_list<T> il)
	    : data_(il.begin())
	    , size_(il.size()) {
	}
	span(T* begin, T* end)
	    : data_(begin)
	    , size_(end - begin) {
		assert(begin);
		assert(end >= begin);
	}
	template <class U>
	span(const std::vector<U>& v)
	    : data_(v.data())
	    , size_(v.size()) {
	}

	// Iterators
	T* begin() const noexcept {
		return data_;
	}
	T* end() const noexcept {
		return data_ + size_;
	}

	// Element access
	T& front() const noexcept {
		assert(size_);
		return data_[0];
	}
	T& back() const noexcept {
		assert(size_);
		return data_[size_ - 1];
	}
	T& operator[](size_t i) const noexcept {
		return data_[i];
	}
	T* data() const {
		return data_;
	}

	// Observers
	size_type size() const {
		return size_;
	}
	size_type size_bytes() const {
		return size_ * sizeof(T);
	}
	bool empty() const {
		return size_ == 0;
	}

public:
	T*     data_;
	size_t size_;
};

} // namespace Typhoon
