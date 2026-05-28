#include "scopedAllocator.h"
#include <cassert>

namespace Typhoon {

struct ScopedAllocator::Finalizer {
	void (*destructor)(void* ptr);
	void*      obj;
	Finalizer* next;
#ifdef _DEBUG
	uint32_t backingEpoch;
#endif
};

ScopedAllocator::ScopedAllocator(ArenaAllocator& backingAllocator)
    : backingAllocator { backingAllocator }
    , finalizerHead { nullptr } {
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
#ifdef _DEBUG
	f->backingEpoch = backingAllocator.getEpoch();
#endif
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
#ifdef _DEBUG
		assert(backingAllocator.getEpoch() == f->backingEpoch);
#endif
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

void ScopedAllocator::debug() const {
	for (const Finalizer *f = finalizerHead, *next = nullptr; f; f = next) {
		assert(backingAllocator.getEpoch() == f->backingEpoch);
	}
}
#endif

} // namespace Typhoon
