#include "memoryUtil.h"
#include <cassert>

namespace Typhoon {

namespace {

Allocator*       gHeapAllocator = nullptr;
LinearAllocator* gScratchAllocator = nullptr;

} // namespace

void setGlobalHeapAllocator(Allocator* allocator) {
	gHeapAllocator = allocator;
}

Allocator& getGlobalHeapAllocator() {
	return *gHeapAllocator;
}

void setGlobalScratchAllocator(LinearAllocator* allocator) {
	gScratchAllocator = allocator;
}

LinearAllocator& getGlobalScratchAllocator() {
	return *gScratchAllocator;
}

void* scratchAlloc(size_t size, size_t alignment) {
	assert(gScratchAllocator);
	return gScratchAllocator->alloc(size, alignment);
}

void scratchFree(void* ptr) {
	if (ptr) {
		assert(gScratchAllocator);
		gScratchAllocator->rewind(ptr);
	}
}

MemoryScope::MemoryScope(LinearAllocator& allocator)
    : allocator(allocator)
    , ptr(allocator.getOffset()) {
}

MemoryScope::MemoryScope()
    : allocator(getGlobalScratchAllocator())
    , ptr(allocator.getOffset()) {
}

MemoryScope::~MemoryScope() {
	allocator.rewind(ptr);
}

} // namespace Typhoon
