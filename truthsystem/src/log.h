#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/daily_file_sink.h"

class Log
{
public:
	static std::shared_ptr<spdlog::logger> Logger;
public:
	static void initLog(spdlog::level::level_enum level = spdlog::level::info);//The default priority is info. The priority below this level is not displayed.
	static void releaseLog();
	static void setLogLevel(spdlog::level::level_enum level);
};

