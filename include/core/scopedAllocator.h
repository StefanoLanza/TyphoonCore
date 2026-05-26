#pragma once

#include "allocator.h"
#include <core/uncopyable.h>
#include <type_traits>

namespace Typhoon {

class ScopedAllocator : Uncopyable {
public:
	explicit ScopedAllocator(ArenaAllocator& backingAllocator);
	~ScopedAllocator();

	template <class T, class... ArgTypes>
	T* make(ArgTypes&&... args) {
		Destructor destructor = nullptr;
		if constexpr (! std::is_trivially_destructible_v<T>) {
			destructor = destructorCall<T>;
		}
		T* ptr = backingAllocator.construct<T>(std::forward<ArgTypes>(args)...);
		registerObject(ptr, destructor);
		return ptr;
	}

	template <class T>
	T* allocArray(size_t elementCount) {
		T* ptr = nullptr;
		if constexpr (std::is_trivially_default_constructible_v<T>) {
			ptr = backingAllocator.allocArray<T>(elementCount);
		}
		else {
			ptr = backingAllocator.constructArray<T>(elementCount);
		}
		if constexpr (std::is_trivially_destructible_v<T>) {
			// Register first element only, to reset the allocator
			registerObject(ptr, nullptr);
		}
		else {
			// Register all array elements, from last to first
			for (size_t i = elementCount; i > 0; --i) {
				registerObject(ptr + i - 1, destructorCall<T>);
			}
		}
		return ptr;
	}

	void destroyAll();

#ifdef _DEBUG
	uint32_t getEpoch() const;
#endif

private:
	using Destructor = void (*)(void* ptr);
	void registerObject(void* obj, Destructor destructor);

	template <typename T>
	static void destructorCall(void* ptr) {
		static_cast<T*>(ptr)->~T();
	}

private:
	struct Finalizer;
	ArenaAllocator& backingAllocator;
	Finalizer*      finalizerHead;
};

} // namespace Typhoon
