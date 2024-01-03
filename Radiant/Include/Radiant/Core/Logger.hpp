#pragma once

#include <spdlog/spdlog.h>

namespace Radiant
{
	inline void LogInit()
	{
		spdlog::set_pattern("%^[%T][Radiant]: %v%$");
	}

	template <typename... Args>
	constexpr void LogDebug(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::debug(InFormat, std::forward<Args>(InArgs)...);
	}

	template<typename... Args>
	constexpr void LogInfo(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::info(InFormat, std::forward<Args>(InArgs)...);
	}

	template <typename... Args>
	constexpr void LogWarn(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::warn(InFormat, std::forward<Args>(InArgs)...);
	}

	template <typename... Args>
	constexpr void LogError(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::error(InFormat, std::forward<Args>(InArgs)...);
	}
}

#define RA_INFO(...) Radiant::LogInfo(__VA_ARGS__);

#define RA_ERROR(...) Radiant::LogError(__VA_ARGS__);
