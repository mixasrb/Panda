#pragma once

#include <format>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define TAG_GPU "\033[36mGPU\033[0m"
#define TAG_CPU "\033[34mCPU\033[0m"
#define TAG_SPU "\033[35mSPU\033[0m"
#define TAG_CD_DRIVE "\033[32mCD-DRIVE\033[0m"
#define TAG_PERIPHERALS "\033[33mPERIPHERALS\033[0m"

extern bool g_isEmulationPaused;

extern uint32_t g_systemClocksFrame;
extern uint32_t g_systemFrames;
extern uint32_t g_systemSeconds;
extern uint32_t g_systemMinutes;
extern uint32_t g_cpuClocksPerFrame;

namespace dbg {

	struct EmulationTime {
		uint32_t clocksPerFrame;
		uint32_t clocks;
		uint32_t frames;
		uint32_t seconds;
		uint32_t minutes;
	};

	struct EmulationClock {
		static const EmulationTime get_current_time() {
			return { g_cpuClocksPerFrame, g_systemClocksFrame, 
				g_systemFrames, g_systemSeconds, g_systemMinutes };
		}
	};

	class AppClockTimeFlag : public spdlog::custom_flag_formatter {
	public:
		void format(const spdlog::details::log_msg&, 
			const std::tm&, spdlog::memory_buf_t& dest) override {

			EmulationTime time = EmulationClock::get_current_time();

			std::string time_str = std::format("{:02d}:{:02d}:{:02d}:{:06d}/{:06d}",
				time.minutes, time.seconds, time.frames, time.clocks, time.clocksPerFrame);

			dest.append(time_str.data(), time_str.data() + time_str.size());
		}

		std::unique_ptr<custom_flag_formatter> clone() const override {
			return std::make_unique<AppClockTimeFlag>();
		}
	};

	inline void SetLogLevel(spdlog::level::level_enum level) {

		auto formatter = std::make_unique<spdlog::pattern_formatter>();

		// Bind your custom class to the '*' flag`
		formatter->add_flag<AppClockTimeFlag>('*');

		// Set the pattern: [%*] will now inject your app clock string
		formatter->set_pattern("[%*] [%^%l%$] %v");

		spdlog::set_formatter(std::move(formatter));

		auto sink = std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(
			spdlog::default_logger()->sinks().front());
		if (sink) sink->set_color(spdlog::level::trace, FOREGROUND_BLUE | FOREGROUND_INTENSITY);

		spdlog::set_level(level);
	}

	template <class... _Types>
	inline void LogTrace(const std::string& tag,
		const std::format_string<_Types...> fmt, const _Types&... args) {

		//Dissable CD drive trace logs
		if (tag == TAG_CD_DRIVE)
			return;

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