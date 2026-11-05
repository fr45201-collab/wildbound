#include "AIMovementController.hpp"
#include <cmath>

namespace wildbound::ai {

AIMovementController::AIMovementController(MovementControllerConfig config)
    : m_config(config)
{
}

void AIMovementController::SetPath(const PathResult& path) {
    m_waypoints = path.waypoints;
    m_currentWaypointIndex = 0;
    m_hasActivePath = !m_waypoints.empty();
    m_reachedDestination = m_waypoints.empty();

    // If starting point is very close to initial waypoint, skip to next
    if (m_waypoints.size() > 1) {
        m_currentWaypointIndex = 1;
    }
}

void AIMovementController::ClearPath() {
    m_waypoints.clear();
    m_currentWaypointIndex = 0;
    m_hasActivePath = false;
    m_reachedDestination = true;
}

Vector2 AIMovementController::GetCurrentWaypoint() const noexcept {
    if (m_currentWaypointIndex < m_waypoints.size()) {
        return m_waypoints[m_currentWaypointIndex];
    }
    if (!m_waypoints.empty()) {
        return m_waypoints.back();
    }
    return Vector2{ 0.0f, 0.0f };
}

Vector2 AIMovementController::GetFinalDestination() const noexcept {
    if (!m_waypoints.empty()) {
        return m_waypoints.back();
    }
    return Vector2{ 0.0f, 0.0f };
}

Vector2 AIMovementController::Update(Vector2 currentPos, float moveSpeed, float dt, Vector2 avoidanceForce) {
    (void)dt;

    if (!m_hasActivePath || m_waypoints.empty() || m_currentWaypointIndex >= m_waypoints.size()) {
        m_reachedDestination = true;
        return Vector2{ 0.0f, 0.0f };
    }

    Vector2 targetWaypoint = m_waypoints[m_currentWaypointIndex];
    float dx = targetWaypoint.x - currentPos.x;
    float dy = targetWaypoint.y - currentPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    bool isFinalWaypoint = (m_currentWaypointIndex == m_waypoints.size() - 1);
    float arrivalRadius = isFinalWaypoint ? m_config.goalArrivalRadius : m_config.waypointArrivalRadius;

    // Check waypoint arrival
    if (dist <= arrivalRadius) {
        if (isFinalWaypoint) {
            m_reachedDestination = true;
            m_hasActivePath = false;
            return Vector2{ 0.0f, 0.0f };
        } else {
            m_currentWaypointIndex++;
            targetWaypoint = m_waypoints[m_currentWaypointIndex];
            dx = targetWaypoint.x - currentPos.x;
            dy = targetWaypoint.y - currentPos.y;
            dist = std::sqrt(dx * dx + dy * dy);
        }
    }

    if (dist < 0.001f) {
        return Vector2{ 0.0f, 0.0f };
    }

    // Desired direction along path
    float invDist = 1.0f / dist;
    Vector2 dir{ dx * invDist, dy * invDist };

    // Blend in avoidance force if enabled
    if (m_config.enableAvoidance && (avoidanceForce.x != 0.0f || avoidanceForce.y != 0.0f)) {
        dir.x += avoidanceForce.x;
        dir.y += avoidanceForce.y;
        float blendedLen = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (blendedLen > 0.001f) {
            dir.x /= blendedLen;
            dir.y /= blendedLen;
        }
    }

    // Determine facing direction
    if (std::abs(dir.x) > std::abs(dir.y)) {
        m_desiredFacing = (dir.x > 0.0f) ? graphics::animation::Direction::Right : graphics::animation::Direction::Left;
    } else {
        m_desiredFacing = (dir.y > 0.0f) ? graphics::animation::Direction::Down : graphics::animation::Direction::Up;
    }

    return Vector2{ dir.x * moveSpeed, dir.y * moveSpeed };
}

} // namespace wildbound::ai

