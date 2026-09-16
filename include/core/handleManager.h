#pragma once

#include "handle.h"
#include <cstddef>
#include <vector>

namespace Typhoon {

class HandleAllocator final {
public:
	explicit HandleAllocator(size_t reservedCapacity = 0);

	Handle acquire();
	void   release(Handle handle);
	bool   isValid(Handle handle) const;
	void   releaseAll();

private:
	static constexpr uint32_t invalidIndex = 0x00FFFFFFu;

	struct Entry {
		uint32_t nextFree : 24;
		uint32_t generation : 8;
	};
	static_assert(sizeof(Entry) == sizeof(uint32_t));

	std::vector<Entry> entries;
	uint32_t           freeHandle;
};

class DenseIndexMap final {
public:
	using Index = uint32_t;

	explicit DenseIndexMap(size_t reservedCapacity = 0);

	Index           insert(Handle handle);
	Index           remove(Handle handle);
	Index           getIndex(Handle handle) const;
	Handle          getHandle(Index index) const;
	size_t          size() const;
	bool            empty() const;
	void            clear();
	const uint32_t* getSparseToDenseTable() const;

private:
	static constexpr Index invalidIndex = std::numeric_limits<Index>::max();

	void ensureSparseCapacity(uint32_t handleIndex);

	// Handle index -> dense index.
	std::vector<Index> sparseToDense;
	// Dense index -> complete Handle.
	std::vector<uint32_t> denseToSparse;
};

} // namespace Typhoon
