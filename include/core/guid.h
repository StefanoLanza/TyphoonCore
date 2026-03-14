#pragma once

#include <cstdint>

namespace Typhoon {

class GUID {
public:
	constexpr GUID()
	    : value { 0 } {
	}
	constexpr explicit GUID(uint32_t value)
	    : value { value } {
	}
	[[nodiscard]] constexpr uint32_t get() const {
		return value;
	}
	[[nodiscard]] constexpr bool isValid() const {
		return value != 0;
	}

private:
	uint32_t value;
};

constexpr GUID nullGUID {};

[[nodiscard]] inline bool operator==(GUID a, GUID b) {
	return a.get() == b.get();
}

[[nodiscard]] inline bool operator!=(GUID a, GUID b) {
	return a.get() != b.get();
}

// Create a new GUID
[[nodiscard]] GUID newGUID();

} // namespace Typhoon
