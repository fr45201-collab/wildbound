#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::ai {

struct GridCoord {
    int x = 0;
    int y = 0;

    bool operator==(const GridCoord& other) const noexcept {
        return x == other.x && y == other.y;
    }

    bool operator!=(const GridCoord& other) const noexcept {
        return !(*this == other);
    }
};

struct PathNode {
    GridCoord coord;
    Vector2 worldPos{ 0.0f, 0.0f };
};

enum class PathStatus {
    Found,
    Partial,
    StartBlocked,
    GoalBlocked,
    Unreachable,
    Error
};

struct PathResult {
    PathStatus status = PathStatus::Error;
    std::vector<Vector2> waypoints;
    float totalDistance = 0.0f;
    size_t nodesExplored = 0;

    bool IsValid() const noexcept {
        return status == PathStatus::Found && !waypoints.empty();
    }
};

struct RaycastResult {
    bool hit = false;
    Vector2 hitPoint{ 0.0f, 0.0f };
    GridCoord hitCell{ 0, 0 };
    float distance = 0.0f;
};

enum class MovementMode {
    FourDirectional,
    EightDirectional
};

} // namespace wildbound::ai

