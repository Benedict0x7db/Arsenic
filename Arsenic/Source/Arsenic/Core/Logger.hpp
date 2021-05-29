#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace arsenic 
{
	namespace logger
	{
		std::shared_ptr<spdlog::logger> getLogger();
	}
}

#ifndef NDEBUG
	#define ARSENIC_TRACE(...)         ::arsenic::logger::getLogger()->trace(__VA_ARGS__)
	#define ARSENIC_INFO(...)          ::arsenic::logger::getLogger()->info(__VA_ARGS__)
	#define ARSENIC_WARN(...)          ::arsenic::logger::getLogger()->warn(__VA_ARGS__)
	#define ARSENIC_ERROR(...)         ::arsenic::logger::getLogger()->error(__VA_ARGS__)
	#define ARSENIC_CRITICAL(...)      ::arsenic::logger::getLogger()->critical(__VA_ARGS__)
#else
	#define ARSENIC_TRACE(...)        
	#define ARSENIC_INFO(...)          
	#define ARSENIC_WARN(...)         
	#define ARSENIC_ERROR(...)        
	#define ARSENIC_CRITICAL(...)    
#endif