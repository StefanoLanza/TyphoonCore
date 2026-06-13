#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Typhoon {

uint32_t hash32(const char* data, size_t len);
uint32_t hash32(const char* str);
uint32_t hash32(std::string_view str);
uint64_t hash64(const char* data, size_t len);
uint32_t FNVhash32(const char* data, size_t len);

template <class T>
uint32_t hash32(const T& obj) {
	return hash32(reinterpret_cast<const char*>(&obj), sizeof(obj));
}

template <class T>
uint64_t hash64(const T& obj) {
	return hash64(reinterpret_cast<const char*>(&obj), sizeof(obj));
}

// https://nullprogram.com/blog/2018/07/31/, "Prospecting for Hash Functions" by Chris Wellons
inline uint32_t hash32(uint32_t x) {
#if 1 // faster, higher bias
      // exact bias: 0.17353355999581582
      // uint32_t lowbias32(uint32_t x)
      // {
	x ^= x >> 16;
	x *= uint32_t(0x7feb352d);
	x ^= x >> 15;
	x *= uint32_t(0x846ca68b);
	x ^= x >> 16;
	return x;
	//}
#else // slower, lower bias
      // exact bias: 0.020888578919738908
      // uint32_t triple32(uint32_t x)
      // {
	x ^= x >> 17;
	x *= uint32_t(0xed5ad4bb);
	x ^= x >> 11;
	x *= uint32_t(0xac4c1b51);
	x ^= x >> 15;
	x *= uint32_t(0x31848bab);
	x ^= x >> 14;
	return x;
	//}
#endif
}

inline uint32_t hash32Combine(uint32_t seed, uint32_t hash) {
	return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

} // namespace Typhoon
