#include "WindowsPlatform.hpp"
#include "WindowsConsole.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#include <windows.h>
#include <psapi.h>

#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif

#include <chrono>
#include <thread>
#include <cstdlib>

namespace wildbound::platform::windows {

WindowsPlatform::WindowsPlatform() = default;

WindowsPlatform::~WindowsPlatform() {
    UnregisterTerminationHandler();
}

bool WindowsPlatform::RegisterTerminationHandler(TerminationSignalHandler handler) {
    return WindowsConsole::RegisterTerminationHandler(std::move(handler));
}

void WindowsPlatform::UnregisterTerminationHandler() {
    WindowsConsole::UnregisterTerminationHandler();
}

void WindowsPlatform::ConfigureConsole() {
    WindowsConsole::ConfigureConsole();
}

MemoryMetrics WindowsPlatform::GetMemoryMetrics() const noexcept {
    MemoryMetrics metrics{};

    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        metrics.totalPhysicalBytes = memStatus.ullTotalPhys;
        metrics.availablePhysicalBytes = memStatus.ullAvailPhys;
    }

    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        metrics.processPhysicalUsedBytes = pmc.WorkingSetSize;
    }

    return metrics;
}

double WindowsPlatform::GetMonotonicTimeSeconds() const noexcept {
    static const auto startTime = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = now - startTime;
    return elapsed.count();
}

void WindowsPlatform::SleepMs(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string WindowsPlatform::GetEnvironmentVariable(const std::string& name) const {
    char buffer[1024];
    DWORD len = ::GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));
    if (len > 0 && len < sizeof(buffer)) {
        return std::string(buffer, len);
    }
    return {};
}

} // namespace wildbound::platform::windows

#else // !_WIN32

namespace wildbound::platform::windows {

WindowsPlatform::WindowsPlatform() = default;
WindowsPlatform::~WindowsPlatform() = default;
bool WindowsPlatform::RegisterTerminationHandler(TerminationSignalHandler) { return false; }
void WindowsPlatform::UnregisterTerminationHandler() {}
void WindowsPlatform::ConfigureConsole() {}
MemoryMetrics WindowsPlatform::GetMemoryMetrics() const noexcept { return {}; }
double WindowsPlatform::GetMonotonicTimeSeconds() const noexcept { return 0.0; }
void WindowsPlatform::SleepMs(uint32_t) {}
std::string WindowsPlatform::GetEnvironmentVariable(const std::string&) const { return {}; }

} // namespace wildbound::platform::windows

#endif
