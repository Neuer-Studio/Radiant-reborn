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
		spdlog::set_level(spdlog::level::debug);
		spdlog::debug(InFormat, std::forward<Args>(InArgs)...);
	}

	template<typename... Args>
	constexpr void LogInfo(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::set_level(spdlog::level::info);
		spdlog::info(InFormat, std::forward<Args>(InArgs)...);
	}

	template <typename... Args>
	constexpr void LogWarn(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::set_level(spdlog::level::warn);
		spdlog::warn(InFormat, std::forward<Args>(InArgs)...);
	}

	template <typename... Args>
	constexpr void LogError(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::set_level(spdlog::level::err);
		spdlog::error(InFormat, std::forward<Args>(InArgs)...);
	}

	template <typename... Args>
	constexpr void LogTrace(spdlog::format_string_t<Args...> InFormat, Args&&... InArgs)
	{
		spdlog::set_level(spdlog::level::trace);
		spdlog::trace(InFormat, std::forward<Args>(InArgs)...);
	}
}

#define RA_INFO(...) Radiant::LogInfo(__VA_ARGS__);
#define RA_WARN(...) Radiant::LogWarn(__VA_ARGS__);
#define RA_ERROR(...) Radiant::LogError(__VA_ARGS__);
#define RA_TRACE(...) Radiant::LogTrace(__VA_ARGS__);
#define RA_DEBUG(...) Radiant::LogDebug(__VA_ARGS__);
