# Wildbound — Navigation System

## 1. System Overview

The Wildbound Navigation System provides spatial discretization, collision rasterization, and coordinate translation between continuous floating-point 2D world space and integer grid cell space.

Implemented in `wildbound::ai::NavigationGrid`, the navigation system enables fast, memory-efficient spatial queries, raycasting, and reachability tests across any biome or map topology.

---

## 2. Spatial Representation

### 2.1 Grid Resolution & Dimensionality
- **Cell Size**: 16.0 pixels $\times$ 16.0 pixels. Matches the base sprite tile resolution of Wildbound's world maps.
- **Coordinate Conversion**:
  $$G_x = \lfloor \frac{W_x}{16.0} \rfloor, \quad G_y = \lfloor \frac{W_y}{16.0} \rfloor$$
  $$W_x = G_x \times 16.0 + 8.0, \quad W_y = G_y \times 16.0 + 8.0$$
  World coordinates derived from grid cells center on the cell's midpoint $(+8.0\text{px})$.

### 2.2 Data Layout & Memory Topology
Grid cells are stored within a contiguous 1D buffer:
```cpp
struct GridCell {
    uint8_t flags = 0;       // Bit 0: Walkable, Bit 1: Dynamic Obstacle, Bit 2: Water/Hazard
    uint8_t cost_weight = 1; // Movement traversal penalty (1 = normal, >1 = mud/rough terrain)
};
std::vector<GridCell> m_cells;
```
For a typical $128 \times 128$ tile map ($2048 \times 2048$ world pixels):
$$\text{Memory Footprint} = 128 \times 128 \times 2 \text{ bytes} = 32\,768 \text{ bytes (32 KB)}$$
The entire navigation grid easily fits within standard CPU L1/L2 caches, guaranteeing near-instantaneous memory lookups.

---

## 3. World Collision Rasterization

Static world obstacles originate from `wildbound::world::World::GetCollisionBoxes()`, which aggregates tile collision flags and decorative world colliders:

```cpp
void NavigationGrid::RasterizeWorldCollision(const std::vector<Rectangle>& collisionBoxes) {
    // 1. Reset all cells to fully walkable
    std::fill(m_cells.begin(), m_cells.end(), GridCell{ CELL_WALKABLE, 1 });

    // 2. Iterate each collision box and stamp blocked flags
    for (const auto& box : collisionBoxes) {
        int minX = std::clamp(static_cast<int>(box.x / m_cellSize), 0, m_width - 1);
        int minY = std::clamp(static_cast<int>(box.y / m_cellSize), 0, m_height - 1);
        int maxX = std::clamp(static_cast<int>((box.x + box.width) / m_cellSize), 0, m_width - 1);
        int maxY = std::clamp(static_cast<int>((box.y + box.height) / m_cellSize), 0, m_height - 1);

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                SetWalkable(x, y, false);
            }
        }
    }
}
```

---

## 4. Line-of-Sight Raycasting & Corner-Cutting Prevention

Line-of-sight raycasting uses a discrete 2D Bresenham traversal algorithm. A critical failure mode in 2D grid raycasting occurs when a ray travels diagonally between two solid wall corners touching only diagonally (corner-cutting).

The Wildbound raycaster enforces strict corner-cutting validation:
```cpp
bool NavigationGrid::Raycast(GridCoord start, GridCoord end, RaycastResult& outResult) const {
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int curX = x0;
    int curY = y0;

    while (true) {
        if (!IsWalkable(curX, curY)) {
            outResult.hit = true;
            outResult.hitCoord = { curX, curY };
            return false;
        }

        if (curX == x1 && curY == y1) break;

        int e2 = 2 * err;
        int prevX = curX;
        int prevY = curY;

        if (e2 > -dy) {
            err -= dy;
            curX += sx;
        }
        if (e2 < dx) {
            err += dx;
            curY += sy;
        }

        // Strict diagonal corner-cutting prevention:
        // If stepping diagonally, both orthogonal neighbors must be walkable!
        if (curX != prevX && curY != prevY) {
            if (!IsWalkable(curX, prevY) || !IsWalkable(prevX, curY)) {
                outResult.hit = true;
                outResult.hitCoord = { curX, curY };
                return false;
            }
        }
    }

    outResult.hit = false;
    return true; // Unobstructed line of sight
}
```

---

## 5. Nearest Walkable Search & Reachability Checks

When a dynamic entity or projectile targets an unwalkable coordinate (such as inside a wall or tree), `FindNearestWalkableCell` performs an expanding outward spiral search:

$$r \in [1, R_{\max}], \quad \theta \in [0, 2\pi]$$

The algorithm returns the closest valid cell by Euclidean distance, guaranteeing that AI path requests always start and terminate on legitimate, navigable terrain.

