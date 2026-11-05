#pragma once

#include "../navigation/NavigationGrid.hpp"
#include "../../graphics/animation/Direction.hpp"
#include <raylib.h>

namespace wildbound::ai {

struct PerceptionConfig {
    float viewDistance = 160.0f;
    float fieldOfViewDegrees = 120.0f; // Cone of vision when idle/patrolling
    bool omnidirectionalWhenAlert = true;
    bool requireLineOfSight = true;
};

class PerceptionSystem {
public:
    explicit PerceptionSystem(PerceptionConfig config = {});
    ~PerceptionSystem() = default;

    bool CanDetectTarget(Vector2 observerPos,
                         graphics::animation::Direction facing,
                         Vector2 targetPos,
                         bool isAlert,
                         const NavigationGrid& navGrid) const;

    bool CheckDistance(Vector2 observerPos, Vector2 targetPos, float maxDistance) const noexcept;
    bool CheckFieldOfView(Vector2 observerPos, graphics::animation::Direction facing, Vector2 targetPos, float fovDegrees) const noexcept;
    bool CheckLineOfSight(Vector2 observerPos, Vector2 targetPos, const NavigationGrid& navGrid) const noexcept;

    void SetConfig(const PerceptionConfig& cfg) { m_config = cfg; }
    const PerceptionConfig& GetConfig() const noexcept { return m_config; }

private:
    PerceptionConfig m_config;
};

} // namespace wildbound::ai
