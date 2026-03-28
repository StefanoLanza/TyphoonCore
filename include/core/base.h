#pragma once

#include <cstdint>

#if defined(_MSC_VER)
	#define RESTRICT __restrict
	#define FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
	#define RESTRICT __restrict__
	#define FORCE_INLINE inline __attribute__((always_inline))
#else
	#define RESTRICT
	#define FORCE_INLINE
#endif

#define UNUSED(x) [[maybe_unused]] x

#define TY_DEPRECATED __declspec(deprecated)

namespace Typhoon {

using u_char = unsigned char;
using uchar = unsigned char;
using u_short = unsigned short;
using ushort = unsigned short;
using u_int = unsigned int;
using uint = unsigned int;
using u_long = unsigned long;
using ulong = unsigned long;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using f32 = float;
using f64 = double;
using byte = uint8_t;
using word = uint16_t;
using dword = uint32_t;

} // namespace Typhoon

#define MacroStr(x)        #x
#define MacroStr2(x)       MacroStr(x)
#define MacroMessage(desc) __pragma(message(__FILE__ "(" MacroStr2(__LINE__) ") :" desc))

//---------------------------------------------------------------------------------------------
// FIXMEs / TODOs / NOTE macros
//---------------------------------------------------------------------------------------------
#define _QUOTE(x)      #x
#define QUOTE(x)       _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define FILE_LINE MacroMessage(__FILE__LINE__)

#define NOTE(x)  MacroMessage("NOTE : " #x)
#define TODO(x)  MacroMessage("TODO :   " #x)
#define FIXME(x) MacroMessage("FIXME :   " #x)

#define RUN_ONCE(code)                                             \
	__pragma(warning(push)) __pragma(warning(disable : 4127)) do { \
		static bool done = 0;                                      \
		if (! done) {                                              \
			done = 1;                                              \
			code;                                                  \
		}                                                          \
	}                                                              \
	while (0)                                                      \
	__pragma(warning(pop))

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)     \
	do {                    \
		if (p) {            \
			(p)->Release(); \
			(p) = NULL;     \
		}                   \
	while (false) }
#endif

#define TOKENPASTE(x, y)  x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#ifndef TY_BIT
#define TY_BIT(i) (1u << (unsigned)(i))
#endif
