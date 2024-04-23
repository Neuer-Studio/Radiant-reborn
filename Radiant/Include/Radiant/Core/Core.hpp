#pragma once

#include <memory>
#include "Logger.hpp"
#include "Constants.hpp"
#include "Math/UUID.hpp"
#include <Radiant/Core/Memory/Shared.hpp>

#define BUILD_ID "v0.1a"

// __VA_ARGS__ expansion to get past MSVC "bug"
#define RE_EXPAND_VARGS(x) x

#define BIT(x) (1u << x)
#define BIND_FN(fn) [this](auto&&... args) -> decltype(auto) \
{ \
	return this->fn(std::forward<decltype(args)>(args)...); \
}

#define RA_HAS_VALUE(var) var.has_value()

template<typename T>
decltype(auto) initializeDefaultValue() {
	if constexpr (std::is_pointer_v<T>) {
		return nullptr;
	}
	else {
		return T{};
	}
}

#define RA_GET_VALUE(var) var.value_or(initializeDefaultValue<decltype(var)::value_type>())

#if defined(RADIANT_PLATFORM_WINDOWS)
#define RADIANT_DEBUG_BREAK __debugbreak()
#elif defined(RADIANT_PLATFORM_LINUX)
#include <signal.h>
#define RADIANT_DEBUG_BREAK raise(SIGTRAP)
#endif

#define RADIANT_VERIFY(cond, ...) \
	if (!(cond))                                   \
	{                     \
		Radiant::LogError("Verify failed: {} at {}:{}", #cond, __FILE__, __LINE__); \
		__debugbreak();                     \
	}


namespace Radiant
{
	template <typename T>
	using Unique = std::unique_ptr<T>;
}