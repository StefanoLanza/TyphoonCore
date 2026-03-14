#pragma once

#include <utility>

namespace Typhoon {

// A simple ScopeGuard that invokes the given callable on destruction.
template <typename F>
class DeferGuard {
public:
	explicit DeferGuard(F&& f) noexcept
	    : func{std::forward<F>(f)} {
	}

	~DeferGuard() noexcept {
		func();
	}

	DeferGuard(DeferGuard&& other) = delete;

	// disable copy
	DeferGuard(const DeferGuard&) = delete;
	DeferGuard& operator=(const DeferGuard&) = delete;

	// disable assignment from move
	DeferGuard& operator=(DeferGuard&&) = delete;

private:
	F func;
};

// Helper to deduce template type
template <typename F>
DeferGuard<F> make_defer(F&& f) noexcept {
	return DeferGuard<F>(std::forward<F>(f));
}

// Macro for ergonomic usage
#define CONCAT_DEFER_VAR(a, b)  _CONCAT_DEFER_VAR(a, b)
#define _CONCAT_DEFER_VAR(a, b) a##b
#define defer(code)             const auto CONCAT_DEFER_VAR(_defer_guard_, __LINE__) = make_defer([&]() { code; })

// Usage:
// {
//     defer { /* cleanup code here */ };
//     // ...
// } // cleanup code runs here

} // namespace Typhoon
