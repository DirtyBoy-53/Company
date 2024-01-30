#include "log.h"

std::shared_ptr<spdlog::logger> Log::Logger = nullptr;

/// <summary>
/// logger
/// </summary>
void Log::initLog(spdlog::level::level_enum level)
{
    // default thread pool settings can be modified *before* creating the async logger:
    spdlog::init_thread_pool(8192, 1); // queue with 8k items and 1 backing thread.
    Logger = spdlog::daily_logger_mt<spdlog::async_factory>("async_daily_logger", "log/log.txt", 0, 0);
    spdlog::flush_every(std::chrono::seconds(3)); 
    spdlog::flush_on(spdlog::level::err); 
    spdlog::set_level(level); // level
    spdlog::set_pattern("%H:%M:%S.%e [%l] [Thread-%t] %v");
}

/// <summary>
/// logger
/// </summary>
void Log::releaseLog()
{
    if (Logger)
        spdlog::drop_all();
}

void Log::setLogLevel(spdlog::level::level_enum level)
{
    spdlog::set_level(level);
}
