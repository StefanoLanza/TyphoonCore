#include "poolAllocator.h"

#include <core/allocator.h>
#include <core/ptrUtil.h>

#include <cassert>
#include <cstdint>

namespace Typhoon {

struct BasePoolAllocator::FreeSlot {
	FreeSlot* next;
};

BasePoolAllocator::BasePoolAllocator(Allocator& backingAllocator, size_t maxElements, size_t elementSize, size_t alignment)
    : backingAllocator(backingAllocator)
    , maxElements(maxElements)
    , elementSize(elementSize)
    , alignment(alignment)
    , buffer { backingAllocator.alloc(maxElements * elementSize, alignment) }
    , nextFreeSlot { nullptr } {
	assert(elementSize >= sizeof(FreeSlot));
	assert(elementSize % alignment == 0);
#ifdef _DEBUG
	epoch = backingAllocator.getEpoch();
#endif
	init();
}

BasePoolAllocator::~BasePoolAllocator() {
#ifdef _DEBUG
	debug();
#endif
	backingAllocator.free(buffer, maxElements * elementSize);
}

void* BasePoolAllocator::alloc() {
#ifdef _DEBUG
	debug();
#endif
	if (nextFreeSlot) {
		// Obtain one element from the head of the free list
		FreeSlot* const head = nextFreeSlot;
		nextFreeSlot = head->next;
		assert(isAligned(head, alignment));
		return head;
	}
	return nullptr;
}

void BasePoolAllocator::free(void* ptr) {
#ifdef _DEBUG
	debug();
#endif
	assert(ptr >= buffer && ptr < (static_cast<char*>(buffer) + maxElements * elementSize));
	assert(isAligned(ptr, alignment));
	// put the returned element at the head of the free list
	FreeSlot* head = static_cast<FreeSlot*>(ptr);
	head->next = nextFreeSlot;
	nextFreeSlot = head;
}

void BasePoolAllocator::clear() {
#ifdef _DEBUG
	debug();
#endif
	init();
}

void BasePoolAllocator::init() {
	assert(buffer);
	// Initialize the free list
	nextFreeSlot = static_cast<FreeSlot*>(buffer);
	union {
		char*     as_char;
		FreeSlot* as_self;
	};
	as_self = nextFreeSlot;
	as_char += elementSize;
	FreeSlot* runner = nextFreeSlot;
	for (size_t i = 1; i < maxElements; ++i) {
		runner->next = as_self;
		runner = as_self;
		as_char += elementSize;
	}
	runner->next = nullptr;
}

#ifdef _DEBUG
void BasePoolAllocator::debug() {
	backingAllocator.check(buffer, epoch);
}
#endif

} // namespace Typhoon
