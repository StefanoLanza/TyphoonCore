#pragma once

#include <core/typeId.h>
#include <unordered_map>

namespace Typhoon {

// Generic map of elements
class GenericMap {
public:
	template <class type>
	void insert(type* element) {
		insert(getTypeId<type>(), element);
	}

	void insert(void* element, TypeId typeId) {
		insert(typeId, element);
	}

	template <class type>
	bool get(type*& element) const {
		void* const voidPtr = find(getTypeId<type>());
		element = static_cast<type*>(voidPtr);
		return voidPtr ? true : false;
	}

	template <class Type>
	Type* get() const {
		void* const voidPtr = find(getTypeId<Type>());
		return static_cast<Type*>(voidPtr);
	}

	void clear();

private:
	void* find(TypeId typeId) const;
	void  insert(TypeId typeId, void*);

private:
	using element_map = std::unordered_map<decltype(TypeId::impl), void*>;
	element_map elements;
};

} // namespace Typhoon
