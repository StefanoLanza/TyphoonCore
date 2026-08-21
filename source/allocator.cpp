#include "allocator.h"
#include "ptrUtil.h"

#include <cassert>
#include <cstdlib>
#include <memory>

namespace Typhoon {

void* HeapAllocator::alloc(size_t size, [[maybe_unused]] size_t alignment) {
#ifdef _MSC_VER
	return _aligned_malloc(size, alignment);
#else
	return ::malloc(size);
#endif
}

void HeapAllocator::free(void* ptr, [[maybe_unused]] size_t size) {
#ifdef _MSC_VER
	::_aligned_free(ptr);
#else
	::free(ptr);
#endif
}

void* HeapAllocator::realloc(void* ptr, [[maybe_unused]] size_t currSize, size_t newSize, [[maybe_unused]] size_t alignment) {
#ifdef _MSC_VER
	return _aligned_realloc(ptr, newSize, alignment);
#else
	return ::realloc(ptr, newSize);
#endif
}

size_t HeapAllocator::maxAllocSize() const {
	return std::numeric_limits<size_t>::max();
}

#ifdef _DEBUG
uint32_t HeapAllocator::getEpoch() const {
	return 0;
}

void HeapAllocator::check([[maybe_unused]] const void* ptr, [[maybe_unused]] uint32_t ptrEpoch) {
	// Rely on malloc builtin checks
}
#endif

BufferAllocator::BufferAllocator(void* buffer, size_t bufferSize)
    : buffer(buffer)
    , backingAllocator(nullptr)
    , curr(buffer)
    , bufferSize(bufferSize)
    , lastAlloc { nullptr }
#ifdef _DEBUG
    , epoch(0)
    , backingEpoch(0)
#endif
{
}

BufferAllocator::BufferAllocator(Allocator& backingAllocator, size_t bufferSize)
    : buffer(backingAllocator.alloc(bufferSize, backingAllocator.defaultAlignment))
    , backingAllocator(&backingAllocator)
    , curr(buffer)
    , bufferSize(bufferSize)
    , lastAlloc { nullptr }
#ifdef _DEBUG
    , epoch(0)
    , backingEpoch(backingAllocator.getEpoch())
#endif
{
}

BufferAllocator::~BufferAllocator() {
	if (backingAllocator) {
		backingAllocator->free(buffer, bufferSize);
	}
}

void* BufferAllocator::alloc(size_t size, size_t alignment) {
	size_t freeSize = bufferSize - pointerDiffU(buffer, curr);
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
		assert(isPointerAligned(ptr, alignment));
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

void BufferAllocator::free(void* ptr, [[maybe_unused]] size_t size) {
	if (lastAlloc == ptr) {
		curr = ptr;
	}
}

size_t BufferAllocator::maxAllocSize() const {
	return bufferSize;
}

void BufferAllocator::reset() {
	curr = buffer;
	lastAlloc = nullptr;
#ifdef _DEBUG
	++epoch;
#endif
}

void BufferAllocator::reset(void* offs) {
	assert(isPointerInRange(offs, buffer, curr));
	curr = offs;
	if (lastAlloc != offs) {
		lastAlloc = nullptr;
	}
}

void* BufferAllocator::getOffset() const {
	return curr;
}

void* BufferAllocator::getBuffer() const {
	return buffer;
}

#ifdef _DEBUG
uint32_t BufferAllocator::getEpoch() const {
	return epoch;
}

void BufferAllocator::check(const void* ptr, uint32_t ptrEpoch) {
	assert(ptrEpoch == this->epoch && isPointerInRange(ptr, buffer, curr));
}

void BufferAllocator::debug() const {
	if (backingAllocator) {
		backingAllocator->check(buffer, backingEpoch);
	}
}

#endif

PagedAllocator::PagedAllocator(Allocator& backingAllocator, size_t pageSize)
    : backingAllocator(&backingAllocator)
    , pageSize(pageSize)
    , rootPage(nullptr)
    , currPage(nullptr)
    , lastAllocation(nullptr)
#ifdef _DEBUG
    , epoch(0)
    , backingEpoch(0)
#endif
{
	assert(pageSize > sizeof(Page));
	assert(pageSize <= backingAllocator.maxAllocSize());
}

PagedAllocator::~PagedAllocator() {
	for (Page* page = rootPage; page;) {
		Page* next = page->next; // fetch before freeing page
		backingAllocator->free(page, pageSize);
		page = next;
	}
}

void* PagedAllocator::alloc(size_t size, size_t alignment) {
	if (size > pageSize - sizeof(Page)) {
		assert(false && "Increase page size or use heap");
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
		assert(isPointerAligned(ptr, alignment));
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

void PagedAllocator::free([[maybe_unused]] void* ptr, [[maybe_unused]] size_t size) {
	// nop
}

size_t PagedAllocator::maxAllocSize() const {
	return pageSize - sizeof(Page);
}

#ifdef _DEBUG

uint32_t PagedAllocator::getEpoch() const {
	return epoch;
}

void PagedAllocator::check(const void* ptr, uint32_t ptrEpoch) {
	assert(ptrEpoch == epoch);
	for (Page* page = currPage; page != nullptr; page = page->prev) {
		if (isPointerInRange(ptr, page->buffer, pageSize - sizeof(Page))) {
			return;
		}
	}
	assert(false);
}
#endif

void PagedAllocator::reset() {
#ifdef _DEBUG
	debug();
#endif

	for (Page* page = rootPage; page; page = page->next) {
		page->offset = page->buffer;
	}
	currPage = rootPage;
	lastAllocation = nullptr;
#ifdef _DEBUG
	++epoch;
#endif
}

void PagedAllocator::reset(void* offset) {
#ifdef _DEBUG
	debug();
#endif

	// Note: do not free pages after the one containing offset
	lastAllocation = nullptr;
	for (Page* page = currPage; page != nullptr; page = page->prev) {
		if (isPointerInRange(offset, page->buffer, pageSize - sizeof(Page))) {
			page->offset = offset;
			currPage = page;
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
#ifdef _DEBUG
	backingEpoch = backingAllocator->getEpoch();
#endif

	void* mem = backingAllocator->alloc(pageSize, Allocator::defaultAlignment);
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
#ifdef _DEBUG
	debug();
#endif
	size_t freeSize = (pageSize - sizeof(Page)) - pointerDiffU(page.buffer, page.offset);
	assert(freeSize <= pageSize - sizeof(Page));
	void* result = std::align(alignment, size, page.offset, freeSize);
	if (result) {
		page.offset = advancePointer(result, size);
	}
	return result;
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

#ifdef _DEBUG
void PagedAllocator::debug() const {
	for (const Page* page = rootPage; page; page = page->next) {
		backingAllocator->check(page, backingEpoch);
	}
}
#endif

} // namespace Typhoon
