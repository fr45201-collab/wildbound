#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <iostream>

namespace wildbound::core {

enum class LogLevel {
    Debug = 0,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void Init(LogLevel minLevel = LogLevel::Debug);
    static void Shutdown();

    static void SetMinLogLevel(LogLevel level);
    static LogLevel GetMinLogLevel();

    static void Log(LogLevel level, std::string_view subsystem, std::string_view message);

    static void Debug(std::string_view subsystem, std::string_view message);
    static void Info(std::string_view subsystem, std::string_view message);
    static void Warn(std::string_view subsystem, std::string_view message);
    static void Error(std::string_view subsystem, std::string_view message);

private:
    static std::string GetTimestamp();
    static const char* LogLevelToString(LogLevel level);

    inline static LogLevel s_minLevel = LogLevel::Debug;
    inline static std::mutex s_logMutex;
    inline static bool s_initialized = false;
};

} // namespace wildbound::core

