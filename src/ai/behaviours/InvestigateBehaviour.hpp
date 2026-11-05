#pragma once

#include <raylib.h>
#include "../../graphics/animation/Direction.hpp"

namespace wildbound::ai {

class InvestigateBehaviour {
public:
    InvestigateBehaviour() = default;
    ~InvestigateBehaviour() = default;

    void StartInvestigation(Vector2 targetPos, float searchDuration = 3.0f);
    void Update(Vector2 currentPos, float dt);
    bool IsComplete() const noexcept { return m_isComplete; }

    Vector2 GetInvestigatePosition() const noexcept { return m_targetPos; }
    bool HasArrived() const noexcept { return m_hasArrived; }
    graphics::animation::Direction GetSearchFacing() const noexcept { return m_searchFacing; }

private:
    Vector2 m_targetPos{ 0.0f, 0.0f };
    float m_searchDuration = 3.0f;
    float m_searchTimer = 0.0f;
    float m_lookTimer = 0.0f;
    bool m_hasArrived = false;
    bool m_isComplete = false;
    graphics::animation::Direction m_searchFacing = graphics::animation::Direction::Down;
};

} // namespace wildbound::ai
