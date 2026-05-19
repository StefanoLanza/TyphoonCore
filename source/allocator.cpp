#include "allocator.h"
#include "ptrUtil.h"

#include <cassert>
#include <cstdlib>
#include <memory>

namespace Typhoon {

void* C_Allocator::alloc(size_t size, [[maybe_unused]] size_t alignment) {
#ifdef _MSC_VER
	return _aligned_malloc(size, alignment);
#else
	return ::malloc(size);
#endif
}

void C_Allocator::free(void* ptr, [[maybe_unused]] size_t size) {
#ifdef _MSC_VER
	::_aligned_free(ptr);
#else
	::free(ptr);
#endif
}

void* C_Allocator::realloc(void* ptr, [[maybe_unused]] size_t currSize, size_t newSize, [[maybe_unused]] size_t alignment) {
#ifdef _MSC_VER
	return _aligned_realloc(ptr, newSize, alignment);
#else
	return ::realloc(ptr, newSize);
#endif
}

BufferAllocator::BufferAllocator(void* buffer, size_t bufferSize)
    : buffer(buffer)
    , backingAllocator(nullptr)
    , curr(buffer)
    , bufferSize(bufferSize)
    , lastAlloc { nullptr }
    , epoch(0) {
}

BufferAllocator::BufferAllocator(HeapAllocator& backingAllocator, size_t bufferSize)
    : buffer(backingAllocator.alloc(bufferSize, backingAllocator.defaultAlignment))
    , backingAllocator(&backingAllocator)
    , curr(buffer)
    , bufferSize(bufferSize)
    , lastAlloc { nullptr }
    , epoch(0) {
}

BufferAllocator::~BufferAllocator() {
	backingAllocator->free(buffer, bufferSize);
}

void* BufferAllocator::alloc(size_t size, size_t alignment) {
	size_t freeSize = pointerDiffU(buffer, curr) + bufferSize;
	void*  result = std::align(alignment, size, curr, freeSize);
	if (result) {
		curr = advancePointer(result, size);
		lastAlloc = result;
	}
	return result;
}

void* BufferAllocator::realloc(void* ptr, size_t currSize, size_t newSize, size_t alignment) {
	if (ptr && lastAlloc == ptr) {
		// Extend last allocation
		assert(isAligned(ptr, alignment));
		if (reinterpret_cast<uintptr_t>(ptr) + newSize > reinterpret_cast<uintptr_t>(buffer) + bufferSize) {
			return nullptr; // out of memory
		}
		curr = advancePointer(ptr, newSize);
		return ptr;
	}
	else {
		void* res = alloc(newSize, alignment);
		if (! res) {
			return nullptr;
		}
		if (ptr) {
			std::memcpy(res, ptr, currSize);
		}
		return res;
	}
}

void BufferAllocator::reset() {
	curr = buffer;
	lastAlloc = nullptr;
	++epoch;
}

void BufferAllocator::reset(void* offs) {
	assert(isPointerInRange(offs, buffer, curr));
	curr = offs;
	if (lastAlloc != offs) {
		lastAlloc = nullptr;
	}
	//++epoch;
}

void* BufferAllocator::getOffset() const {
	return curr;
}

uint32_t BufferAllocator::getEpoch() const {
	return epoch;
}

void* BufferAllocator::getBuffer() const {
	return buffer;
}

PagedAllocator::PagedAllocator(HeapAllocator& backingAllocator, size_t pageSize)
    : allocator(&backingAllocator)
    , pageSize(pageSize)
    , rootPage(nullptr)
    , currPage(nullptr)
    , lastAllocation(nullptr)
    , epoch(0) {
	assert(pageSize > sizeof(Page));
}

PagedAllocator::~PagedAllocator() {
	for (Page* page = rootPage; page;) {
		Page* next = page->next; // fetch before freeing page
		allocator->free(page, pageSize);
		page = next;
	}
}

void* PagedAllocator::alloc(size_t size, size_t alignment) {
	if (size > pageSize - sizeof(Page)) {
		return nullptr; // can never be satisfied
	}

	for (Page* page = currPage; page != nullptr; page = page->next) {
		currPage = page;
		if (void* result = allocFromPage(*currPage, size, alignment); result) {
			lastAllocation = result;
			return result;
		}
	}

	Page* newPage = allocPage();
	if (! rootPage) {
		rootPage = newPage;
	}
	newPage->prev = currPage;
	if (currPage) {
		currPage->next = newPage;
	}
	currPage = newPage;
	void* result = allocFromPage(*newPage, size, alignment);
	if (result) {
		lastAllocation = result;
	}
	return result;
}

void* PagedAllocator::realloc(void* ptr, size_t currSize, size_t newSize, size_t alignment) {
	if (ptr && lastAllocation == ptr) {
		assert(currPage);
		assert(isAligned(ptr, alignment));
		size_t freeSize = (pageSize - sizeof(Page)) - pointerDiffU(currPage->buffer, ptr);
		if (freeSize >= newSize) {
			currPage->offset = advancePointer(ptr, newSize);
			return ptr;
		}
	}

	// in-place realloc failed

	void* res = alloc(newSize, alignment);
	if (ptr) {
		std::memcpy(res, ptr, currSize);
	}

	return res;
}

void PagedAllocator::reset() {
	for (Page* page = rootPage; page; page = page->next) {
		page->offset = page->buffer;
	}
	currPage = rootPage;
	lastAllocation = nullptr;
	++epoch;
}

void PagedAllocator::reset(void* offset) {
	// Note: do not free pages after the one containing offset
	lastAllocation = nullptr;
	for (Page* page = currPage; page != nullptr; page = page->prev) {
		if (isPointerInRange(offset, page->buffer, pageSize - sizeof(Page))) {
			page->offset = offset;
			currPage = page;
			//++epoch;
			return;
		}
	}
	assert(false);
}

inline void* PagedAllocator::getOffset() const {
	return currPage ? currPage->offset : nullptr;
}

PagedAllocator::Page* PagedAllocator::allocPage() {
	// Single allocation for both page and buffer
	void* mem = allocator->alloc(pageSize, Allocator::defaultAlignment);
	assert(mem);
	void*      buffer = advancePointer(mem, sizeof(Page));
	const Page newPage {
		.prev = nullptr,
		.next = nullptr,
		.buffer = buffer,
		.offset = buffer,
	};
	std::memcpy(mem, &newPage, sizeof newPage);
	return static_cast<Page*>(mem);
}

void* PagedAllocator::allocFromPage(Page& page, size_t size, size_t alignment) const {
	size_t freeSize = (pageSize - sizeof(Page)) - pointerDiffU(page.buffer, page.offset);
	assert(freeSize <= pageSize - sizeof(Page));
	void* result = std::align(alignment, size, page.offset, freeSize);
	if (result) {
		page.offset = advancePointer(result, size);
	}
	return result;
}

uint32_t PagedAllocator::getEpoch() const {
	return epoch;
}

size_t PagedAllocator::getCapacity() const {
	size_t capacity = 0;
	for (const Page* page = currPage; page; page = page->prev) {
		capacity += pageSize - sizeof(Page);
	}
	return capacity;
}

size_t PagedAllocator::getAllocatedSize() const {
	size_t size = 0;
	for (const Page* page = currPage; page; page = page->prev) {
		size += pointerDiffU(page->offset, page->buffer);
	}
	return size;
}

} // namespace Typhoon
