#pragma once

#include <cassert>
#include <core/typeId.h>

namespace Typhoon {

// FIXME This is a Var, remove it
struct VoidPtr {
	void*  ptr;
	TypeId typeId;
};

template <class T>
inline VoidPtr makeVoidPtr(T* ptr) {
	static_assert(! std::is_pointer_v<T>);
	assert(ptr);
	return { ptr, getTypeId<T>() };
}

} // namespace Typhoon
