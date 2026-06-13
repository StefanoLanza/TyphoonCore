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
	explicit StringTable(Allocator& allocator, size_t bufferSize, uint32 maxStrings);
	~StringTable();

	static size_t requiredBufferSize(uint32 maxStrings, size_t stringDataCapacity);

	bool             empty() const;
	StringId         insert(std::string_view sv);
	std::string_view fetch(StringId strId) const;
	bool             compare(StringId first, std::string_view second) const;

private:
	struct Entry;
	const Entry* getEntries() const;
	const char*  getStringBuffer() const;

private:
	static constexpr uint32 _bucketCount = 64;

	Allocator& _allocator;
	char*      _buffer;
	size_t     _bufferSize;
	uint32     _maxStrings;
	uint32     _strOffs;
	uint32     _strCapacity;
	uint32     _numEntries;
};

class StringHandle {
public:
	StringHandle(const char* str);
	StringHandle(StringId strId);
};

} // namespace Typhoon
