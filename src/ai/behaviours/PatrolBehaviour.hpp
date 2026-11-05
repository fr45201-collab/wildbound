#pragma once

#include "../core/AIConfiguration.hpp"
#include <raylib.h>
#include <vector>

namespace wildbound::ai {

class PatrolBehaviour {
public:
    PatrolBehaviour() = default;
    ~PatrolBehaviour() = default;

    void SetRoute(const PatrolRoute& route);
    void SetWanderZone(Vector2 center, float radius);

    Vector2 GetCurrentTargetPosition() const noexcept;
    bool Update(Vector2 currentPos, float dt);
    void Reset();

    size_t GetCurrentIndex() const noexcept { return m_currentIndex; }
    bool IsWaiting() const noexcept { return m_isWaiting; }
    float GetWaitTimer() const noexcept { return m_waitTimer; }

private:
    PatrolRoute m_route;
    Vector2 m_wanderCenter{ 0.0f, 0.0f };
    float m_wanderRadius = 80.0f;
    bool m_useRoute = false;

    size_t m_currentIndex = 0;
    float m_waitTimer = 0.0f;
    bool m_isWaiting = false;
    Vector2 m_currentTarget{ 0.0f, 0.0f };
};

} // namespace wildbound::ai

