#pragma once

#include <cassert>
#include <cstddef>
#include <utility>

namespace Typhoon {

class Allocator;

class BasePoolAllocator {
protected:
	BasePoolAllocator(Allocator& backingAllocator, size_t maxElements, size_t elementSize, size_t alignment);
	~BasePoolAllocator();

	void*  alloc();
	void   free(void* ptr);
	void   clear();
	size_t getCapacity() const;

private:
	void init();

private:
	struct FreeSlot;

	Allocator& backingAllocator;
	size_t     maxElements;
	size_t     elementSize;
	size_t     alignment;
	void*      buffer;
	FreeSlot*  nextFreeSlot;
};

template <class T>
class PoolAllocator final : private BasePoolAllocator {
public:
	explicit PoolAllocator(Allocator& backingAllocator, size_t maxElements)
	    : BasePoolAllocator(backingAllocator, maxElements, sizeof(T), alignof(T)) {
		static_assert(sizeof(T) % alignof(T) == 0);
	}

	template <class... ArgTypes>
	T* create(ArgTypes&&... args) {
		void* ptr = alloc();
		assert(ptr);
		return new (ptr) T(std::forward<ArgTypes>(args)...);
	}
	void destroy(T* ptr) {
		assert(ptr);
		if constexpr (! std::is_trivially_destructible_v<T>) {
			ptr->~T();
		}
		free(ptr);
	}

	void clear() requires(std::is_trivially_destructible_v<T>) {
		BasePoolAllocator::clear();
	}
	void getCapacity() const {
		return BasePoolAllocator::getCapacity();
	}
};

} // namespace Typhoon
