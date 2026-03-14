#include "hash.h"
#include <cstring>

namespace Typhoon {

template <size_t L>
HashedString<L>::HashedString(const char* str)
    : m_hash(hash32(str)) {
	strncpy_s(m_str, L, str, _TRUNCATE);
}

template <size_t L>
HashedString<L>& HashedString<L>::operator=(const char* str) {
	strncpy_s(m_str, L, str, _TRUNCATE);
	m_hash = hash32(str);
	return *this;
}

} // namespace Typhoon
