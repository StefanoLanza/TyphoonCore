#pragma once

#include <cstdint>

namespace Typhoon {

#define TY_CORE_MAJOR_VERSION 1
#define TY_CORE_MINOR_VERSION 0
#define TY_CORE_PATCHLEVEL    0

// This macro turns the version numbers into a numeric value
#define TY_CORE_VERSIONNUM(X, Y, Z) ((X) * 1000 + (Y) * 100 + (Z))

#define TY_CORE_COMPILEDVERSION TY_CORE_VERSIONNUM(TY_CORE_MAJOR_VERSION, TY_CORE_MINOR_VERSION, TY_CORE_PATCHLEVEL)

struct Version {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
};

inline Version getVersion() {
	return { TY_CORE_MAJOR_VERSION, TY_CORE_MINOR_VERSION, TY_CORE_PATCHLEVEL };
}

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)

inline const char* getVersionString() {
	return STR(TY_CORE_MAJOR_VERSION) "." STR(TY_CORE_MINOR_VERSION) "." STR(TY_CORE_PATCHLEVEL);
}

#undef STR_HELPER
#undef STR

} // namespace Typhoon
