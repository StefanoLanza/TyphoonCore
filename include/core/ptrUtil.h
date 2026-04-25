#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Typhoon {

inline uintptr_t alignPointer(uintptr_t value, size_t alignment) {
	return (value + (alignment - 1)) & (~(alignment - 1));
}

inline void* alignPointer(void* ptr, size_t alignment) {
	return reinterpret_cast<void*>(alignPointer(reinterpret_cast<uintptr_t>(ptr), alignment));
}

inline size_t alignSize(uintptr_t size, size_t alignment) {
	return (size + (alignment - 1)) & (~(alignment - 1));
}

inline void* advancePointer(void* ptr, ptrdiff_t offset) {
	return static_cast<std::byte*>(ptr) + offset;
}

inline bool isAligned(const void* ptr, size_t alignment) {
	return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
}

inline const void* advancePointer(const void* ptr, ptrdiff_t offset) {
	return static_cast<const std::byte*>(ptr) + offset;
}

inline intptr_t pointerDiff(const void* a, const void* b) {
	return static_cast<const std::byte*>(a) - static_cast<const std::byte*>(b);
}

template <typename T>
inline T* advancePointer(T* ptr, ptrdiff_t offset) {
	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + offset);
}

template <typename T>
inline T* decoratePointer(T* ptr, uint32_t epoch) {
	// expect pointers to be 8 bytes aligned. lowest 3 bits must zero
	assert((reinterpret_cast<uintptr_t>(ptr) & 0b111) == 0);
	assert(epoch < 8);
	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) | epoch);
}

template <typename T>
inline T* undecoratePointer(T* ptr) {
	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) & ~0b111);
}

inline uint32_t extractMarkerFromPointer(const void* ptr) {
	return reinterpret_cast<uintptr_t>(ptr) & 0b111;
}

} // namespace Typhoon
