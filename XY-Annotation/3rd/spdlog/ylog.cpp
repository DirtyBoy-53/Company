#include "ylog.h"
#include <iostream>
std::shared_ptr<spdlog::logger> YLog::Logger = nullptr;

/// <summary>
/// 创建Logger
/// </summary>
void YLog::initLog(std::string filename,spdlog::level::level_enum level)
{
    // default thread pool settings can be modified *before* creating the async Logger:
    spdlog::init_thread_pool(8192, 1); // queue with 8k items and 1 backing thread.
    Logger = spdlog::daily_logger_mt<spdlog::async_factory>("async_daily_logger", filename, 0, 0);
    spdlog::flush_every(std::chrono::seconds(3)); 
    spdlog::flush_on(spdlog::level::err); 
    spdlog::set_level(level); // level
    spdlog::set_pattern("%H:%M:%S.%e [%l] [Thread-%t] %v");
}

/// <summary>
/// 释放Logger
/// </summary>
void YLog::releaseLog()
{
    if (Logger)
        spdlog::drop_all();
}

void YLog::setLogLevel(spdlog::level::level_enum level)
{
    spdlog::set_level(level);
}
