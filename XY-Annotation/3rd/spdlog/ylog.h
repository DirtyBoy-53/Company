#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/daily_file_sink.h"

class YLog{
public:
    static std::shared_ptr<spdlog::logger> Logger;
    static void initLog(std::string filename = "log/log.txt",spdlog::level::level_enum level = spdlog::level::info);
    static void releaseLog();
    static void setLogLevel(spdlog::level::level_enum level);
};