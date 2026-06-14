#pragma once

#include <core/base.h>
#include <core/stringId.h>
#include <core/uncopyable.h>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace Typhoon {

class Allocator;

class StringTable : private Uncopyable {
public:
	explicit StringTable(Allocator& allocator, size_t bufferSize);
	~StringTable();

	[[nodiscard]] bool             empty() const;
	[[nodiscard]] StringId         insert(std::string_view sv);
	[[nodiscard]] std::string_view fetch(StringId strId) const;
	[[nodiscard]] bool             compare(StringId first, std::string_view second) const;

private:
	struct Entry;
	using Index = uint16;

	const Entry* getEntries() const;
	Index*       getBuckets() const;

private:
	Allocator& _allocator;
	char*      _buffer;
	size_t     _bufferSize;
	uint32     _strBytesUsed;
	uint32     _numEntries;
};

} // namespace Typhoon
