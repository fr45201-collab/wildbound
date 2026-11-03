#include "Logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace wildbound::core {

void Logger::Init(LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    s_minLevel = minLevel;
    s_initialized = true;
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(s_logMutex);
    s_initialized = false;
}

void Logger::SetMinLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    s_minLevel = level;
}

LogLevel Logger::GetMinLogLevel() {
    std::lock_guard<std::mutex> lock(s_logMutex);
    return s_minLevel;
}

void Logger::Log(LogLevel level, std::string_view subsystem, std::string_view message) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    if (level < s_minLevel) {
        return;
    }

    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    out << "[" << LogLevelToString(level) << "] "
        << "[" << subsystem << "] "
        << message << std::endl;
}

void Logger::Debug(std::string_view subsystem, std::string_view message) {
    Log(LogLevel::Debug, subsystem, message);
}

void Logger::Info(std::string_view subsystem, std::string_view message) {
    Log(LogLevel::Info, subsystem, message);
}

void Logger::Warn(std::string_view subsystem, std::string_view message) {
    Log(LogLevel::Warning, subsystem, message);
}

void Logger::Error(std::string_view subsystem, std::string_view message) {
    Log(LogLevel::Error, subsystem, message);
}

const char* Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};
#if defined(_WIN32)
    localtime_s(&tmNow, &timeT);
#else
    localtime_r(&timeT, &tmNow);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace wildbound::core

