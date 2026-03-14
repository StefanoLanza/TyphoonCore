#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

namespace Typhoon {

template <size_t L>
class HashedString {
public:
	using Hash = uint32_t;

	HashedString()
	    : m_hash(0) {
		m_str[0] = 0;
	}

	HashedString(const char* str);

	HashedString<L>& operator=(const char*);

	void clear() {
		m_hash = 0;
		m_str[0] = 0;
	}

	bool operator==(const HashedString& str) const {
		return (m_hash == str.m_hash) && (! strcmp(m_str, str.m_str));
	}
	bool operator==(Hash hash) const {
		return m_hash == hash;
	}
	operator const char*() const {
		return m_str;
	}
	const char* str() const {
		return m_str;
	}
	Hash hash() const {
		return m_hash;
	}

private:
	char m_str[L];
	Hash m_hash;
};

uint32_t hash32(const char* str);

} // namespace Typhoon

#include "hashedString.inl"
