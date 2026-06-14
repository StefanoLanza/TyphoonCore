#include "stringTable.h"
#include <cassert>
#include <core/allocator.h>
#include <core/hash.h>
#include <limits>

namespace Typhoon {

struct StringTable::Entry {
	uint32 offs;
	uint32 hash;
	Index  next;
	uint16 len;
};

constexpr uint32 bucketCount = 256;

// Buffer layout is | buckets | entries -> ... <- strings |
StringTable::StringTable(Allocator& allocator, size_t bufferSize)
    : _allocator { allocator }
    , _buffer { static_cast<char*>(allocator.alloc(bufferSize, 1)) }
    , _bufferSize { bufferSize }
    , _strBytesUsed { 0 }
    , _numEntries { 0 } {

	assert(bufferSize > sizeof(Index) * bucketCount && "bufferSize too small to hold buckets and entries");

	Index* buckets = getBuckets();
	for (uint32_t i = 0; i < bucketCount; i++) {
		buckets[i] = (Index)-1;
	}
}

StringTable::~StringTable() {
	_allocator.free(_buffer, _bufferSize);
}

bool StringTable::empty() const {
	return _numEntries == 0;
}

StringId StringTable::insert(std::string_view sv) {
	const uint32 len = (uint32)sv.length();
	assert(len <= 255);
	Index* buckets = getBuckets();
	Entry* entries = reinterpret_cast<Entry*>(buckets + bucketCount);

	const uint32 hash = FNVhash32(sv.data(), sv.length());
	const uint32 bucketIdx = hash % bucketCount;

	// Search string
	Index e = buckets[bucketIdx];
	while (e != (Index)-1) {
		const Entry& entry = entries[e];
		if (entry.hash == hash && entry.len == len) {
#ifdef _DEBUG
			assert(std::memcmp(_buffer + entry.offs, sv.data(), len) == 0 && "hash collision detected");
#endif
			return StringId { e };
		}
		e = entry.next;
	}

	// Insert new string
	// Compute where the new string and new entry would land
	// Strings grow downward from the top of the buffer
	const uint32 newStrBytesUsed = _strBytesUsed + len + 1; // +1 for '\0'
	const uint32 strOffs = static_cast<uint32>(_bufferSize) - newStrBytesUsed;

	const Entry* nextEntry = entries + _numEntries;

	// Single unified collision check: entry region vs string region
	if (reinterpret_cast<const char*>(nextEntry + 1) > _buffer + strOffs) {
		assert(false && "Full buffer");
		return {};
	}

	std::memcpy(_buffer + strOffs, sv.data(), len);
	_buffer[strOffs + len] = '\0';

	assert(_numEntries < std::numeric_limits<Index>::max() && "entry count exceeds range");
	const uint32 id = _numEntries++;
	_strBytesUsed = newStrBytesUsed;
	entries[id] = {
		.offs = strOffs,
		.hash = hash,
		.next = buckets[bucketIdx],
		.len = static_cast<uint8>(len),
	};
	buckets[bucketIdx] = static_cast<Index>(id);

	return StringId { id };
}

std::string_view StringTable::fetch(StringId strId) const {
	assert(strId);
	const Entry& e = getEntries()[strId.getValue()];
	return { _buffer + e.offs, e.len };
}

bool StringTable::compare(StringId first, std::string_view second) const {
	assert(first);
	const Entry& e = getEntries()[first.getValue()];
	if (e.len == second.length()) {
		const char* str = _buffer + e.offs;
		return (std::memcmp(str, second.data(), second.length()) == 0);
	}
	return false;
}

const StringTable::Entry* StringTable::getEntries() const {
	return reinterpret_cast<const Entry*>(_buffer + sizeof(Index) * bucketCount);
}

StringTable::Index* StringTable::getBuckets() const {
	return reinterpret_cast<Index*>(_buffer);
}

} // namespace Typhoon
