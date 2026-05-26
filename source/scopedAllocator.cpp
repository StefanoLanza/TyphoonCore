#include "scopedAllocator.h"
#include <cassert>

namespace Typhoon {

struct ScopedAllocator::Finalizer {
	void (*destructor)(void* ptr);
	void*      obj;
	Finalizer* next;
};

ScopedAllocator::ScopedAllocator(ArenaAllocator& backingAllocator)
    : backingAllocator(backingAllocator)
    , finalizerHead(nullptr) {
}

ScopedAllocator::~ScopedAllocator() {
	destroyAll();
}

void ScopedAllocator::registerObject(void* obj, Destructor destructor) {
	auto f = backingAllocator.alloc<Finalizer>();
	assert(f);
	f->destructor = destructor;
	f->obj = obj;
	f->next = finalizerHead;
	finalizerHead = f;
}

void ScopedAllocator::destroyAll() {
	void* last = nullptr;
	for (Finalizer *f = finalizerHead, *next = nullptr; f; f = next) {
		if (f->destructor) {
			f->destructor(f->obj);
		}
		next = f->next;
		last = f->obj;
	}
	if (last) {
		backingAllocator.reset(last);
	}
	finalizerHead = nullptr;
}

#ifdef _DEBUG
uint32_t ScopedAllocator::getEpoch() const {
	return backingAllocator.getEpoch();
}
#endif

} // namespace Typhoon
