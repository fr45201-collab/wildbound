#pragma once

#include "../Platform.hpp"

namespace wildbound::platform::windows {

class WindowsPlatform final : public IPlatform {
public:
    WindowsPlatform();
    ~WindowsPlatform() override;

    [[nodiscard]] PlatformType GetType() const noexcept override { return PlatformType::Windows; }
    [[nodiscard]] const char* GetName() const noexcept override { return "Windows"; }

    bool RegisterTerminationHandler(TerminationSignalHandler handler) override;
    void UnregisterTerminationHandler() override;
    void ConfigureConsole() override;

    [[nodiscard]] MemoryMetrics GetMemoryMetrics() const noexcept override;
    [[nodiscard]] double GetMonotonicTimeSeconds() const noexcept override;
    void SleepMs(uint32_t ms) override;
    [[nodiscard]] std::string GetEnvironmentVariable(const std::string& name) const override;
};

} // namespace wildbound::platform::windows

