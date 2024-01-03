#pragma once

#include <memory>
#include "Logger.hpp"

#define BUILD_ID "v0.1a"

// __VA_ARGS__ expansion to get past MSVC "bug"
#define RE_EXPAND_VARGS(x) x

#define BIT(x) (1u << x)
#define BIND_FN(fn) [this](auto&&... args) -> decltype(auto) \
{ \
	return this->fn(std::forward<decltype(args)>(args)...); \
}

#if defined(RADIANT_PLATFORM_WINDOWS)
#define RADIANT_DEBUG_BREAK __debugbreak()
#elif defined(RADIANT_PLATFORM_LINUX)
#include <signal.h>
#define RADIANT_DEBUG_BREAK raise(SIGTRAP)
#endif

#define RADIANT_VERIFY(cond, ...) \
	if (!(cond))                                   \
	{                                              \
		Radiant::LogError("Verify failed: {} at {}:{}", #cond, __FILE__, __LINE__); \
		RADIANT_DEBUG_BREAK;                      \
	}

namespace Radiant
{
	template <typename T>
	using Unique = std::unique_ptr<T>;

	template <typename T>
	using Ref = std::shared_ptr<T>;
}