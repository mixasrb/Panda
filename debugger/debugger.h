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

	enum Level :uint8_t {
		TRACE = 0,
		//DEBUG = 1,
		INFO = 2,
		WARN = 3,
		//ERROR = 4,
		CRITICAL = 5
	};

	template <class... _Types>
	inline void Log(const Level level, const std::string& tag,
		const std::format_string<_Types...> fmt, _Types&&... args) {

		switch (level) {
		case Level::TRACE:
			spdlog::trace("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
			break;
		/* case Level::DEBUG:
			spdlog::debug("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
			break; */
		case Level::INFO:
			spdlog::info("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
			break;
		case Level::WARN:
			spdlog::warn("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
			break;
		/* case Level::ERROR:
			spdlog::error("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
			break; */
		case Level::CRITICAL:
			spdlog::critical("[{}] {}", tag, std::vformat(fmt.get(), std::make_format_args(args...)));
			g_isEmulationPaused = true;
			break;
		}
	}
}

using namespace dbg;