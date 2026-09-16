#include "handleManager.h"
#include <cassert>

namespace Typhoon {

HandleAllocator::HandleAllocator(size_t reservedCapacity)
    : freeHandle(invalidIndex) {
	entries.reserve(reservedCapacity);
}

Handle HandleAllocator::acquire() {
	uint32_t index;

	if (freeHandle != invalidIndex) {
		index = freeHandle;
		Entry& entry = entries[index];
		freeHandle = entry.nextFree;
		// An allocated entry is marked by InvalidIndex.
		entry.nextFree = invalidIndex;
		return Handle(index, entry.generation);
	}

	index = static_cast<uint32_t>(entries.size());

	// 24 bits are available for the index.
	assert(index < invalidIndex);

	entries.push_back({
	    invalidIndex, // nextFree: allocated
	    1             // generation
	});

	return Handle { index, 1 };
}

void HandleAllocator::release(Handle handle) {
	assert(isValid(handle));

	Entry& entry = entries[handle.index];

	// Advance generation so previously issued handles become stale
	entry.generation = static_cast<uint8_t>(entry.generation + 1);

	// Generation 0 is reserved for null handles
	if (entry.generation == 0) {
		entry.generation = 1;
	}

	// Put this entry at the head of the free list
	entry.nextFree = freeHandle;
	freeHandle = handle.index;
}

bool HandleAllocator::isValid(Handle handle) const {
	if (handle.index >= entries.size()) {
		return false;
	}
	const Entry& entry = entries[handle.index];
	// Note: a null handle has generation == 0, which always fails the next check since entry.generation >= 1
	return entry.nextFree == invalidIndex && entry.generation == handle.generation;
}

void HandleAllocator::releaseAll() {
	entries.clear();
	freeHandle = invalidIndex;
}

DenseIndexMap::DenseIndexMap(size_t reservedCapacity) {
	sparseToDense.reserve(reservedCapacity);
	denseToSparse.reserve(reservedCapacity);
}

DenseIndexMap::Index DenseIndexMap::insert(Handle handle) {
	assert(handle.isValid());

	const Index denseIndex = static_cast<Index>(denseToSparse.size());

	ensureSparseCapacity(handle.index);

	sparseToDense[handle.index] = denseIndex;

	denseToSparse.push_back(handle.get());

	return denseIndex;
}

DenseIndexMap::Index DenseIndexMap::remove(Handle handle) {
	const Index denseIndex = sparseToDense[handle.index];
	const Index last = static_cast<Index>(denseToSparse.size() - 1);
	if (denseIndex != last) {
		const Handle movedHandle = Handle(denseToSparse[last]);
		denseToSparse[denseIndex] = denseToSparse[last];
		sparseToDense[movedHandle.index] = denseIndex;
	}
	denseToSparse.pop_back();
	sparseToDense[handle.index] = invalidIndex;
	return denseIndex;
}

DenseIndexMap::Index DenseIndexMap::getIndex(Handle handle) const {
	return sparseToDense[handle.index];
}

Handle DenseIndexMap::getHandle(DenseIndexMap::Index index) const {
	assert(index < denseToSparse.size());
	return Handle(denseToSparse[index]);
}

size_t DenseIndexMap::size() const {
	return denseToSparse.size();
}

bool DenseIndexMap::empty() const {
	return denseToSparse.empty();
}

void DenseIndexMap::clear() {
	sparseToDense.clear();
	denseToSparse.clear();
}

const uint32_t* DenseIndexMap::getSparseToDenseTable() const {
	return sparseToDense.data();
}

void DenseIndexMap::ensureSparseCapacity(uint32_t handleIndex) {
	if (handleIndex < sparseToDense.size()) {
		return;
	}
	sparseToDense.resize(static_cast<size_t>(handleIndex) + 1, invalidIndex);
}

} // namespace Typhoon
