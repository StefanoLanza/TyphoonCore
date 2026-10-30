#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace Typhoon {

template <class EnumType>
class Flags final {
	static_assert(std::is_enum_v<EnumType>);

public:
	using Enum = EnumType;
	using ValueType = std::underlying_type<EnumType>::type;
	static_assert(std::is_unsigned_v<ValueType>, "Flags requires an enum with an unsigned underlying type");

	constexpr Flags()
	    : bits {} {};

	constexpr Flags(EnumType bit) noexcept
	    : bits { static_cast<ValueType>(bit) } {
	}

	constexpr Flags(EnumType bit, std::same_as<EnumType> auto... args) noexcept
	    : bits { static_cast<ValueType>(static_cast<ValueType>(bit) | (static_cast<ValueType>(args) | ...)) } {
	}

	[[nodiscard]] constexpr ValueType value() const;
	constexpr void                    set(EnumType bit);
	constexpr void                    unset(EnumType bit);
	[[nodiscard]] constexpr bool      isSet(EnumType bit) const;
	[[nodiscard]] constexpr bool      isNotSet(EnumType bit) const;
	constexpr Flags&                  flip(EnumType bit);
	[[nodiscard]] constexpr bool      allOf(Flags other) const noexcept {
        return (bits & other.bits) == other.bits;
	}

	[[nodiscard]] constexpr bool anyOf(Flags other) const noexcept {
		return (bits & other.bits) != 0;
	}

	constexpr Flags operator~() const noexcept {
		return Flags { std::true_type {}, ~bits };
	}

	constexpr Flags operator&(const Flags& r) const noexcept {
		return Flags { std::true_type {}, bits & r.bits };
	}

	constexpr Flags operator|(const Flags& r) const noexcept {
		return Flags { std::true_type {}, bits | r.bits };
	}

	constexpr Flags operator^(const Flags& r) const noexcept {
		return Flags { std::true_type {}, bits ^ r.bits };
	}

	constexpr Flags& operator|=(const Flags& r) noexcept {
		bits |= r.bits;
		return *this;
	}

	constexpr Flags& operator&=(const Flags& r) noexcept {
		bits &= r.bits;
		return *this;
	}

	constexpr Flags& operator^=(const Flags& r) noexcept {
		bits ^= r.bits;
		return *this;
	}

	friend constexpr bool operator==(Flags, Flags) noexcept = default;

private:
	template <class U>
	constexpr Flags(std::true_type, U bits) noexcept
	    : bits { static_cast<ValueType>(bits) } {
	}

private:
	ValueType bits;
};

template <class EnumType>
inline constexpr Flags<EnumType>::ValueType Flags<EnumType>::value() const {
	return bits;
}

template <class EnumType>
inline constexpr void Flags<EnumType>::set(EnumType bit) {
	bits |= static_cast<ValueType>(bit);
}

template <class EnumType>
inline constexpr void Flags<EnumType>::unset(EnumType bit) {
	bits &= ~static_cast<ValueType>(bit);
}

template <class EnumType>
[[nodiscard]] inline constexpr bool Flags<EnumType>::isSet(EnumType bit) const {
	return bits & static_cast<ValueType>(bit);
}

template <class EnumType>
[[nodiscard]] inline constexpr bool Flags<EnumType>::isNotSet(EnumType bit) const {
	return 0 == (bits & static_cast<ValueType>(bit));
}

template <class EnumType>
inline constexpr Flags<EnumType>& Flags<EnumType>::flip(EnumType bit) {
	bits ^= static_cast<ValueType>(bit);
	return *this;
}

template <class T>
inline constexpr Flags<T> operator&(T l, const Flags<T>& r) noexcept {
	return r & Flags { l };
}

template <class T>
inline constexpr Flags<T> operator|(T l, const Flags<T>& r) noexcept {
	return r | Flags { l };
}

template <class T>
inline constexpr Flags<T> operator^(T l, const Flags<T>& r) noexcept {
	return r ^ Flags { l };
}

template <class T>
inline constexpr Flags<T> operator&(const Flags<T>& r, T l) noexcept {
	return r & Flags { l };
}

template <class T>
inline constexpr Flags<T> operator|(const Flags<T>& r, T l) noexcept {
	return r | Flags { l };
}

template <class T>
inline constexpr Flags<T> operator^(const Flags<T>& r, T l) noexcept {
	return r ^ Flags { l };
}

} // namespace Typhoon
