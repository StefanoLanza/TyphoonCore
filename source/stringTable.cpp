#include "stringTable.h"
#include <cassert>
#include <core/allocator.h>
#include <core/hash.h>

namespace Typhoon {

struct StringTable::Entry {
	uint32 offs;
	uint32 hash;
	uint32 len; // TODO with string ?
	uint32 next;
};

size_t StringTable::requiredBufferSize(uint32 maxStrings, size_t stringDataCapacity) {
	// +maxStrings: one null termination per string
	return sizeof(uint32) * _bucketCount + sizeof(Entry) * maxStrings + stringDataCapacity + maxStrings;
}

StringTable::StringTable(Allocator& allocator, size_t bufferSize, uint32 maxStrings)
    : _allocator { allocator }
    , _buffer { static_cast<char*>(allocator.alloc(bufferSize, 1)) }
    , _bufferSize { bufferSize }
    , _maxStrings { maxStrings }
    , _strOffs { 0 }
    , _numEntries { 0 } {

	const size_t overhead = sizeof(uint32) * _bucketCount + sizeof(Entry) * maxStrings;
	assert(bufferSize > overhead && "bufferSize too small to hold buckets and entries");
	_strCapacity = static_cast<uint32>(_bufferSize - overhead);

	uint32* buckets = reinterpret_cast<uint32*>(_buffer);
	for (uint32_t i = 0; i < _bucketCount; i++) {
		buckets[i] = UINT32_MAX;
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
	uint32*      buckets = reinterpret_cast<uint32*>(_buffer);
	Entry*       entries = reinterpret_cast<Entry*>(buckets + _bucketCount);
	char*        strings = reinterpret_cast<char*>(entries + _maxStrings);

	const uint32 hash = FNVhash32(sv.data(), sv.length());
	const uint32 bucketIdx = hash % _bucketCount;

	// Search string
	uint32 e = buckets[bucketIdx];
	while (e != UINT32_MAX) {
		const Entry& entry = entries[e];
		if (entry.hash == hash && entry.len == len) {
			const char* existing = strings + entry.offs;
			if (std::memcmp(existing, sv.data(), len) == 0) {
				return StringId { e };
			}
		}
		e = entry.next;
	}

	// Insert new string
	if (_numEntries >= _maxStrings) {
		assert(false);
		return {};
	}
	if (static_cast<uint64>(_strOffs) + len + 1 > _strCapacity) {
		assert(false);
		return {};
	}

	const uint32 id = _numEntries++;
	entries[id] = {
		.offs = _strOffs,
		.hash = hash,
		.len = len,
		.next = buckets[bucketIdx],
	};

	std::memcpy(strings + _strOffs, sv.data(), len);
	strings[_strOffs + len] = 0; // null terminate
	_strOffs += len + 1;
	buckets[bucketIdx] = id;

	return StringId { id };
}

std::string_view StringTable::fetch(StringId strId) const {
	assert(strId);
	const Entry& e = getEntries()[strId.getValue()];
	return { getStringBuffer() + e.offs, e.len };
}

bool StringTable::compare(StringId first, std::string_view second) const {
	assert(first);
	const Entry& e = getEntries()[first.getValue()];
	if (e.len == second.length()) {
		const char* str = getStringBuffer() + e.offs;
		return (std::memcmp(str, second.data(), second.length()) == 0);
	}
	return false;
}

const StringTable::Entry* StringTable::getEntries() const {
	return reinterpret_cast<const Entry*>(_buffer + sizeof(uint32) * _bucketCount);
}

const char* StringTable::getStringBuffer() const {
	return _buffer + sizeof(uint32) * _bucketCount + sizeof(Entry) * _maxStrings;
}

} // namespace Typhoon
