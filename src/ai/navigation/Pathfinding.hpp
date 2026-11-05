#pragma once

#include "NavigationGrid.hpp"
#include "NavigationQuery.hpp"
#include <vector>
#include <memory>

namespace wildbound::ai {

struct PathfindingConfig {
    MovementMode mode = MovementMode::EightDirectional;
    bool preventCornerCutting = true;
    bool enableSmoothing = true;
    size_t maxSearchIterations = 4000;
    float diagonalCost = 1.41421356f;
    float orthogonalCost = 1.0f;
};

class Pathfinding {
public:
    explicit Pathfinding(const NavigationGrid& grid, PathfindingConfig config = {});
    ~Pathfinding() = default;

    PathResult FindPath(Vector2 start, Vector2 goal);
    PathResult FindPath(GridCoord start, GridCoord goal);

    void SetConfig(const PathfindingConfig& config) { m_config = config; }
    const PathfindingConfig& GetConfig() const noexcept { return m_config; }

    // Path smoothing utility
    static std::vector<Vector2> SmoothPath(const std::vector<Vector2>& rawWaypoints, const NavigationGrid& grid);

private:
    float Heuristic(GridCoord a, GridCoord b) const noexcept;
    bool CanMoveDiagonal(GridCoord from, int dx, int dy) const noexcept;

    const NavigationGrid& m_grid;
    PathfindingConfig m_config;
};

} // namespace wildbound::ai

