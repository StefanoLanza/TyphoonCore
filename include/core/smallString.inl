#include "hash.h"
#include <cstring>

namespace Typhoon {

template <size_t L>
SmallString<L>::SmallString(const char* str)
    : hash(hash32(storage)) {
	strncpy_s(storage, L, str, _TRUNCATE);
}

template <size_t L>
SmallString<L>& SmallString<L>::operator=(const char* str) {
	strncpy_s(storage, L, str, _TRUNCATE);
	hash = hash32(str);
	return *this;
}

} // namespace Typhoon
