#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Core/Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace arsenic 
{
namespace logger
{
	static std::shared_ptr<spdlog::logger> s_logger = nullptr;

	static void init()
    {
		assert(s_logger == nullptr);

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("arsenic.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_logger = std::make_shared<spdlog::logger>("arsenic", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_logger);
		s_logger->set_level(spdlog::level::trace);
		s_logger->flush_on(spdlog::level::trace);
	}

	std::shared_ptr<spdlog::logger> getLogger()
    {
		if (!s_logger) {
			init();
		} 

		return s_logger;
    }
}
}

