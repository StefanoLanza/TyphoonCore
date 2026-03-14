#pragma once

#include "handle.h"
#include <cassert>
#include <core/base.h>
#include <cstddef>
#include <vector>

namespace Typhoon {

class HandleManager {
public:
	explicit HandleManager(size_t reservedCapacity = 0);
	~HandleManager();

	Handle AcquireHandle();
	// \return index of deleted element
	// Use as element[index] = elements.back(); elements.pop_back();
	uint          ReleaseElementByHandle(Handle handle);
	size_t        GetSize() const;
	void          ReleaseAll();
	uint          GetIndex(Handle handle) const;
	size_t        GetCount() const;
	Handle        GetHandle(size_t index) const;
	bool          IsValid(Handle handle) const;
	const uint32* GetHandleToIndexTable() const {
		return m_sparseToDense.data();
	}

private:
	std::vector<uint32> m_sparseToDense;
	std::vector<uint32> m_denseToSparse;
	std::vector<uint8>  m_generations;
	int                 m_freeHandle;
};

inline uint HandleManager::GetIndex(Handle handle) const {
	assert(IsValid(handle));
	return m_sparseToDense[handle.index];
}

inline Handle HandleManager::GetHandle(size_t elementIndex) const {
	Handle handle;
	handle.index = m_denseToSparse[elementIndex];
	handle.generation = m_generations[handle.index];
	return handle;
}

void mapHandlesToIndices(uint32_t* indices, const Handle* handles, const uint32_t* sparseToDense, size_t numHandles);

} // namespace Typhoon
