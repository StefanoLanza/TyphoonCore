#pragma once

#include <core/typeId.h>

namespace Typhoon {

struct VoidPtr {
	void*  ptr;
	TypeId typeId;
};

template <class T>
inline VoidPtr makeVoidPtr(T* ptr) {
	static_assert(! std::is_pointer_v<T>);
	return { ptr, getTypeId<T>() };
}

} // namespace Typhoon
