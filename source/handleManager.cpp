#include "handleManager.h"
#include <cassert>

namespace Typhoon {

void mapHandlesToIndices(uint32_t* indices, const Handle* handles, const uint32_t* sparseToDense, size_t numHandles) {
	for (size_t i = 0; i < numHandles; ++i) {
		indices[i] = sparseToDense[handles[i].index];
	}
}

namespace {

void incGeneration(uint8_t& g) {
	++g;
	// Reserve g == 0 for null handle
	if (g == 0) {
		g = 1;
	}
}

} // namespace

HandleManager::HandleManager(size_t reservedCapacity)
    : m_freeHandle(-1) {
	if (reservedCapacity > 0) {
		m_generations.reserve(reservedCapacity);
		m_denseToSparse.reserve(reservedCapacity);
		m_sparseToDense.reserve(reservedCapacity);
	}
}

HandleManager::~HandleManager() = default;

bool HandleManager::IsValid(Handle handle) const {
	return (handle.index < m_generations.size()) && (m_generations[handle.index] == handle.generation);
}

Handle HandleManager::AcquireHandle() {
	const uint32 denseIndex = static_cast<uint32>(m_denseToSparse.size());

	Handle handle;
	if (m_freeHandle == -1) {
		// Create new handle
		handle.set(static_cast<uint32_t>(m_sparseToDense.size()), 1);
		m_sparseToDense.push_back(denseIndex);
		m_generations.push_back(handle.generation);
	}
	else {
		// Recycle a handle that was released
		const uint nextFree = m_sparseToDense[m_freeHandle];
		const uint sparseIndex = m_freeHandle;
		handle.set(sparseIndex, m_generations[sparseIndex]);
		m_sparseToDense[sparseIndex] = denseIndex;
		m_freeHandle = nextFree;
	}
	m_denseToSparse.push_back(handle.index);

	return handle;
}

uint HandleManager::ReleaseElementByHandle(Handle handle) {
	assert(IsValid(handle));
	assert(! m_denseToSparse.empty());

	const uint32 denseIndex = m_sparseToDense[handle.index];

	// Invalidate handle
	incGeneration(m_generations[handle.index]);

	// The last element is copied over the deleted one. Update its handle accordingly
	const size_t last = m_denseToSparse.size() - 1;
	if (denseIndex < last) {
		const uint32 handleOfLastElement = m_denseToSparse[last];
		m_sparseToDense[handleOfLastElement] = denseIndex;
		// Move the released handled to the back of the indexToHandle array, so that it can be recycled
		// std::swap(m_denseToSparse[elementIndex], m_denseToSparse[last]);
		m_denseToSparse[denseIndex] = m_denseToSparse[last];
	}
	m_denseToSparse.pop_back();

	// Add released handle to linked list of free handles
	m_sparseToDense[handle.index] = m_freeHandle;
	m_freeHandle = handle.index;

	return denseIndex;
}

size_t HandleManager::GetSize() const {
	return m_denseToSparse.size();
}

void HandleManager::ReleaseAll() {
	m_sparseToDense.clear();
	m_denseToSparse.clear();
	m_generations.clear();
	m_freeHandle = -1;
}

} // namespace Typhoon
