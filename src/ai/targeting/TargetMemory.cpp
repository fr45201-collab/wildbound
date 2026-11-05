#include "TargetMemory.hpp"

namespace wildbound::ai {

TargetMemory::TargetMemory(TargetMemoryConfig config)
    : m_config(config)
{
}

void TargetMemory::RecordTarget(uint32_t targetId, Vector2 position, float threat) {
    m_targetId = targetId;
    m_lastKnownPosition = position;
    m_threatScore = threat;
    m_timeSinceLastSeen = 0.0f;
    m_hasMemory = true;
    m_expired = false;
}

void TargetMemory::Update(float dt) {
    if (!m_hasMemory) return;

    m_timeSinceLastSeen += dt;
    if (m_timeSinceLastSeen >= m_config.memoryDuration) {
        m_expired = true;
    }
}

void TargetMemory::Clear() {
    m_targetId = 0;
    m_lastKnownPosition = Vector2{ 0.0f, 0.0f };
    m_timeSinceLastSeen = 0.0f;
    m_threatScore = 1.0f;
    m_hasMemory = false;
    m_expired = false;
}

} // namespace wildbound::ai

