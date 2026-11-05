#include "PatrolBehaviour.hpp"
#include <cmath>

namespace wildbound::ai {

void PatrolBehaviour::SetRoute(const PatrolRoute& route) {
    m_route = route;
    m_useRoute = !m_route.points.empty();
    m_currentIndex = 0;
    m_isWaiting = false;
    m_waitTimer = 0.0f;
    if (m_useRoute) {
        m_currentTarget = m_route.points[0].position;
    }
}

void PatrolBehaviour::SetWanderZone(Vector2 center, float radius) {
    m_wanderCenter = center;
    m_wanderRadius = radius;
    m_useRoute = false;
    m_currentTarget = center;
}

Vector2 PatrolBehaviour::GetCurrentTargetPosition() const noexcept {
    return m_currentTarget;
}

bool PatrolBehaviour::Update(Vector2 currentPos, float dt) {
    if (m_isWaiting) {
        m_waitTimer -= dt;
        if (m_waitTimer <= 0.0f) {
            m_isWaiting = false;

            if (m_useRoute && !m_route.points.empty()) {
                m_currentIndex = (m_currentIndex + 1) % m_route.points.size();
                m_currentTarget = m_route.points[m_currentIndex].position;
            } else {
                // Pick next random wander point within radius
                float angle = static_cast<float>(rand() % 360) * (3.14159265f / 180.0f);
                float dist = static_cast<float>(rand() % static_cast<int>(m_wanderRadius > 10.0f ? m_wanderRadius : 10.0f));
                m_currentTarget = Vector2{
                    m_wanderCenter.x + std::cos(angle) * dist,
                    m_wanderCenter.y + std::sin(angle) * dist
                };
            }
            return true; // Target changed
        }
        return false;
    }

    // Check arrival at current patrol point
    float dx = m_currentTarget.x - currentPos.x;
    float dy = m_currentTarget.y - currentPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= 24.0f) {
        m_isWaiting = true;
        float waitDuration = 2.0f;
        if (m_useRoute && m_currentIndex < m_route.points.size()) {
            waitDuration = m_route.points[m_currentIndex].waitDuration;
        }
        m_waitTimer = waitDuration;
    }

    return false;
}

void PatrolBehaviour::Reset() {
    m_currentIndex = 0;
    m_waitTimer = 0.0f;
    m_isWaiting = false;
    if (m_useRoute && !m_route.points.empty()) {
        m_currentTarget = m_route.points[0].position;
    } else {
        m_currentTarget = m_wanderCenter;
    }
}

} // namespace wildbound::ai

