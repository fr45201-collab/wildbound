#pragma once

#include "../navigation/NavigationQuery.hpp"
#include "../../graphics/animation/Direction.hpp"
#include <raylib.h>
#include <vector>

namespace wildbound::ai {

struct MovementControllerConfig {
    float waypointArrivalRadius = 16.0f;
    float goalArrivalRadius = 24.0f;
    float cornerSlowdownFactor = 0.8f;
    bool enableAvoidance = true;
};

class AIMovementController {
public:
    explicit AIMovementController(MovementControllerConfig config = {});
    ~AIMovementController() = default;

    void SetPath(const PathResult& path);
    void ClearPath();

    bool HasActivePath() const noexcept { return m_hasActivePath; }
    bool HasReachedDestination() const noexcept { return m_reachedDestination; }

    size_t GetCurrentWaypointIndex() const noexcept { return m_currentWaypointIndex; }
    size_t GetTotalWaypoints() const noexcept { return m_waypoints.size(); }
    Vector2 GetCurrentWaypoint() const noexcept;
    Vector2 GetFinalDestination() const noexcept;
    const std::vector<Vector2>& GetWaypoints() const noexcept { return m_waypoints; }

    // Computes desired movement velocity given current entity position, move speed, and avoidance force
    Vector2 Update(Vector2 currentPos, float moveSpeed, float dt, Vector2 avoidanceForce = { 0.0f, 0.0f });

    graphics::animation::Direction GetDesiredFacing() const noexcept { return m_desiredFacing; }

    void SetConfig(const MovementControllerConfig& cfg) { m_config = cfg; }
    const MovementControllerConfig& GetConfig() const noexcept { return m_config; }

private:
    MovementControllerConfig m_config;
    std::vector<Vector2> m_waypoints;
    size_t m_currentWaypointIndex = 0;
    bool m_hasActivePath = false;
    bool m_reachedDestination = false;
    graphics::animation::Direction m_desiredFacing = graphics::animation::Direction::Down;
};

} // namespace wildbound::ai
