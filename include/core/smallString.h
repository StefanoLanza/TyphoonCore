#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace Typhoon {

template <size_t L>
class SmallString {
public:
	SmallString() {
		storage[0] = 0;
		hash = 0;
	}

	explicit SmallString(const char* str);

	SmallString<L>& operator=(const char*);

	void clear() {
		hash = 0;
		storage[0] = 0;
	}

	bool empty() const {
		return storage[0] == 0;
	}

	const char* str() const {
		return storage;
	}
	uint32_t getHash() const {
		return hash;
	}

	bool operator==(const SmallString& str) const {
		return (hash == str.hash) && (! strcmp(storage, str.storage));
	}

	bool operator==(const char* other) const {
		return ! strcmp(storage, other);
	}

	bool operator==(std::string_view other) const {
		return other == storage;
	}

private:
	char     storage[L];
	uint32_t hash;
};

} // namespace Typhoon

#include "smallString.inl"
