#include "InvestigateBehaviour.hpp"
#include <cmath>

namespace wildbound::ai {

void InvestigateBehaviour::StartInvestigation(Vector2 targetPos, float searchDuration) {
    m_targetPos = targetPos;
    m_searchDuration = searchDuration;
    m_searchTimer = 0.0f;
    m_lookTimer = 0.0f;
    m_hasArrived = false;
    m_isComplete = false;
    m_searchFacing = graphics::animation::Direction::Down;
}

void InvestigateBehaviour::Update(Vector2 currentPos, float dt) {
    if (m_isComplete) return;

    if (!m_hasArrived) {
        float dx = m_targetPos.x - currentPos.x;
        float dy = m_targetPos.y - currentPos.y;
        float distSq = dx * dx + dy * dy;
        if (distSq <= 24.0f * 24.0f) {
            m_hasArrived = true;
            m_searchTimer = 0.0f;
            m_lookTimer = 0.0f;
        }
    } else {
        // Arrived at investigation position, look around periodically
        m_searchTimer += dt;
        m_lookTimer += dt;

        if (m_lookTimer >= 0.8f) {
            m_lookTimer = 0.0f;
            // Cycle through directions
            switch (m_searchFacing) {
                case graphics::animation::Direction::Down:  m_searchFacing = graphics::animation::Direction::Right; break;
                case graphics::animation::Direction::Right: m_searchFacing = graphics::animation::Direction::Up; break;
                case graphics::animation::Direction::Up:    m_searchFacing = graphics::animation::Direction::Left; break;
                case graphics::animation::Direction::Left:  m_searchFacing = graphics::animation::Direction::Down; break;
            }
        }

        if (m_searchTimer >= m_searchDuration) {
            m_isComplete = true;
        }
    }
}

} // namespace wildbound::ai

