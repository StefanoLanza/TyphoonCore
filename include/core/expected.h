#pragma once

#include <version>
#ifdef __cpp_lib_expected
#include <expected>

template <typename T, typename E>
using Expected = std::expected<T, E>;

#define UNEXPECTED std::unexpected

#else

#include <core/expected.hpp>

template <typename T, typename E>
using Expected = tl::expected<T, E>;

#define UNEXPECTED tl::make_unexpected

#endif
