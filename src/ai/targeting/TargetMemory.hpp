#pragma once

#include <raylib.h>
#include <cstdint>

namespace wildbound::ai {

struct TargetMemoryConfig {
    float memoryDuration = 5.0f; // Seconds before memory expires
};

class TargetMemory {
public:
    explicit TargetMemory(TargetMemoryConfig config = {});
    ~TargetMemory() = default;

    void RecordTarget(uint32_t targetId, Vector2 position, float threat = 1.0f);
    void Update(float dt);
    void Clear();

    bool HasActiveMemory() const noexcept { return m_hasMemory && !m_expired; }
    bool IsExpired() const noexcept { return m_expired; }

    uint32_t GetTargetId() const noexcept { return m_targetId; }
    Vector2 GetLastKnownPosition() const noexcept { return m_lastKnownPosition; }
    float GetTimeSinceLastSeen() const noexcept { return m_timeSinceLastSeen; }
    float GetThreatScore() const noexcept { return m_threatScore; }

    void SetConfig(const TargetMemoryConfig& cfg) { m_config = cfg; }
    const TargetMemoryConfig& GetConfig() const noexcept { return m_config; }

private:
    TargetMemoryConfig m_config;
    uint32_t m_targetId = 0;
    Vector2 m_lastKnownPosition{ 0.0f, 0.0f };
    float m_timeSinceLastSeen = 0.0f;
    float m_threatScore = 1.0f;
    bool m_hasMemory = false;
    bool m_expired = false;
};

} // namespace wildbound::ai

