#pragma once

#include <cstdint>

namespace Typhoon {

#define TY_CORE_MAJOR_VERSION 1
#define TY_CORE_MINOR_VERSION 0
#define TY_CORE_PATCHLEVEL    0

// This macro turns the version numbers into a numeric value
#define TY_CORE_VERSIONNUM(X, Y, Z) ((X) * 1000 + (Y) * 100 + (Z))

#define TY_CORE_COMPILEDVERSION TY_CORE_VERSIONNUM(TY_CORE_MAJOR_VERSION, TY_CORE_MINOR_VERSION, TY_CORE_PATCHLEVEL)

} // namespace Typhoon
