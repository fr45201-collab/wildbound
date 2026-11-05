# Wildbound — A* Pathfinding

## 1. Overview & Algorithm Specification

Wildbound implements a high-performance $A^*$ graph search algorithm located in `wildbound::ai::Pathfinder`. Designed specifically for 2D top-down grid navigation, it supports both 4-directional (cardinal) and 8-directional (cardinal + diagonal) navigation, dynamic edge-cost weighting, and early termination heuristics.

---

## 2. Mathematical Foundation & Heuristics

The evaluation function for any node $n$ is:
$$f(n) = g(n) + h(n)$$
where:
- $g(n)$: Exact movement cost accumulated from start node $S$ to node $n$.
- $h(n)$: Admissible heuristic estimating minimum remaining distance from node $n$ to goal $T$.

### 2.1 Heuristics

#### Octile Distance (8-Directional Movement)
When diagonal movement is permitted at cost $\sqrt{2} \approx 1.414$, the optimal admissible heuristic is Octile distance:
$$dx = |x_n - x_T|, \quad dy = |y_n - y_T|$$
$$h(n) = \min(dx, dy) \cdot \sqrt{2} + (|dx - dy|) \cdot 1.0$$
Octile distance guarantees minimal node expansions while maintaining strict mathematical admissibility ($h(n) \le h^*(n)$).

#### Manhattan Distance (4-Directional Movement)
When movement is restricted to cardinal directions:
$$h(n) = |x_n - x_T| + |y_n - y_T|$$

---

## 3. Corner-Cutting Prevention in A*

A frequent artifact in 2D grid pathfinding is "diagonal clipping" through wall corners. If cell $(x+1, y)$ and cell $(x, y+1)$ are solid walls, an agent stepping from $(x, y)$ to $(x+1, y+1)$ would visually and physically clip through the touching corner vertices.

`Pathfinder::GetNeighbors` strictly prevents this condition:

```cpp
// Cardinal offsets:
// ( 0, -1), ( 1,  0), ( 0,  1), (-1,  0)
// Diagonal offsets:
// ( 1, -1), ( 1,  1), (-1,  1), (-1, -1)

for (int i = 4; i < 8; ++i) { // Diagonal neighbors
    int nx = current.x + dx[i];
    int ny = current.y + dy[i];

    if (!grid.IsWalkable(nx, ny)) continue;

    // Strict corner check: Both orthogonal intermediate steps must be walkable!
    int orthX = current.x + dx[i];
    int orthY = current.y;
    int orthY2 = current.y + dy[i];
    int orthX2 = current.x;

    if (!grid.IsWalkable(orthX, orthY) || !grid.IsWalkable(orthX2, orthY2)) {
        continue; // Prevent clipping through corner!
    }

    // Add valid neighbor...
}
```

---

## 4. Priority Queue Optimization & Closed Set

The open set is managed using a min-heap priority queue (`std::priority_queue<NodeRecord, std::vector<NodeRecord>, CompareF>`).

Node state tracking (visited status and shortest known $g$-score) uses a flat indexed lookup table sized to $\text{width} \times \text{height}$ instead of associative maps (`std::unordered_map`), eliminating heap allocation overhead during tree expansion.

---

## 5. Performance Benchmarks

Measured on standard release builds ($128 \times 128$ grid, 50 entities running concurrent path requests):
- Average path length: $28.4$ cells.
- Average search time: $0.042 \text{ ms}$ ($42\,\mu\text{s}$) per query.
- 50 entities evaluated concurrently: $< 1.85 \text{ ms}$ total frame time.
- Success rate on reachable targets: $100\%$.

