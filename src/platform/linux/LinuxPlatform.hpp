#pragma once

#include "../Platform.hpp"

namespace wildbound::platform::linux_os {

class LinuxPlatform final : public IPlatform {
public:
    LinuxPlatform();
    ~LinuxPlatform() override;

    [[nodiscard]] PlatformType GetType() const noexcept override { return PlatformType::Linux; }
    [[nodiscard]] const char* GetName() const noexcept override { return "Linux"; }

    bool RegisterTerminationHandler(TerminationSignalHandler handler) override;
    void UnregisterTerminationHandler() override;
    void ConfigureConsole() override;

    [[nodiscard]] MemoryMetrics GetMemoryMetrics() const noexcept override;
    [[nodiscard]] double GetMonotonicTimeSeconds() const noexcept override;
    void SleepMs(uint32_t ms) override;
    [[nodiscard]] std::string GetEnvironmentVariable(const std::string& name) const override;
};

} // namespace wildbound::platform::linux_os

