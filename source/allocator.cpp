#include "allocator.h"
#include "ptrUtil.h"

#include <cassert>
#include <cstdlib>
#include <limits>
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
	size_t freeSize = reinterpret_cast<uintptr_t>(buffer) + bufferSize - reinterpret_cast<uintptr_t>(curr);
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
	assert(static_cast<const std::byte*>(offs) >= static_cast<const std::byte*>(buffer) &&
	       static_cast<const std::byte*>(offs) < static_cast<const std::byte*>(buffer) + bufferSize);
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
    , currPage(nullptr)
    , pageCount(0)
    , epoch(0) {
	assert(pageSize > sizeof(Page));
}

PagedAllocator::~PagedAllocator() {
	for (Page* page = currPage; page;) {
		Page* prev = page->prev; // Fetch before freeing page
		allocator->free(page->buffer, pageSize);
		page = prev;
	}
}

void* PagedAllocator::alloc(size_t size, size_t alignment) {
	if (size > pageSize - sizeof(Page)) {
		return nullptr;
	}

	for (Page* page = currPage; page != nullptr; page = page->prev) {
		if (void* result = allocFromPage(*page, size, alignment); result) {
			return result;
		}
	}

	Page* newPage = allocPage();
	if (newPage) {
		newPage->prev = currPage;
		currPage = newPage;
		if (void* result = allocFromPage(*newPage, size, alignment); result) {
			return result;
		}
	}
	return nullptr;
}

void* PagedAllocator::realloc(void* ptr, size_t currSize, size_t newSize, size_t alignment) {
	for (Page* page = currPage; page != nullptr; page = page->prev) {
		if (ptr && page->lastAllocation == ptr) {
			assert(isAligned(ptr, alignment));
			size_t freeSize = reinterpret_cast<uintptr_t>(page->buffer) + page->size - reinterpret_cast<uintptr_t>(ptr);
			if (freeSize >= newSize) {
				page->offset = advancePointer(ptr, newSize);
				return ptr;
			}
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
	Page* rootPage = nullptr;
	for (Page* page = currPage; page; page = page->prev) {
		page->offset = advancePointer(page->buffer, sizeof(Page));
		page->lastAllocation = nullptr;
		rootPage = page;
	}
	currPage = rootPage;
	++epoch;
}

void PagedAllocator::reset(void* offset) {
	for (Page* page = currPage; page != nullptr; page = page->prev) {
		if (offset >= page->buffer && offset < static_cast<const char*>(page->buffer) + page->size) {
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
	void* buffer = allocator->alloc(pageSize, Allocator::defaultAlignment);
	if (buffer) {
		const Page newPage {
			.prev = nullptr,
			.buffer = buffer,
			.offset = advancePointer(buffer, sizeof(Page)),
			.lastAllocation = nullptr,
			.size = pageSize - sizeof(Page),
		};
		std::memcpy(buffer, &newPage, sizeof newPage);
		++pageCount;
	}
	return static_cast<Page*>(buffer);
}

void* PagedAllocator::allocFromPage(Page& page, size_t size, size_t alignment) const {
	size_t freeSize = reinterpret_cast<uintptr_t>(page.buffer) + page.size - reinterpret_cast<uintptr_t>(page.offset);
	assert(freeSize <= page.size - sizeof(Page));
	void* result = std::align(alignment, size, page.offset, freeSize);
	if (result) {
		page.lastAllocation = result;
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
		size += static_cast<const char*>(page->offset) - static_cast<const char*>(page->buffer);
	}
	return size;
}

} // namespace Typhoon
