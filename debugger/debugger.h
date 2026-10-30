#pragma once

#include <format>

namespace dbg {

	enum Level :uint8_t {
		TRACE = 0,
		//DEBUG = 1,
		INFO = 2,
		WARN = 3,
		//ERROR = 4,
		CRITICAL = 5
	};

	template <class... _Types>
	void Log(const Level level, const std::string& tag, const std::format_string<_Types...> fmt, _Types&&... args);

	void Main();
}