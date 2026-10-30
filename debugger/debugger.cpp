#include "debugger.h"

#include "../dependencies/spdlog/spdlog.h"

bool g_isEmulationPaused = false;

template <class... _Types>
void dbg::Log(const Level level, const std::string& tag,
    const std::format_string<_Types...> fmt, _Types&&... args) {
	
    switch (level) {
	case dbg::Level::TRACE:
		spdlog::trace("[{}] {}", tag, std::vformat(fmt, std::make_format_args(args...)));
		break;
	/*case dbg::Level::DEBUG:
		spdlog::debug("[{}] {}", tag, std::vformat(fmt, std::make_format_args(args...)));
		break;*/
	case dbg::Level::INFO:
		spdlog::info("[{}] {}", tag, std::vformat(fmt, std::make_format_args(args...)));
		break;
	case dbg::Level::WARN:
		spdlog::warn("[{}] {}", tag, std::vformat(fmt, std::make_format_args(args...)));
		break;
	/*case dbg::Level::ERROR:
		spdlog::error("[{}] {}", tag, std::vformat(fmt, std::make_format_args(args...)));
		break;*/
	case dbg::Level::CRITICAL:
		spdlog::critical("[{}] {}", tag, std::vformat(fmt, std::make_format_args(args...)));
		g_isEmulationPaused = true;
		break;
    }
}

void dbg::Main() {
    spdlog::info("Welcome to spdlog!");
    spdlog::error("Some error message with arg: {}", 1);

    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    spdlog::info("Support for floats {:03.2f}", 1.23456);
    spdlog::info("Positional args are {1} {0}..", "too", "supported");
    spdlog::info("{:<30}", "left aligned");

    spdlog::set_level(spdlog::level::debug); // Set *global* log level to debug
    spdlog::debug("This message should be displayed..");

    // change log pattern
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

    // Compile time log levels
    // Note that this does not change the current log level, it will only
    // remove (depending on SPDLOG_ACTIVE_LEVEL) the call on the release code.
    SPDLOG_TRACE("Some trace message with param {}", 42);
    SPDLOG_DEBUG("Some debug message");
}
