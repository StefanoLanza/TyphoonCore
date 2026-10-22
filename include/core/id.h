#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#if __cplusplus >= 202002L
#include <cassert>
#include <concepts>
#include <utility> // in_range
#endif

namespace Typhoon {

// Strongly typed id
/* Example:
struct ComponentTag;  // no need to define it
using ComponentId = Id<ComponentTag, uint32_t>;
*/
template <typename Tag, typename Impl = uint32_t, Impl nullValue = 0>
#if __cplusplus >= 202002L
requires std::unsigned_integral<Impl>
#endif
struct Id {
	constexpr Id()
	    : value { nullValue } {
	}
	explicit constexpr Id(Impl value)
	    : value { value } {
	}
#if __cplusplus >= 202002L
	template <std::integral T>
#endif
	explicit constexpr Id(T value)
	    : value { static_cast<Impl>(value) } {
		static_assert(std::is_integral_v<T>);
#if __cplusplus >= 202002L
		assert(std::in_range<Impl>(value));
#endif
	}

	[[nodiscard]] explicit constexpr operator bool() const {
		return value != nullValue;
	}

	constexpr void set(Impl newValue) {
		value = newValue;
	}

	constexpr void reset() {
		value = nullValue;
	}

	[[nodiscard]] constexpr Impl getValue() const {
		return value;
	}

	[[nodiscard]] constexpr bool isValid() const {
		return value != nullValue;
	}

	[[nodiscard]] constexpr bool isNull() const {
		return value == nullValue;
	}

	[[nodiscard]] friend constexpr bool operator==(Id a, Id b) {
		return a.value == b.value;
	}
	[[nodiscard]] friend constexpr bool operator!=(Id a, Id b) {
		return a.value != b.value;
	}

	[[nodiscard]] constexpr static Impl getNullValue() {
		return nullValue;
	}

private:
	Impl value;
};

} // namespace Typhoon
