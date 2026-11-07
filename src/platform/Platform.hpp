#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

namespace wildbound::platform {

enum class PlatformType {
    Windows,
    Linux,
    Unknown
};

struct MemoryMetrics {
    uint64_t totalPhysicalBytes{ 0 };
    uint64_t availablePhysicalBytes{ 0 };
    uint64_t processPhysicalUsedBytes{ 0 };
};

using TerminationSignalHandler = std::function<void(const std::string& reason)>;

/**
 * Cross-platform abstraction interface for operating system primitives.
 * Encapsulates OS signal handling, system telemetry, console initialization,
 * and high-resolution platform metrics without leaking platform headers.
 */
class IPlatform {
public:
    virtual ~IPlatform() = default;

    [[nodiscard]] virtual PlatformType GetType() const noexcept = 0;
    [[nodiscard]] virtual const char* GetName() const noexcept = 0;

    /// Register OS termination signal handler (Ctrl+C, SIGINT, SIGTERM, SIGHUP, shutdown)
    virtual bool RegisterTerminationHandler(TerminationSignalHandler handler) = 0;
    virtual void UnregisterTerminationHandler() = 0;

    /// Configure standard console environment (UTF-8 encoding, virtual terminal mode)
    virtual void ConfigureConsole() = 0;

    /// Retrieve host and process physical memory metrics
    [[nodiscard]] virtual MemoryMetrics GetMemoryMetrics() const noexcept = 0;

    /// High-resolution monotonic timestamp in fractional seconds
    [[nodiscard]] virtual double GetMonotonicTimeSeconds() const noexcept = 0;

    /// Sleep / yield execution for the specified milliseconds
    virtual void SleepMs(uint32_t ms) = 0;

    /// Retrieve environment variable value, or empty string if not set
    [[nodiscard]] virtual std::string GetEnvironmentVariable(const std::string& name) const = 0;
};

/// Factory function creating the appropriate platform implementation for current target
std::unique_ptr<IPlatform> CreatePlatform();

/// Access the shared platform instance
IPlatform& GetPlatform();

} // namespace wildbound::platform

