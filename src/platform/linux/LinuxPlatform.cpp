#include "LinuxPlatform.hpp"
#include "LinuxSignals.hpp"

#if defined(__linux__) || (defined(__unix__) && !defined(__APPLE__))
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstdlib>

namespace wildbound::platform::linux_os {

LinuxPlatform::LinuxPlatform() = default;

LinuxPlatform::~LinuxPlatform() {
    UnregisterTerminationHandler();
}

bool LinuxPlatform::RegisterTerminationHandler(TerminationSignalHandler handler) {
    return LinuxSignals::RegisterTerminationHandler(std::move(handler));
}

void LinuxPlatform::UnregisterTerminationHandler() {
    LinuxSignals::UnregisterTerminationHandler();
}

void LinuxPlatform::ConfigureConsole() {
    // Standard Linux terminals use UTF-8 by default; ensures stdout line buffering if needed
    setvbuf(stdout, nullptr, _IOLBF, 0);
}

MemoryMetrics LinuxPlatform::GetMemoryMetrics() const noexcept {
    MemoryMetrics metrics{};

    // Parse /proc/meminfo for host physical RAM
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            std::istringstream iss(line);
            std::string key;
            uint64_t valueKb = 0;
            if (iss >> key >> valueKb) {
                if (key == "MemTotal:") {
                    metrics.totalPhysicalBytes = valueKb * 1024ULL;
                } else if (key == "MemAvailable:") {
                    metrics.availablePhysicalBytes = valueKb * 1024ULL;
                }
            }
        }
    }

    // Parse /proc/self/statm for resident process memory
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        uint64_t sizePages = 0;
        uint64_t residentPages = 0;
        if (statm >> sizePages >> residentPages) {
            long pageSize = sysconf(_SC_PAGE_SIZE);
            if (pageSize > 0) {
                metrics.processPhysicalUsedBytes = residentPages * static_cast<uint64_t>(pageSize);
            }
        }
    }

    return metrics;
}

double LinuxPlatform::GetMonotonicTimeSeconds() const noexcept {
    static const auto startTime = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = now - startTime;
    return elapsed.count();
}

void LinuxPlatform::SleepMs(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string LinuxPlatform::GetEnvironmentVariable(const std::string& name) const {
    const char* val = std::getenv(name.c_str());
    return val ? std::string(val) : std::string();
}

} // namespace wildbound::platform::linux_os

#else // !Linux/Unix

namespace wildbound::platform::linux_os {

LinuxPlatform::LinuxPlatform() = default;
LinuxPlatform::~LinuxPlatform() = default;
bool LinuxPlatform::RegisterTerminationHandler(TerminationSignalHandler) { return false; }
void LinuxPlatform::UnregisterTerminationHandler() {}
void LinuxPlatform::ConfigureConsole() {}
MemoryMetrics LinuxPlatform::GetMemoryMetrics() const noexcept { return {}; }
double LinuxPlatform::GetMonotonicTimeSeconds() const noexcept { return 0.0; }
void LinuxPlatform::SleepMs(uint32_t) {}
std::string LinuxPlatform::GetEnvironmentVariable(const std::string&) const { return {}; }

} // namespace wildbound::platform::linux_os

#endif

