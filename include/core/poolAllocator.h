#pragma once

#include <cassert>
#include <cstddef>
#include <utility>

namespace Typhoon {

class Allocator;

class BasePoolAllocator {
protected:
	BasePoolAllocator(Allocator& baseAllocator, size_t capacity, size_t elementSize, size_t alignment);
	~BasePoolAllocator();

	void*  alloc();
	void   free(void* ptr);
	//size_t getIndex(const void* ptr) const;
	//void*  getPtr(size_t index) const;
	void   clear();

private:
	struct FreeSlot;
	struct Page;

	void* allocFromPage(Page& page) const;
	Page* allocPage();
	void  freePage(Page& page);

	Allocator& baseAllocator;
	size_t     pageSize;
	Page*      rootPage;
	size_t     elementSize;
	size_t     alignment;
};

template <class T>
class PoolAllocator : private BasePoolAllocator {
public:
	PoolAllocator(Allocator& baseAllocator, size_t capacity = 65536)
	    : BasePoolAllocator(baseAllocator, capacity, sizeof(T), alignof(T)) {
	}

	template <class... ArgTypes>
	T* tryMake(ArgTypes&&... args) {
		void* ptr = alloc();
		return ptr ? new (ptr) T(std::forward<ArgTypes>(args)...) : nullptr;
	}
	template <class... ArgTypes>
	T* make(ArgTypes&&... args) {
		void* ptr = alloc();
		assert(ptr);
		return new (ptr) T(std::forward<ArgTypes>(args)...);
	}
	void destroy(T* ptr) {
		if (ptr) {
			ptr->~T();
			free(ptr);
		}
	}

	//using BasePoolAllocator::getIndex;
	using BasePoolAllocator::clear;

#if 0
	void destroyAt(size_t index) {
		destroy(static_cast<T*>(getPtr(index)));
	}

	T& operator[](size_t index) const {
		return *static_cast<T*>(getPtr(index));
	}
#endif
};

} // namespace Typhoon
