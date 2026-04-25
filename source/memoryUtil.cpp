#include "memoryUtil.h"
#include <cassert>

namespace Typhoon {

namespace {

Allocator*       gHeapAllocator = nullptr;
ArenaAllocator* gScratchAllocator = nullptr;

} // namespace

void setGlobalHeapAllocator(Allocator* allocator) {
	gHeapAllocator = allocator;
}

Allocator& getGlobalHeapAllocator() {
	return *gHeapAllocator;
}

void setGlobalScratchAllocator(ArenaAllocator* allocator) {
	gScratchAllocator = allocator;
}

ArenaAllocator& getGlobalScratchAllocator() {
	return *gScratchAllocator;
}

void* scratchAlloc(size_t size, size_t alignment) {
	assert(gScratchAllocator);
	return gScratchAllocator->alloc(size, alignment);
}

void scratchFree(void* ptr) {
	if (ptr) {
		assert(gScratchAllocator);
		gScratchAllocator->reset(ptr);
	}
}

MemoryScope::MemoryScope(ArenaAllocator& allocator)
    : allocator(allocator)
    , ptr(allocator.getOffset()) {
}

MemoryScope::MemoryScope()
    : allocator(getGlobalScratchAllocator())
    , ptr(allocator.getOffset()) {
}

MemoryScope::~MemoryScope() {
	allocator.reset(ptr);
}

} // namespace Typhoon
