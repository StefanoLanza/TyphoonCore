#pragma once

#include <type_traits>

namespace Typhoon {

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || \
    (!defined(_MSVC_LANG) && __cplusplus >= 202002L)

template <class T>
concept isComplete = requires { sizeof(T); };

#else

template <class T, class = void>
struct IsComplete : std::false_type {};

template <class T>
struct IsComplete<T, std::void_t<decltype(sizeof(T))>>
    : std::true_type {};

template <class T>
inline constexpr bool isComplete = IsComplete<T>::value;

#endif

} // namespace Typhoon
