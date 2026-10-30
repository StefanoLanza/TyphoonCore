#pragma once

#include <core/id.h>
#include <cstdint>

namespace Typhoon {

struct StringTag;
using StringId = Id<StringTag, uint32_t, -1>;

struct StringRef {
	StringRef(StringId id)
	    : id { id } {
	}
	StringRef(const char* str)
	    : str { str } {
	}

	StringId    id {};
	const char* str = nullptr;
};

} // namespace Typhoon
