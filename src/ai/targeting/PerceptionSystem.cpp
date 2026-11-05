#include "PerceptionSystem.hpp"
#include <cmath>

namespace wildbound::ai {

PerceptionSystem::PerceptionSystem(PerceptionConfig config)
    : m_config(config)
{
}

bool PerceptionSystem::CheckDistance(Vector2 observerPos, Vector2 targetPos, float maxDistance) const noexcept {
    float dx = targetPos.x - observerPos.x;
    float dy = targetPos.y - observerPos.y;
    return (dx * dx + dy * dy) <= (maxDistance * maxDistance);
}

bool PerceptionSystem::CheckFieldOfView(Vector2 observerPos,
                                        graphics::animation::Direction facing,
                                        Vector2 targetPos,
                                        float fovDegrees) const noexcept
{
    if (fovDegrees >= 360.0f) return true;

    float dx = targetPos.x - observerPos.x;
    float dy = targetPos.y - observerPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.001f) return true;

    Vector2 toTarget{ dx / dist, dy / dist };
    Vector2 facingVec = DirectionToVector(facing);

    float dot = facingVec.x * toTarget.x + facingVec.y * toTarget.y;

    // Half angle in radians
    float halfAngleRad = (fovDegrees * 0.5f) * (3.14159265f / 180.0f);
    float minCos = std::cos(halfAngleRad);

    return dot >= minCos;
}

bool PerceptionSystem::CheckLineOfSight(Vector2 observerPos, Vector2 targetPos, const NavigationGrid& navGrid) const noexcept {
    return navGrid.HasLineOfSight(observerPos, targetPos);
}

bool PerceptionSystem::CanDetectTarget(Vector2 observerPos,
                                      graphics::animation::Direction facing,
                                      Vector2 targetPos,
                                      bool isAlert,
                                      const NavigationGrid& navGrid) const
{
    // 1. Distance check
    if (!CheckDistance(observerPos, targetPos, m_config.viewDistance)) {
        return false;
    }

    // 2. Field of view check (omnidirectional when alert)
    if (!isAlert || !m_config.omnidirectionalWhenAlert) {
        if (!CheckFieldOfView(observerPos, facing, targetPos, m_config.fieldOfViewDegrees)) {
            return false;
        }
    }

    // 3. Line of sight check
    if (m_config.requireLineOfSight) {
        if (!CheckLineOfSight(observerPos, targetPos, navGrid)) {
            return false;
        }
    }

    return true;
}

} // namespace wildbound::ai
