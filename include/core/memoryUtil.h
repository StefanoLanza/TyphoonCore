#pragma once

#include <core/allocator.h>
#include <core/ptrUtil.h>
#include <core/uncopyable.h>
#include <memory>
#include <type_traits>

namespace Typhoon {

class MemoryScope : Uncopyable {
public:
	explicit MemoryScope(LinearAllocator& allocator);
	MemoryScope();
	~MemoryScope();

private:
	LinearAllocator& allocator;
	void*            ptr;
};

void             setGlobalHeapAllocator(Allocator* allocator);
Allocator&       getGlobalHeapAllocator();
void             setGlobalScratchAllocator(LinearAllocator* allocator);
LinearAllocator& getGlobalScratchAllocator();

void* scratchAlloc(size_t size, size_t alignment);
void  scratchFree(void* ptr);

template <class T>
T* scratchAllocArray(size_t elementCount, size_t alignment = std::alignment_of_v<T>) {
	static_assert(std::is_trivially_copyable_v<T>);
	return static_cast<T*>(scratchAlloc(sizeof(T) * elementCount, alignment));
}

template <class T>
T* scratchAlloc() {
	void* mem = scratchAlloc(sizeof(T), std::alignment_of_v<T>);
	return new (mem) T;
}

template <class T>
T* frameAllocArray(size_t elementCount) {
	static_assert(std::is_trivially_copyable_v<T>);
	return static_cast<T*>(frameAlloc(sizeof(T) * elementCount, std::alignment_of_v<T>));
}

inline void* alignAlloc(size_t alignment, size_t size, void*& storage, size_t& space) {
	void* const mem = std::align(alignment, size, storage, space);
	if (mem) {
		storage = advancePointer(storage, size);
		space -= size;
	}
	return mem;
}

} // namespace Typhoon
