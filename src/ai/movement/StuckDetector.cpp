#include "StuckDetector.hpp"
#include <cmath>

namespace wildbound::ai {

StuckDetector::StuckDetector(StuckDetectorConfig config)
    : m_config(config)
{
}

void StuckDetector::Reset() {
    m_checkTimer = 0.0f;
    m_stuckDuration = 0.0f;
    m_isStuck = false;
    m_retryCount = 0;
}

void StuckDetector::Reset(Vector2 currentPos) {
    Reset();
    m_lastCheckPos = currentPos;
}

bool StuckDetector::Update(Vector2 currentPos, Vector2 currentVelocity, float dt) {
    float velSq = currentVelocity.x * currentVelocity.x + currentVelocity.y * currentVelocity.y;

    // If entity isn't trying to move (idle, attack, etc.), it cannot be stuck
    if (velSq < 1.0f) {
        m_stuckDuration = 0.0f;
        m_isStuck = false;
        m_lastCheckPos = currentPos;
        return false;
    }

    m_checkTimer += dt;
    if (m_checkTimer >= m_config.checkInterval) {
        m_checkTimer = 0.0f;

        float dx = currentPos.x - m_lastCheckPos.x;
        float dy = currentPos.y - m_lastCheckPos.y;
        float distSq = dx * dx + dy * dy;
        float minProgSq = m_config.minProgressDistance * m_config.minProgressDistance;

        if (distSq < minProgSq) {
            m_stuckDuration += m_config.checkInterval;
            if (m_stuckDuration >= m_config.stuckTimeThreshold) {
                m_isStuck = true;
                return true;
            }
        } else {
            // Made sufficient progress, reset stuck timer
            m_stuckDuration = 0.0f;
            m_isStuck = false;
        }

        m_lastCheckPos = currentPos;
    }

    return m_isStuck;
}

void StuckDetector::OnPathReplanned() {
    m_retryCount++;
    m_stuckDuration = 0.0f;
    m_isStuck = false;
    m_checkTimer = 0.0f;
}

} // namespace wildbound::ai

