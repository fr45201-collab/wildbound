#include "Pathfinding.hpp"
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace wildbound::ai {

namespace {

struct AStarNode {
    GridCoord coord;
    float gCost = 0.0f;
    float fCost = 0.0f;

    bool operator>(const AStarNode& other) const noexcept {
        return fCost > other.fCost;
    }
};

inline int CoordToKey(GridCoord c, int width) {
    return c.y * width + c.x;
}

inline GridCoord KeyToCoord(int key, int width) {
    return GridCoord{ key % width, key / width };
}

} // namespace

Pathfinding::Pathfinding(const NavigationGrid& grid, PathfindingConfig config)
    : m_grid(grid)
    , m_config(config)
{
}

float Pathfinding::Heuristic(GridCoord a, GridCoord b) const noexcept {
    int dx = std::abs(a.x - b.x);
    int dy = std::abs(a.y - b.y);

    if (m_config.mode == MovementMode::FourDirectional) {
        // Manhattan distance
        return static_cast<float>(dx + dy) * m_config.orthogonalCost;
    } else {
        // Octile distance
        int minD = std::min(dx, dy);
        int maxD = std::max(dx, dy);
        return static_cast<float>(minD) * m_config.diagonalCost +
               static_cast<float>(maxD - minD) * m_config.orthogonalCost;
    }
}

bool Pathfinding::CanMoveDiagonal(GridCoord from, int dx, int dy) const noexcept {
    if (!m_config.preventCornerCutting) return true;

    // Both adjacent orthogonal neighbours must be walkable
    // Moving from (x, y) to (x + dx, y + dy) requires:
    // (x + dx, y) is walkable AND (x, y + dy) is walkable
    return m_grid.IsCellWalkable(from.x + dx, from.y) &&
           m_grid.IsCellWalkable(from.x, from.y + dy);
}

PathResult Pathfinding::FindPath(Vector2 start, Vector2 goal) {
    GridCoord startC = m_grid.WorldToGrid(start);
    GridCoord goalC = m_grid.WorldToGrid(goal);

    // If start is blocked, attempt to relocate to nearest walkable cell
    if (!m_grid.IsCellWalkable(startC)) {
        Vector2 resolvedStart = m_grid.GetNearestWalkablePosition(start, 48.0f);
        startC = m_grid.WorldToGrid(resolvedStart);
        if (!m_grid.IsCellWalkable(startC)) {
            PathResult res;
            res.status = PathStatus::StartBlocked;
            return res;
        }
    }

    // If goal is blocked, attempt to find nearest reachable target position
    if (!m_grid.IsCellWalkable(goalC)) {
        Vector2 resolvedGoal = m_grid.FindNearestReachableTargetPosition(goal, start);
        goalC = m_grid.WorldToGrid(resolvedGoal);
        if (!m_grid.IsCellWalkable(goalC)) {
            PathResult res;
            res.status = PathStatus::GoalBlocked;
            return res;
        }
    }

    PathResult result = FindPath(startC, goalC);

    if (result.IsValid()) {
        // Ensure the path begins at original start world position and ends at goal if goal is walkable
        if (!result.waypoints.empty()) {
            result.waypoints.front() = start;
        }
        if (m_grid.IsWalkable(goal)) {
            result.waypoints.back() = goal;
        }

        if (m_config.enableSmoothing && result.waypoints.size() > 2) {
            result.waypoints = SmoothPath(result.waypoints, m_grid);
        }

        // Recalculate accurate total path distance
        result.totalDistance = 0.0f;
        for (size_t i = 1; i < result.waypoints.size(); ++i) {
            float dx = result.waypoints[i].x - result.waypoints[i - 1].x;
            float dy = result.waypoints[i].y - result.waypoints[i - 1].y;
            result.totalDistance += std::sqrt(dx * dx + dy * dy);
        }
    }

    return result;
}

PathResult Pathfinding::FindPath(GridCoord start, GridCoord goal) {
    PathResult result;

    if (!m_grid.IsInBounds(start) || !m_grid.IsCellWalkable(start)) {
        result.status = PathStatus::StartBlocked;
        return result;
    }

    if (!m_grid.IsInBounds(goal) || !m_grid.IsCellWalkable(goal)) {
        result.status = PathStatus::GoalBlocked;
        return result;
    }

    if (start == goal) {
        result.status = PathStatus::Found;
        result.waypoints.push_back(m_grid.GridToWorld(start));
        result.totalDistance = 0.0f;
        result.nodesExplored = 1;
        return result;
    }

    int gridW = m_grid.GetWidthInCells();

    // Priority queue of open nodes
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;
    std::unordered_map<int, float> gScore;
    std::unordered_map<int, GridCoord> cameFrom;

    int startKey = CoordToKey(start, gridW);

    gScore[startKey] = 0.0f;
    openSet.push(AStarNode{ start, 0.0f, Heuristic(start, goal) });

    // Direction offsets
    static const int dx4[] = { 0, 1, 0, -1 };
    static const int dy4[] = { -1, 0, 1, 0 };
    static const float cost4[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    static const int dx8[] = { 0, 1, 0, -1, 1, 1, -1, -1 };
    static const int dy8[] = { -1, 0, 1, 0, -1, 1, 1, -1 };

    int dirCount = (m_config.mode == MovementMode::FourDirectional) ? 4 : 8;

    size_t explored = 0;
    bool found = false;

    while (!openSet.empty() && explored < m_config.maxSearchIterations) {
        AStarNode current = openSet.top();
        openSet.pop();
        explored++;

        if (current.coord == goal) {
            found = true;
            break;
        }

        int currentKey = CoordToKey(current.coord, gridW);
        if (current.gCost > gScore[currentKey]) {
            continue; // Already found a cheaper path to this node
        }

        for (int i = 0; i < dirCount; ++i) {
            int nx = current.coord.x + (dirCount == 4 ? dx4[i] : dx8[i]);
            int ny = current.coord.y + (dirCount == 4 ? dy4[i] : dy8[i]);
            GridCoord neighbor{ nx, ny };

            if (!m_grid.IsInBounds(neighbor) || !m_grid.IsCellWalkable(neighbor)) {
                continue;
            }

            // Diagonal corner-cutting check
            int stepX = nx - current.coord.x;
            int stepY = ny - current.coord.y;
            bool isDiagonal = (stepX != 0 && stepY != 0);

            if (isDiagonal && !CanMoveDiagonal(current.coord, stepX, stepY)) {
                continue;
            }

            float stepCost = isDiagonal ? m_config.diagonalCost : m_config.orthogonalCost;
            float tentativeG = current.gCost + stepCost;

            int neighborKey = CoordToKey(neighbor, gridW);
            auto it = gScore.find(neighborKey);

            if (it == gScore.end() || tentativeG < it->second) {
                gScore[neighborKey] = tentativeG;
                cameFrom[neighborKey] = current.coord;
                float f = tentativeG + Heuristic(neighbor, goal);
                openSet.push(AStarNode{ neighbor, tentativeG, f });
            }
        }
    }

    result.nodesExplored = explored;

    if (!found) {
        result.status = PathStatus::Unreachable;
        return result;
    }

    // Reconstruct path from goal back to start
    std::vector<Vector2> path;
    GridCoord curr = goal;

    while (curr != start) {
        path.push_back(m_grid.GridToWorld(curr));
        int k = CoordToKey(curr, gridW);
        auto it = cameFrom.find(k);
        if (it == cameFrom.end()) break;
        curr = it->second;
    }
    path.push_back(m_grid.GridToWorld(start));

    std::reverse(path.begin(), path.end());

    result.status = PathStatus::Found;
    result.waypoints = std::move(path);

    // Compute initial total distance
    result.totalDistance = 0.0f;
    for (size_t i = 1; i < result.waypoints.size(); ++i) {
        float dx = result.waypoints[i].x - result.waypoints[i - 1].x;
        float dy = result.waypoints[i].y - result.waypoints[i - 1].y;
        result.totalDistance += std::sqrt(dx * dx + dy * dy);
    }

    return result;
}

std::vector<Vector2> Pathfinding::SmoothPath(const std::vector<Vector2>& rawWaypoints, const NavigationGrid& grid) {
    if (rawWaypoints.size() <= 2) return rawWaypoints;

    std::vector<Vector2> smoothed;
    smoothed.push_back(rawWaypoints.front());

    size_t currentIdx = 0;

    while (currentIdx < rawWaypoints.size() - 1) {
        size_t furthestIdx = currentIdx + 1;

        // Look as far ahead as possible with direct line of sight
        for (size_t testIdx = rawWaypoints.size() - 1; testIdx > currentIdx + 1; --testIdx) {
            if (grid.HasLineOfSight(rawWaypoints[currentIdx], rawWaypoints[testIdx])) {
                furthestIdx = testIdx;
                break;
            }
        }

        smoothed.push_back(rawWaypoints[furthestIdx]);
        currentIdx = furthestIdx;
    }

    return smoothed;
}

} // namespace wildbound::ai
