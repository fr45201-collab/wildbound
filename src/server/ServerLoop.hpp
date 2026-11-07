#pragma once

#include <functional>
#include <chrono>
#include <cstdint>

namespace wildbound::server {

using NetworkPollFn = std::function<void()>;
using FixedUpdateFn = std::function<void(float fixedDt)>;
using ReplicationUpdateFn = std::function<void()>;
using DiagnosticsUpdateFn = std::function<void(float frameDt, float tickDurationMs, bool wasOverload)>;

/**
 * High-precision, fixed-timestep authoritative simulation loop.
 * Implements time accumulation, bounded catch-up steps, and frame clamping.
 */
class ServerLoop {
public:
    ServerLoop(uint32_t tickRate = 30, uint32_t maxFrameTimeMs = 250);
    ~ServerLoop() = default;

    /// Set execution hooks
    void SetNetworkPollHook(NetworkPollFn fn) { m_networkPoll = std::move(fn); }
    void SetFixedUpdateHook(FixedUpdateFn fn) { m_fixedUpdate = std::move(fn); }
    void SetReplicationHook(ReplicationUpdateFn fn) { m_replicationUpdate = std::move(fn); }
    void SetDiagnosticsHook(DiagnosticsUpdateFn fn) { m_diagnosticsUpdate = std::move(fn); }

    /// Run the simulation loop until Stop() is called
    void Run();

    /// Execute a single step (useful for unit testing and deterministic simulation)
    void SingleStep(float dt);

    /// Stop the running loop
    void Stop() noexcept { m_running = false; }
    [[nodiscard]] bool IsRunning() const noexcept { return m_running; }

    [[nodiscard]] uint32_t GetTickRate() const noexcept { return m_tickRate; }
    void SetTickRate(uint32_t hz);

    [[nodiscard]] float GetFixedDeltaTime() const noexcept { return m_fixedDeltaTime; }

private:
    uint32_t m_tickRate{ 30 };
    float m_fixedDeltaTime{ 1.0f / 30.0f };
    float m_maxFrameTimeSec{ 0.25f };
    uint32_t m_maxCatchUpSteps{ 5 };

    bool m_running{ false };

    NetworkPollFn m_networkPoll;
    FixedUpdateFn m_fixedUpdate;
    ReplicationUpdateFn m_replicationUpdate;
    DiagnosticsUpdateFn m_diagnosticsUpdate;
};

} // namespace wildbound::server

