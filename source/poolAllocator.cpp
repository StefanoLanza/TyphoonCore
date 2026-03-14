#include "poolAllocator.h"
#include <cassert>
#include <core/allocator.h>
#include <core/ptrUtil.h>
#include <cstdint>
#include <cstring>

namespace Typhoon {

struct BasePoolAllocator::FreeSlot { // FIXME as uint32_t offset
	FreeSlot* next;
};

struct BasePoolAllocator::Page {
	Page*     next;
	void*     buffer;
	FreeSlot* nextFreeSlot;
	size_t    allocatedSize;
};

BasePoolAllocator::BasePoolAllocator(Allocator& baseAllocator, size_t capacity, size_t elementSize, size_t alignment)
    : baseAllocator(baseAllocator)
    , pageSize(capacity)
    , rootPage(nullptr)
    , elementSize(elementSize)
    , alignment(alignment) {
	assert(sizeof(FreeSlot) <= elementSize);
	assert(elementSize + sizeof(FreeSlot) <= pageSize);
}

BasePoolAllocator::~BasePoolAllocator() {
	for (const Page* page = rootPage; page;) {
		Page* next = page->next; // Fetch before freeing page
		baseAllocator.free(page->buffer, pageSize);
		page = next;
	}
}

void* BasePoolAllocator::alloc() {
	// Try allocating element from one of the existing pages
	for (Page* page = rootPage; page; page = page->next) {
		if (void* ptr = allocFromPage(*page); ptr) {
			return ptr;
		}
	}
	// Allocate new page
	Page* newPage = allocPage();
	// Prepend new page to  linked list
	newPage->next = rootPage;
	rootPage = newPage;
	// Allocate element from new page
	return allocFromPage(*newPage);
}

void BasePoolAllocator::free(void* ptr) {
	if (! ptr) {
		return;
	}
	for (Page* page = this->rootPage; page; page = page->next) {
		if (ptr >= page->buffer && ptr < (static_cast<char*>(page->buffer) + pageSize)) {
			assert(isAligned(ptr, this->alignment));
			assert(page->allocatedSize >= this->elementSize);
			// put the returned element at the head of the free list
			FreeSlot* head = static_cast<FreeSlot*>(ptr);
			head->next = page->nextFreeSlot;
			page->nextFreeSlot = head;
			page->allocatedSize -= elementSize;
		}
	}
}

void* BasePoolAllocator::allocFromPage(Page& page) const {
	if (page.nextFreeSlot) {
		page.allocatedSize += this->elementSize;
		// Obtain one element from the head of the free list
		FreeSlot* const head = page.nextFreeSlot;
		page.nextFreeSlot = head->next;
		assert(isAligned(head, this->alignment));
		return head;
	}
	return nullptr;
}

#if 0
size_t BasePoolAllocator::getIndex(const void* ptr) const {
	size_t       index = 0;
	const size_t elmPerPage = (pageSize - alignSize(sizeof(Page), alignment)) / elementSize;
	for (const Page* page = rootPage; page; page = page->next) {
		if (ptr >= page->buffer && ptr < (static_cast<char*>(page->buffer) + pageSize)) {
			uintptr_t pageOffs = reinterpret_cast<uintptr_t>(page->buffer) + alignSize(sizeof(Page), alignment);
			index = (reinterpret_cast<uintptr_t>(ptr) - pageOffs) / elementSize;
			break;
		}
		index += elmPerPage;
	}
	return index; // FIXME page too
}

void* BasePoolAllocator::getPtr(size_t index) const {
	const size_t elmPerPage = (pageSize - alignSize(sizeof(Page), alignment)) / elementSize;
	for (const Page* page = rootPage; page; page = page->next) {
		if (index < elmPerPage) {
			return static_cast<char*>(page->buffer) + alignSize(sizeof(Page), alignment) + elementSize * index; // FIXME header
		}
		index -= elmPerPage;
	}
	assert(false);
	return nullptr;
}
#endif

void BasePoolAllocator::clear() {
	for (Page* page = rootPage; page; page = page->next) {
		freePage(*page); // FIXME do not reset linked list
	}
}

void BasePoolAllocator::freePage(Page& page) {
	union {
		char*     as_char;
		FreeSlot* as_self;
	};

	page.nextFreeSlot = static_cast<FreeSlot*>(advancePointer(page.buffer, alignSize(sizeof(Page), alignment)));

	as_self = page.nextFreeSlot;
	as_char += elementSize;

	// Initialize the free list - make every next of each element point to the next element in the list
	// TODO Make this faster
	const size_t elmPerPage = (pageSize - alignSize(sizeof(Page), alignment)) / elementSize;
	FreeSlot*    runner = page.nextFreeSlot;
	for (size_t i = 1; i < elmPerPage; ++i) {
		runner->next = as_self;
		runner = as_self;
		as_char += elementSize;
	}

	runner->next = nullptr;
	page.allocatedSize = 0;
}

BasePoolAllocator::Page* BasePoolAllocator::allocPage() {
	void* buffer = baseAllocator.alloc(pageSize, Allocator::defaultAlignment);
	if (buffer) {
		// Allocate page at start of buffer
		Page newPage {};
		newPage.next = nullptr;
		newPage.buffer = buffer;
		freePage(newPage);
		std::memcpy(buffer, &newPage, sizeof newPage);
	}
	return static_cast<Page*>(buffer);
}

} // namespace Typhoon
