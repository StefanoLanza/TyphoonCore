#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

#include <core/uncopyable.h>

namespace Typhoon {

class Allocator : Unmoveable {
public:
	static constexpr size_t defaultAlignment = alignof(void*);

	virtual ~Allocator() = default;

	virtual void* alloc(size_t size, size_t alignment) = 0;
	virtual void* realloc(void* ptr, size_t currSize, size_t newSize, size_t alignment) = 0;

	// Helpers
	template <class T, bool zero = false>
	T* alloc() {
		static_assert(std::is_trivially_default_constructible_v<T>, "Use construct");
		void* ptr = alloc(sizeof(T), alignof(T));
		if (! ptr) {
			return nullptr;
		}
		if constexpr (zero) {
			std::memset(ptr, 0, sizeof(T));
		}
		return static_cast<T*>(ptr);
	}

	template <class T, bool zeroPOD = false>
	T* allocArray(size_t count) {
		static_assert(std::is_trivially_default_constructible_v<T>, "Use constructArray");
		void* ptr = alloc(sizeof(T) * count, alignof(T));
		if (! ptr) {
			return nullptr;
		}
		if constexpr (zeroPOD) {
			std::memset(ptr, 0, sizeof(T) * count);
		}
		return static_cast<T*>(ptr);
	}

	template <class T, class... ArgTypes>
	T* constructArray(size_t count, ArgTypes&&... args) {
		void* ptr = alloc(sizeof(T) * count, alignof(T));
		if (! ptr) {
			return nullptr;
		}
		for (size_t i = 0; i < count; ++i) {
			new (static_cast<std::byte*>(ptr) + sizeof(T) * i) T { std::forward<ArgTypes>(args)... };
		}
		return static_cast<T*>(ptr);
	}

	template <class T, class... ArgTypes>
	T* construct(ArgTypes&&... args) {
		void* ptr = alloc(sizeof(T), alignof(T));
		return ptr ? new (ptr) T { std::forward<ArgTypes>(args)... } : nullptr;
	}
};

/**
 * @brief Heap allocator
 */
class HeapAllocator : public Allocator {
public:
	virtual void free(void* ptr, size_t size) = 0;

	template <class T>
	void destroy(T* obj) {
		if (obj) {
			obj->~T();
			free(obj, sizeof *obj);
		}
	}

	template <class T>
	void destroyArray(T* objs, size_t n) {
		if (objs) {
			for (size_t i = 0; i < n; ++i) {
				objs[i].~T();
			}
			free(objs, sizeof *objs * n);
		}
	}
};

class ArenaAllocator : public Allocator {
public:
	using Allocator::alloc;

	virtual void     reset() = 0;
	virtual void     reset(void* offset) = 0;
	virtual void*    getOffset() const = 0;
	virtual uint32_t getEpoch() const = 0;

	template <class T>
	void destroy(T* obj) {
		if (obj) {
			obj->~T();
		}
	}

	template <class T>
	void destroyArray(T* objs, size_t n) {
		if (objs) {
			for (size_t i = 0; i < n; ++i) {
				objs[i].~T();
			}
		}
	}
};

/**
 * @brief Heap allocator implementation using malloc, free and realloc
 */
class C_Allocator final : public HeapAllocator {
public:
	void* alloc(size_t size, size_t alignment) override;
	void  free(void* ptr, size_t size) override;
	void* realloc(void* ptr, size_t currSize, size_t newSize, size_t alignment) override;
};

class BufferAllocator final : public ArenaAllocator {
public:
	BufferAllocator(void* buffer, size_t bufferSize);
	BufferAllocator(HeapAllocator& backingAllocator, size_t bufferSize);
	~BufferAllocator();

	void*    alloc(size_t size, size_t alignment) override;
	void*    realloc(void* ptr, size_t oldSize, size_t newSize, size_t alignment) override;
	void     reset() override;
	void     reset(void* offset) override;
	void*    getOffset() const override;
	uint32_t getEpoch() const override;
	void*    getBuffer() const;

private:
	void*          buffer;
	HeapAllocator* backingAllocator;
	void*          curr;
	size_t         bufferSize;
	void*          lastAlloc;
	uint32_t       epoch;
};

class PagedAllocator final : public ArenaAllocator {
public:
	PagedAllocator(HeapAllocator& backingAllocator, size_t pageSize = defaultPageSize);
	~PagedAllocator();

	void*    alloc(size_t size, size_t alignment) override;
	void*    realloc(void* ptr, size_t oldSize, size_t newSize, size_t alignment) override;
	void     reset() override;
	void     reset(void* offset) override;
	void*    getOffset() const override;
	uint32_t getEpoch() const override;
	size_t   getCapacity() const;
	size_t   getAllocatedSize() const;

	static constexpr size_t defaultPageSize = 65536;

private:
	struct Page {
		Page* prev;
		Page* next;
		void* buffer;
		void* offset;
	};

	Page* allocPage();
	void* allocFromPage(Page& page, size_t size, size_t alignment) const;

private:
	HeapAllocator* allocator;
	size_t         pageSize;
	Page*          rootPage;
	Page*          currPage;
	void*          lastAllocation;
	uint32_t       epoch;
};

} // namespace Typhoon
