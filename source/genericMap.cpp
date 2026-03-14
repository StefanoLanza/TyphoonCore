#include "genericMap.h"
#include <cassert>

namespace Typhoon {

void* GenericMap::find(TypeId typeId) const {
	auto it = elements.find(typeId.impl);
	return it == elements.end() ? nullptr : it->second;
}

void GenericMap::insert(TypeId typeId, void* elm) {
	assert(elm);
	elements.emplace(typeId.impl, elm);
}

void GenericMap::clear() {
	elements.clear();
}

} // namespace Typhoon
