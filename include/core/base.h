#pragma once

#include <cstdint>

#define RESTRICT __restrict

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

#define UNUSED(x) [[maybe_unused]] x

#define TY_DEPRECATED __declspec(deprecated)

namespace Typhoon {

typedef unsigned char  u_char;
typedef unsigned char  uchar;
typedef unsigned short u_short;
typedef unsigned short ushort;
typedef unsigned int   u_int;
typedef unsigned int   uint;
typedef unsigned long  u_long;
typedef unsigned long  ulong;
typedef int8_t         int8;
typedef int16_t        int16;
typedef int32_t        int32;
typedef int64_t        int64;
typedef uint8_t        uint8;
typedef uint16_t       uint16;
typedef uint32_t       uint32;
typedef uint64_t       uint64;
typedef float          f32;
typedef double         f64;
typedef uint8_t        byte;
typedef uint16_t       word;
typedef uint32_t       dword;

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
