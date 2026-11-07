#include "ServerLoop.hpp"
#include <thread>
#include <algorithm>

namespace wildbound::server {

ServerLoop::ServerLoop(uint32_t tickRate, uint32_t maxFrameTimeMs) {
    SetTickRate(tickRate);
    m_maxFrameTimeSec = static_cast<float>(maxFrameTimeMs) / 1000.0f;
}

void ServerLoop::SetTickRate(uint32_t hz) {
    m_tickRate = std::clamp(hz, 10u, 120u);
    m_fixedDeltaTime = 1.0f / static_cast<float>(m_tickRate);
}

void ServerLoop::SingleStep(float dt) {
    const auto tickStart = std::chrono::steady_clock::now();

    if (m_networkPoll) {
        m_networkPoll();
    }

    float clampedDt = (dt > m_maxFrameTimeSec) ? m_maxFrameTimeSec : dt;
    uint32_t steps = static_cast<uint32_t>(clampedDt / m_fixedDeltaTime);
    if (steps == 0 && clampedDt > 0.0f) steps = 1;
    bool wasOverload = false;
    if (steps > m_maxCatchUpSteps) {
        steps = m_maxCatchUpSteps;
        wasOverload = true;
    }

    for (uint32_t i = 0; i < steps; ++i) {
        if (m_fixedUpdate) {
            m_fixedUpdate(m_fixedDeltaTime);
        }
    }

    if (m_replicationUpdate) {
        m_replicationUpdate();
    }

    const auto tickEnd = std::chrono::steady_clock::now();
    const float durationMs = std::chrono::duration<float, std::milli>(tickEnd - tickStart).count();

    if (m_diagnosticsUpdate) {
        m_diagnosticsUpdate(dt, durationMs, wasOverload);
    }
}

void ServerLoop::Run() {
    m_running = true;

    using Clock = std::chrono::steady_clock;
    auto previousTime = Clock::now();
    double accumulator = 0.0;

    while (m_running) {
        const auto currentTime = Clock::now();
        double frameTime = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;

        // Clamp frame time to prevent runaway spiral of death if a frame stalls
        if (frameTime > m_maxFrameTimeSec) {
            frameTime = m_maxFrameTimeSec;
        }

        accumulator += frameTime;

        // 1. Poll incoming network transport packets
        if (m_networkPoll) {
            m_networkPoll();
        }

        uint32_t stepsThisFrame = 0;
        bool wasOverload = false;
        const auto tickStart = Clock::now();

        // 2. Fixed-timestep simulation steps
        while (accumulator >= m_fixedDeltaTime) {
            if (stepsThisFrame >= m_maxCatchUpSteps) {
                wasOverload = true;
                accumulator = 0.0; // Drop excess time to preserve bounded CPU usage
                break;
            }

            if (m_fixedUpdate) {
                m_fixedUpdate(m_fixedDeltaTime);
            }

            accumulator -= m_fixedDeltaTime;
            stepsThisFrame++;
        }

        // 3. Outgoing replication update
        if (stepsThisFrame > 0 && m_replicationUpdate) {
            m_replicationUpdate();
        }

        const auto tickEnd = Clock::now();
        const float durationMs = std::chrono::duration<float, std::milli>(tickEnd - tickStart).count();

        // 4. Update diagnostics
        if (m_diagnosticsUpdate) {
            m_diagnosticsUpdate(static_cast<float>(frameTime), durationMs, wasOverload);
        }

        // 5. Yield CPU if we are well ahead of the next tick
        if (accumulator < m_fixedDeltaTime * 0.5) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

} // namespace wildbound::server

