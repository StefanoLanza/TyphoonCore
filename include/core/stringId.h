#pragma once

#include <core/id.h>
#include <cstdint>

namespace Typhoon {

struct StringTag;
using StringId = Id<StringTag, uint32_t, -1>;

} // namespace Typhoon
