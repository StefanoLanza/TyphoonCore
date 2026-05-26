#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace Typhoon {

class Allocator;

class BasePoolAllocator {
public:
	BasePoolAllocator(Allocator& backingAllocator, size_t maxElements, size_t elementSize, size_t alignment);
	~BasePoolAllocator();

	void*  alloc();
	void   free(void* ptr);
	void   clear();
	size_t getCapacity() const;

private:
	void init();
#ifdef _DEBUG
	void debug();
#endif

private:
	struct FreeSlot {
		FreeSlot* next;
	};

	Allocator& backingAllocator;
	size_t     maxElements;
	size_t     elementSize;
	size_t     alignment;
	void*      buffer;
	FreeSlot*  nextFreeSlot;
#ifdef _DEBUG
	uint32_t epoch = 0;
#endif
};

template <class T>
class PoolAllocator final {
public:
	explicit PoolAllocator(Allocator& backingAllocator, size_t maxElements)
	    : base { backingAllocator, maxElements, sizeof(T), alignof(T) } {
		static_assert(sizeof(T) % alignof(T) == 0);
	}

	template <class... ArgTypes>
	T* create(ArgTypes&&... args) {
		void* ptr = base.alloc();
		assert(ptr);
		return new (ptr) T { std::forward<ArgTypes>(args)... };
	}
	void destroy(T* ptr) {
		assert(ptr);
		std::destroy_at(ptr);
		base.free(ptr);
	}

	void clear() requires(std::is_trivially_destructible_v<T>) {
		base.clear();
	}
	size_t getCapacity() const {
		return base.getCapacity();
	}

private:
	BasePoolAllocator base;
};

} // namespace Typhoon
