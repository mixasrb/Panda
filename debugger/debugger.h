#pragma once

#include <format>

#include "../dependencies/spdlog/spdlog.h"
#include "../dependencies/spdlog/sinks/stdout_color_sinks.h"

extern bool g_isEmulationPaused;

namespace dbg {

	inline void SetLogLevel(spdlog::level::level_enum level) {
		auto sink = std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(
			spdlog::default_logger()->sinks().front());
		if (sink) sink->set_color(spdlog::level::trace, FOREGROUND_BLUE | FOREGROUND_INTENSITY);

		spdlog::set_level(level);
	}

	template <class... _Types>
	inline void LogTrace(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		spdlog::trace("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
	}

	template <class... _Types>
	inline void LogDebug(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		spdlog::debug("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
	}

	template <class... _Types>
	inline void LogInfo(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		spdlog::info("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
	}

	template <class... _Types>
	inline void LogWarn(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		spdlog::warn("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
		g_isEmulationPaused = true;
	}

	template <class... _Types>
	inline void LogError(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		spdlog::error("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
		g_isEmulationPaused = true;
	}

	template <class... _Types>
	inline void LogCritical(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		spdlog::critical("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
		g_isEmulationPaused = true;
	}
}

using namespace dbg;