#include "Platform.hpp"

#if defined(_WIN32)
#include "windows/WindowsPlatform.hpp"
#elif defined(__linux__) || (defined(__unix__) && !defined(__APPLE__))
#include "linux/LinuxPlatform.hpp"
#else
#include <chrono>
#include <thread>
#include <cstdlib>
#endif

namespace wildbound::platform {

#if !defined(_WIN32) && !defined(__linux__) && !(defined(__unix__) && !defined(__APPLE__))
class GenericFallbackPlatform final : public IPlatform {
public:
    [[nodiscard]] PlatformType GetType() const noexcept override { return PlatformType::Unknown; }
    [[nodiscard]] const char* GetName() const noexcept override { return "Generic"; }
    bool RegisterTerminationHandler(TerminationSignalHandler) override { return false; }
    void UnregisterTerminationHandler() override {}
    void ConfigureConsole() override {}
    [[nodiscard]] MemoryMetrics GetMemoryMetrics() const noexcept override { return {}; }
    [[nodiscard]] double GetMonotonicTimeSeconds() const noexcept override {
        static const auto start = std::chrono::steady_clock::now();
        const std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;
        return diff.count();
    }
    void SleepMs(uint32_t ms) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    [[nodiscard]] std::string GetEnvironmentVariable(const std::string& name) const override {
        const char* val = std::getenv(name.c_str());
        return val ? std::string(val) : std::string();
    }
};
#endif

std::unique_ptr<IPlatform> CreatePlatform() {
#if defined(_WIN32)
    return std::make_unique<windows::WindowsPlatform>();
#elif defined(__linux__) || (defined(__unix__) && !defined(__APPLE__))
    return std::make_unique<linux_os::LinuxPlatform>();
#else
    return std::make_unique<GenericFallbackPlatform>();
#endif
}

IPlatform& GetPlatform() {
    static std::unique_ptr<IPlatform> s_instance = CreatePlatform();
    return *s_instance;
}

} // namespace wildbound::platform

