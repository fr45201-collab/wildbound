#pragma once

#include "NavigationQuery.hpp"
#include <raylib.h>
#include <vector>
#include <cstdint>

namespace wildbound::world {
    class World;
}

namespace wildbound::ai {

class NavigationGrid {
public:
    NavigationGrid();
    ~NavigationGrid() = default;

    bool Initialize(float worldWidth, float worldHeight, float cellSize = 16.0f);
    void Clear();

    // Population from World collision geometry
    void BuildFromWorld(const world::World& world);

    // Cell manipulation & queries
    void SetCellBlocked(int cx, int cy, bool blocked);
    void SetCellBlocked(GridCoord coord, bool blocked);

    bool IsInBounds(int cx, int cy) const noexcept;
    bool IsInBounds(GridCoord coord) const noexcept;

    bool IsCellWalkable(int cx, int cy) const noexcept;
    bool IsCellWalkable(GridCoord coord) const noexcept;
    bool IsWalkable(Vector2 worldPos) const noexcept;

    // Coordinate conversions
    GridCoord WorldToGrid(Vector2 worldPos) const noexcept;
    Vector2 GridToWorld(int cx, int cy) const noexcept;
    Vector2 GridToWorld(GridCoord coord) const noexcept;

    // Navigation spatial queries
    bool HasLineOfSight(Vector2 start, Vector2 end) const noexcept;
    RaycastResult Raycast(Vector2 start, Vector2 end) const noexcept;

    Vector2 GetNearestWalkablePosition(Vector2 pos, float maxRadius = 96.0f) const noexcept;
    bool IsPositionReachable(Vector2 start, Vector2 goal) const noexcept;
    bool ValidatePath(const std::vector<Vector2>& path) const noexcept;
    Vector2 FindNearestReachableTargetPosition(Vector2 targetPos, Vector2 fromPos) const noexcept;

    // Metrics & Getters
    int GetWidthInCells() const noexcept { return m_width; }
    int GetHeightInCells() const noexcept { return m_height; }
    float GetCellSize() const noexcept { return m_cellSize; }
    float GetWorldWidth() const noexcept { return m_worldWidth; }
    float GetWorldHeight() const noexcept { return m_worldHeight; }
    size_t GetTotalCellCount() const noexcept { return m_cells.size(); }
    size_t GetBlockedCellCount() const noexcept { return m_blockedCellCount; }
    const std::vector<uint8_t>& GetRawCells() const noexcept { return m_cells; }

private:
    int CellIndex(int cx, int cy) const noexcept {
        return cy * m_width + cx;
    }

    float m_cellSize = 16.0f;
    float m_worldWidth = 0.0f;
    float m_worldHeight = 0.0f;
    int m_width = 0;
    int m_height = 0;
    size_t m_blockedCellCount = 0;
    std::vector<uint8_t> m_cells; // 0 = walkable, 1 = blocked
};

} // namespace wildbound::ai

