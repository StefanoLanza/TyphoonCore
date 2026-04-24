#include "hash.h"
#include <algorithm>
#include <cassert>
#include <cstring>

namespace Typhoon {

template <size_t L>
SmallString<L>::SmallString(const char* str) {
	*this = str;
}

template <size_t L>
SmallString<L>::SmallString(std::string_view sv) {
	*this = sv;
}

template <size_t L>
SmallString<L>& SmallString<L>::operator=(const char* str) {
	strncpy_s(storage, L, str, _TRUNCATE);
	hash = hash32(str);
	return *this;
}

template <size_t L>
SmallString<L>& SmallString<L>::operator=(std::string_view sv) {
	size_t copyLen = std::min(sv.size(), L - 1);
	std::memcpy(storage, sv.data(), copyLen);
	storage[copyLen] = '\0';
	hash = hash32(str);
	return *this;
}

} // namespace Typhoon
