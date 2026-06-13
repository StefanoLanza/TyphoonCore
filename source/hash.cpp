#include "hash.h"
#include <cassert>
#include <cstring>

namespace Typhoon {

static inline uint16_t get16bits(const char* d) {
	uint16_t v;
	std::memcpy(&v, d, sizeof(v));
	return v;
}

uint32_t hash32(const char* data, size_t len) {
	uint32_t  hash = static_cast<uint32_t>(len);
	uint32_t  tmp;
	const int rem = len & 3;

	len >>= 2;

	/* Main loop */
	for (; len > 0; len--) {
		hash += get16bits(data);
		tmp = (get16bits(data + 2) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		data += 2 * sizeof(uint16_t);
		hash += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
	case 3:
		hash += get16bits(data);
		hash ^= hash << 16;
		hash ^= data[sizeof(uint16_t)] << 18;
		hash += hash >> 11;
		break;
	case 2:
		hash += get16bits(data);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1:
		hash += *data;
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

uint32_t FNVhash32(const char* data, size_t len) {
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < len; i++) {
		hash ^= static_cast<uint8_t>(data[i]);
		hash *= 16777619u;
	}
	return hash;
}

uint32_t hash32(const char* str) {
	assert(str);
	return hash32(str, std::strlen(str));
}

uint32_t hash32(std::string_view str) {
	return hash32(str.data(), str.length());
}

uint64_t hash64(const char* data, size_t len) {
	return std::hash<std::string_view> {}(std::string_view { data, len });
}

} // namespace Typhoon
