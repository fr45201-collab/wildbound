#include "NavigationGrid.hpp"
#include "../../world/World.hpp"
#include "../../core/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <queue>

using wildbound::core::Logger;

namespace wildbound::ai {

NavigationGrid::NavigationGrid() = default;

bool NavigationGrid::Initialize(float worldWidth, float worldHeight, float cellSize) {
    if (worldWidth <= 0.0f || worldHeight <= 0.0f || cellSize <= 0.0f) {
        Logger::Error("NAV", "Invalid NavigationGrid initialization dimensions");
        return false;
    }

    m_worldWidth = worldWidth;
    m_worldHeight = worldHeight;
    m_cellSize = cellSize;

    m_width = static_cast<int>(std::ceil(m_worldWidth / m_cellSize));
    m_height = static_cast<int>(std::ceil(m_worldHeight / m_cellSize));

    m_cells.assign(static_cast<size_t>(m_width) * m_height, 0);
    m_blockedCellCount = 0;

    Logger::Info("NAV", "Initialized NavigationGrid: " + std::to_string(m_width) + "x" +
                 std::to_string(m_height) + " (" + std::to_string(m_cells.size()) + " cells, " +
                 std::to_string(m_cellSize) + "px cell size)");
    return true;
}

void NavigationGrid::Clear() {
    std::fill(m_cells.begin(), m_cells.end(), static_cast<uint8_t>(0));
    m_blockedCellCount = 0;
}

void NavigationGrid::BuildFromWorld(const world::World& world) {
    Rectangle bounds = world.GetWorldBounds();
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
        Logger::Warn("NAV", "World bounds invalid, cannot build NavigationGrid");
        return;
    }

    Initialize(bounds.width, bounds.height, m_cellSize);

    // Rasterize static collision boxes into navigation grid cells
    Rectangle fullQuery{ 0.0f, 0.0f, m_worldWidth, m_worldHeight };
    auto staticBoxes = world.GetCollisionBoxes(fullQuery);

    for (const auto& box : staticBoxes) {
        if (!box.IsSolid()) continue;

        // Compute overlapping cell range
        int minX = std::clamp(static_cast<int>(std::floor(box.bounds.x / m_cellSize)), 0, m_width - 1);
        int maxX = std::clamp(static_cast<int>(std::floor((box.bounds.x + box.bounds.width) / m_cellSize)), 0, m_width - 1);
        int minY = std::clamp(static_cast<int>(std::floor(box.bounds.y / m_cellSize)), 0, m_height - 1);
        int maxY = std::clamp(static_cast<int>(std::floor((box.bounds.y + box.bounds.height) / m_cellSize)), 0, m_height - 1);

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                Rectangle cellRect{
                    x * m_cellSize,
                    y * m_cellSize,
                    m_cellSize,
                    m_cellSize
                };

                if (box.Intersects(cellRect)) {
                    SetCellBlocked(x, y, true);
                }
            }
        }
    }

    Logger::Info("NAV", "NavigationGrid built from World: " + std::to_string(m_blockedCellCount) +
                 " blocked cells out of " + std::to_string(m_cells.size()));
}

void NavigationGrid::SetCellBlocked(int cx, int cy, bool blocked) {
    if (!IsInBounds(cx, cy)) return;
    int idx = CellIndex(cx, cy);
    uint8_t newVal = blocked ? 1 : 0;
    if (m_cells[idx] != newVal) {
        m_cells[idx] = newVal;
        if (blocked) {
            m_blockedCellCount++;
        } else if (m_blockedCellCount > 0) {
            m_blockedCellCount--;
        }
    }
}

void NavigationGrid::SetCellBlocked(GridCoord coord, bool blocked) {
    SetCellBlocked(coord.x, coord.y, blocked);
}

bool NavigationGrid::IsInBounds(int cx, int cy) const noexcept {
    return cx >= 0 && cx < m_width && cy >= 0 && cy < m_height;
}

bool NavigationGrid::IsInBounds(GridCoord coord) const noexcept {
    return IsInBounds(coord.x, coord.y);
}

bool NavigationGrid::IsCellWalkable(int cx, int cy) const noexcept {
    if (!IsInBounds(cx, cy)) return false;
    return m_cells[CellIndex(cx, cy)] == 0;
}

bool NavigationGrid::IsCellWalkable(GridCoord coord) const noexcept {
    return IsCellWalkable(coord.x, coord.y);
}

bool NavigationGrid::IsWalkable(Vector2 worldPos) const noexcept {
    GridCoord c = WorldToGrid(worldPos);
    return IsCellWalkable(c);
}

GridCoord NavigationGrid::WorldToGrid(Vector2 worldPos) const noexcept {
    int cx = static_cast<int>(std::floor(worldPos.x / m_cellSize));
    int cy = static_cast<int>(std::floor(worldPos.y / m_cellSize));
    return GridCoord{ cx, cy };
}

Vector2 NavigationGrid::GridToWorld(int cx, int cy) const noexcept {
    return Vector2{
        (static_cast<float>(cx) + 0.5f) * m_cellSize,
        (static_cast<float>(cy) + 0.5f) * m_cellSize
    };
}

Vector2 NavigationGrid::GridToWorld(GridCoord coord) const noexcept {
    return GridToWorld(coord.x, coord.y);
}

bool NavigationGrid::HasLineOfSight(Vector2 start, Vector2 end) const noexcept {
    return !Raycast(start, end).hit;
}

RaycastResult NavigationGrid::Raycast(Vector2 start, Vector2 end) const noexcept {
    RaycastResult result;
    result.hit = false;

    GridCoord c0 = WorldToGrid(start);
    GridCoord c1 = WorldToGrid(end);

    int x0 = c0.x;
    int y0 = c0.y;
    int x1 = c1.x;
    int y1 = c1.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int curX = x0;
    int curY = y0;

    while (true) {
        if (!IsInBounds(curX, curY) || !IsCellWalkable(curX, curY)) {
            result.hit = true;
            result.hitCell = GridCoord{ curX, curY };
            result.hitPoint = GridToWorld(curX, curY);
            float distDx = result.hitPoint.x - start.x;
            float distDy = result.hitPoint.y - start.y;
            result.distance = std::sqrt(distDx * distDx + distDy * distDy);
            return result;
        }

        if (curX == x1 && curY == y1) break;

        int prevX = curX;
        int prevY = curY;
        bool steppedX = false;
        bool steppedY = false;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            curX += sx;
            steppedX = true;
        }
        if (e2 < dx) {
            err += dx;
            curY += sy;
            steppedY = true;
        }

        // Corner-cutting check: if stepping diagonally, orthogonal neighbours must both be clear
        if (steppedX && steppedY) {
            if (!IsCellWalkable(curX, prevY) || !IsCellWalkable(prevX, curY)) {
                result.hit = true;
                result.hitCell = GridCoord{ curX, curY };
                result.hitPoint = GridToWorld(curX, curY);
                float distDx = result.hitPoint.x - start.x;
                float distDy = result.hitPoint.y - start.y;
                result.distance = std::sqrt(distDx * distDx + distDy * distDy);
                return result;
            }
        }
    }

    return result;
}

Vector2 NavigationGrid::GetNearestWalkablePosition(Vector2 pos, float maxRadius) const noexcept {
    if (IsWalkable(pos)) return pos;

    GridCoord center = WorldToGrid(pos);
    int maxCellRadius = static_cast<int>(std::ceil(maxRadius / m_cellSize));

    float bestDistSq = 99999999.0f;
    Vector2 bestPos = pos;
    bool found = false;

    for (int r = 1; r <= maxCellRadius; ++r) {
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                if (std::abs(dx) != r && std::abs(dy) != r) continue; // Boundary only

                int cx = center.x + dx;
                int cy = center.y + dy;
                if (IsCellWalkable(cx, cy)) {
                    Vector2 candidate = GridToWorld(cx, cy);
                    float dX = candidate.x - pos.x;
                    float dY = candidate.y - pos.y;
                    float distSq = dX * dX + dY * dY;
                    if (distSq < bestDistSq) {
                        bestDistSq = distSq;
                        bestPos = candidate;
                        found = true;
                    }
                }
            }
        }
        if (found) return bestPos;
    }

    return pos;
}

bool NavigationGrid::IsPositionReachable(Vector2 start, Vector2 goal) const noexcept {
    if (!IsWalkable(start) || !IsWalkable(goal)) return false;

    GridCoord startC = WorldToGrid(start);
    GridCoord goalC = WorldToGrid(goal);
    if (startC == goalC) return true;

    // Simple BFS reachability flood check with bounded search depth
    std::queue<GridCoord> q;
    std::vector<bool> visited(static_cast<size_t>(m_width) * m_height, false);

    q.push(startC);
    visited[CellIndex(startC.x, startC.y)] = true;

    const int dx[] = { 0, 1, 0, -1 };
    const int dy[] = { -1, 0, 1, 0 };

    size_t explored = 0;
    const size_t maxExplore = 4000;

    while (!q.empty() && explored < maxExplore) {
        GridCoord cur = q.front();
        q.pop();
        explored++;

        if (cur == goalC) return true;

        for (int i = 0; i < 4; ++i) {
            int nx = cur.x + dx[i];
            int ny = cur.y + dy[i];

            if (IsInBounds(nx, ny) && IsCellWalkable(nx, ny)) {
                int idx = CellIndex(nx, ny);
                if (!visited[idx]) {
                    visited[idx] = true;
                    q.push(GridCoord{ nx, ny });
                }
            }
        }
    }

    return false;
}

bool NavigationGrid::ValidatePath(const std::vector<Vector2>& path) const noexcept {
    if (path.empty()) return false;
    for (size_t i = 0; i < path.size(); ++i) {
        if (!IsWalkable(path[i])) return false;
        if (i > 0) {
            if (!HasLineOfSight(path[i - 1], path[i])) return false;
        }
    }
    return true;
}

Vector2 NavigationGrid::FindNearestReachableTargetPosition(Vector2 targetPos, Vector2 fromPos) const noexcept {
    if (IsWalkable(targetPos) && IsPositionReachable(fromPos, targetPos)) {
        return targetPos;
    }

    GridCoord targetC = WorldToGrid(targetPos);
    float bestDistSq = 99999999.0f;
    Vector2 bestPos = fromPos;
    bool found = false;

    // Check adjacent neighbor cells
    const int dx[] = { 0, 1, 0, -1, 1, 1, -1, -1 };
    const int dy[] = { -1, 0, 1, 0, -1, 1, 1, -1 };

    for (int i = 0; i < 8; ++i) {
        int nx = targetC.x + dx[i];
        int ny = targetC.y + dy[i];
        if (IsCellWalkable(nx, ny)) {
            Vector2 cand = GridToWorld(nx, ny);
            float dX = cand.x - fromPos.x;
            float dY = cand.y - fromPos.y;
            float distSq = dX * dX + dY * dY;
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                bestPos = cand;
                found = true;
            }
        }
    }

    return found ? bestPos : GetNearestWalkablePosition(targetPos, 64.0f);
}

} // namespace wildbound::ai
