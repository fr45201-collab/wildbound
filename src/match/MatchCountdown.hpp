#pragma once

#include <algorithm>

namespace wildbound::match {

/**
 * Pre-Match Countdown Timer Controller.
 */
class MatchCountdown {
public:
    MatchCountdown() = default;
    ~MatchCountdown() = default;

    void Start(float durationSeconds) {
        m_totalDuration = std::max(0.0f, durationSeconds);
        m_timeRemaining = m_totalDuration;
        m_isActive = true;
        m_isPaused = false;
        m_isFinished = (m_totalDuration <= 0.0f);
    }

    void Update(float dt) {
        if (!m_isActive || m_isPaused || m_isFinished) {
            return;
        }

        m_timeRemaining -= dt;
        if (m_timeRemaining <= 0.0f) {
            m_timeRemaining = 0.0f;
            m_isFinished = true;
            m_isActive = false;
        }
    }

    void Pause() noexcept { m_isPaused = true; }
    void Resume() noexcept { m_isPaused = false; }
    void Reset() noexcept {
        m_timeRemaining = 0.0f;
        m_totalDuration = 0.0f;
        m_isActive = false;
        m_isPaused = false;
        m_isFinished = false;
    }

    [[nodiscard]] bool IsActive() const noexcept { return m_isActive; }
    [[nodiscard]] bool IsPaused() const noexcept { return m_isPaused; }
    [[nodiscard]] bool IsFinished() const noexcept { return m_isFinished; }
    [[nodiscard]] float GetTimeRemaining() const noexcept { return m_timeRemaining; }
    [[nodiscard]] float GetTotalDuration() const noexcept { return m_totalDuration; }

    [[nodiscard]] float GetProgress() const noexcept {
        if (m_totalDuration <= 0.0f) return 1.0f;
        return std::clamp(1.0f - (m_timeRemaining / m_totalDuration), 0.0f, 1.0f);
    }

private:
    float m_totalDuration = 10.0f;
    float m_timeRemaining = 0.0f;
    bool m_isActive = false;
    bool m_isPaused = false;
    bool m_isFinished = false;
};

} // namespace wildbound::match

