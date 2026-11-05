#pragma once

#include <raylib.h>

namespace wildbound::ai {

struct StuckDetectorConfig {
    float checkInterval = 0.5f;
    float minProgressDistance = 6.0f;
    float stuckTimeThreshold = 1.2f;
    int maxRetryCount = 3;
};

class StuckDetector {
public:
    explicit StuckDetector(StuckDetectorConfig config = {});
    ~StuckDetector() = default;

    void Reset();
    void Reset(Vector2 currentPos);

    bool Update(Vector2 currentPos, Vector2 currentVelocity, float dt);

    bool IsStuck() const noexcept { return m_isStuck; }
    bool HasExceededMaxRetries() const noexcept { return m_retryCount >= m_config.maxRetryCount; }
    int GetRetryCount() const noexcept { return m_retryCount; }

    void OnPathReplanned();

    void SetConfig(const StuckDetectorConfig& cfg) { m_config = cfg; }
    const StuckDetectorConfig& GetConfig() const noexcept { return m_config; }

private:
    StuckDetectorConfig m_config;
    Vector2 m_lastCheckPos{ 0.0f, 0.0f };
    float m_checkTimer = 0.0f;
    float m_stuckDuration = 0.0f;
    int m_retryCount = 0;
    bool m_isStuck = false;
};

} // namespace wildbound::ai

