#pragma once

#include <functional> // hash
#include <type_traits>

namespace Typhoon {

// struct uniquely representing a C++ type
struct TypeId {
	const void* impl = nullptr;

	explicit constexpr operator bool() const {
		return impl != nullptr;
	}
	// Debug/logging only. NOT stable across runs, builds, or processes. Never serialize this
	uintptr_t value() const {
		return reinterpret_cast<uintptr_t>(impl);
	}
};

inline constexpr TypeId nullTypeId { nullptr };
inline constexpr bool   operator==(TypeId a, TypeId b) {
    return a.impl == b.impl;
}
inline constexpr bool operator!=(TypeId a, TypeId b) {
	return ! (a.impl == b.impl);
}

namespace detail {

template <typename T>
struct TypeTag {
	// MUST NOT be const/constexpr: const data lands in a read-only COMDAT and
	// can be merged by MSVC /OPT:ICF, lld --icf=all, or -fmerge-all-constants,
	// which would give two distinct types the same address.
	inline static char id = 0;
};

} // namespace detail

template <typename T>
constexpr TypeId getTypeId() noexcept {
	using BareType = std::remove_cv_t<T>; // remove const and volatile
	return TypeId { &detail::TypeTag<BareType>::id };
}

// Forces constant initialization: a compile error if the id ever stops being
// a constant expression. Prefer this over calling getTypeId<T>() directly.
template <typename T>
inline constexpr TypeId typeId_v = getTypeId<T>();

// This can be specialized, e.g. for polymorphic objects
template <typename T>
constexpr TypeId getTypeId(const T* /*dummy*/) noexcept {
	static_assert(! std::is_pointer_v<T>);
	return getTypeId<T>();
}

using TypeName = const char*;

TypeName typeIdToName(TypeId typeId);
TypeId   typeNameToId(const char* typeName);
void     registerTypeName(TypeId typeId, const char* className);

template <typename T>
TypeName typeName() {
	return typeIdToName(typeId_v<T>);
}

// This can be specialized, e.g. for polymorphic objects
template <typename T>
TypeName typeName(const T* dummy) {
	static_assert(! std::is_pointer_v<T>);
	return typeIdToName(getTypeId(dummy));
}

} // namespace Typhoon

template <>
struct std::hash<Typhoon::TypeId> {
	std::size_t operator()(const Typhoon::TypeId& typeId) const {
		// 1-byte tags are often adjacent in memory, so raw pointer hashes have
		// almost no entropy in the low bits. Mix (splitmix64 finalizer).
		std::uint64_t x = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(typeId.impl));
		x ^= x >> 30;
		x *= 0xbf58476d1ce4e5b9ULL;
		x ^= x >> 27;
		x *= 0x94d049bb133111ebULL;
		x ^= x >> 31;
		return static_cast<std::size_t>(x);
	}
};
