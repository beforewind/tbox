#ifndef TLOGGER_H
#define TLOGGER_H

#include <string>
#include <corecrt_io.h>

#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif

#ifndef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#endif

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define UTIL_DLL_API

namespace utils
{
    class UTIL_DLL_API TLogger
    {
    public:
        auto GetLogger()
        {
            return nml_logger;
        }

        TLogger();
        ~TLogger();
        TLogger(const TLogger &) = delete;
        TLogger &operator=(const TLogger &) = delete;

    private:
        std::shared_ptr<spdlog::logger> nml_logger;
    };
}
UTIL_DLL_API utils::TLogger &GetInstance();

#define SPDLOG_LOGGER_CALL_(level, ...) GetInstance().GetLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
#define LogTrace(...) SPDLOG_LOGGER_CALL_(spdlog::level::trace, __VA_ARGS__)
#define LogDebug(...) SPDLOG_LOGGER_CALL_(spdlog::level::debug, __VA_ARGS__)
#define LogInfo(...) SPDLOG_LOGGER_CALL_(spdlog::level::info, __VA_ARGS__)
#define LogWarn(...) SPDLOG_LOGGER_CALL_(spdlog::level::warn, __VA_ARGS__)
#define LogError(...) SPDLOG_LOGGER_CALL_(spdlog::level::err, __VA_ARGS__)
#define LogCritical(...) SPDLOG_LOGGER_CALL_(spdlog::level::critical, __VA_ARGS__)
#define LogCriticalIf(b, ...)                                          \
    do                                                                 \
    {                                                                  \
        if ((b))                                                       \
        {                                                              \
            SPDLOG_LOGGER_CALL_(spdlog::level::critical, __VA_ARGS__); \
        }                                                              \
    } while (0)

// 缺省使用两级TDEBUG和TERROR //
// TDEBUG-用于提供调试信息
// TERROR-用于提供错误信息
#define TTRACE(...) SPDLOG_LOGGER_CALL_(spdlog::level::trace, __VA_ARGS__)
#define TDEBUG(...) SPDLOG_LOGGER_CALL_(spdlog::level::debug, __VA_ARGS__)
#define TINFO(...) SPDLOG_LOGGER_CALL_(spdlog::level::info, __VA_ARGS__)
#define TWARN(...) SPDLOG_LOGGER_CALL_(spdlog::level::warn, __VA_ARGS__)
#define TERROR(...) SPDLOG_LOGGER_CALL_(spdlog::level::err, __VA_ARGS__)
#define TCRITICAL(...) SPDLOG_LOGGER_CALL_(spdlog::level::critical, __VA_ARGS__)

#if 0
std::shared_ptr<spdlog::logger> file_logger;
#define TDEBUG(...)                                                 \
    SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__); \
    SPDLOG_LOGGER_DEBUG(spdlog::get("file_logger"), __VA_ARGS__)
#define TLOG(...)                                                  \
    SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__); \
    SPDLOG_LOGGER_INFO(spdlog::get("file_logger"), __VA_ARGS__)
#define TWARN(...)                                                 \
    SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__); \
    SPDLOG_LOGGER_WARN(spdlog::get("file_logger"), __VA_ARGS__)
#define TERROR(...)                                                 \
    SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__); \
    SPDLOG_LOGGER_ERROR(spdlog::get("file_logger"), __VA_ARGS__)
#endif

#ifdef WIN32
#define errcode WSAGetLastError()
#endif

#endif // TLOGGER_H
