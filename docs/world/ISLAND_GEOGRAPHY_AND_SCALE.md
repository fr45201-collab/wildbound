# Wildbound — Island Geography & Scale

## 1. Scale Evaluation & Dimension Justification

The world scale for the Isle of Wildbound was systematically selected by analyzing tile resolution, camera viewport dimensions, character movement velocities, spatial memory overhead, and Battle Royale pacing requirements.

### 1.1 Candidate Scale Analysis

| Proposed Dimensions | World Pixels | Land Area (Sq Pixels) | Diagonal Distance | Run Traversal Time | Match Pace Suitability | Navigation Grid Cells (16px) |
|---|---|---|---|---|---|---|
| **320 × 320 tiles** *(Selected)* | $10,240 \times 10,240$ px | $104,857,600 \text{ px}^2$ | $14,481 \text{ px}$ | **85.3s straight / 120.6s diag** | **Optimal for 5-8 min matches** | $640 \times 640 = 409,600$ cells |
| **384 × 384 tiles** | $12,288 \times 12,288$ px | $150,994,944 \text{ px}^2$ | $17,378 \text{ px}$ | $102.4\text{s} \text{ straight / } 144.8\text{s} \text{ diag}$ | Slower mid-game rotations | $768 \times 768 = 589,824$ cells |
| **448 × 448 tiles** | $14,336 \times 14,336$ px | $205,520,896 \text{ px}^2$ | $20,274 \text{ px}$ | $119.5\text{s} \text{ straight / } 168.9\text{s} \text{ diag}$ | Excessive dead space | $896 \times 896 = 802,816$ cells |

### 1.2 Mathematical Justification for 320 × 320 Tiles
1. **Character Traversal Speeds**:
   - Base Player Walk Speed: $80.0\text{ px/sec}$.
   - Base Player Run Speed: $120.0\text{ px/sec}$.
   - Traversal between adjacent settlements (e.g., Greenwood Village to Ancient Fortress = $1,470\text{ px}$):
     $$T_{\text{run}} = \frac{1470}{120} \approx 12.25 \text{ seconds}$$
   - Rotations between distant biomes (e.g., Greenwood Village to Sunfall Outpost = $3,720\text{ px}$):
     $$T_{\text{run}} = \frac{3720}{120} \approx 31.0 \text{ seconds}$$
   This allows tactical positioning ahead of shrinking circle phases without tedious long-distance running.
2. **Memory & Cache Locality**:
   - At a 16px navigation grid resolution, 320x320 tiles produce a $640 \times 640$ cell matrix ($409,600$ cells).
   - Occupying only $\sim 800\text{ KB}$ of RAM, the complete navigation graph fits comfortably within CPU L2 cache, ensuring A* path queries complete in $< 0.1\text{ ms}$.

---

## 2. Island Geography & Natural Boundaries

The island is framed by natural oceanic boundaries:
- **Total Map Extents**: $X \in [0, 10240\text{ px}], Y \in [0, 10240\text{ px}]$.
- **Playable Island Landmass**:
  $$\text{Playable Bounds} = [X: 960\text{ px}, Y: 960\text{ px}, W: 8320\text{ px}, H: 8320\text{ px}]$$
- **Non-Playable Ocean Abyss**: A 30-tile deep ocean water perimeter surrounds the landmass, marked with solid collision barriers preventing entities from wandering off-map.
- **Topographical Elevation**:
  - Southern and Western Lowlands ($Z_0$: Sea Level).
  - Central Plateau ($Z_1$: Gentle Forest Uplands).
  - Northern Iron Crags ($Z_2$: Rugged Mountain Highlands and Alpine Scree).

---

## 3. Hydrology & Water Systems

The island features two interconnected water networks:
1. **The Great Forest River**: Originating from the Iron Crag waterfalls at $(X: 5120, Y: 1280)$, flowing south in an S-curve past Ancient Fortress, through Greenwood woods, and into the southern ocean at $(X: 5500, Y: 9280)$.
   - Width: $128\text{ px} - 256\text{ px}$ ($4 - 8$ tiles).
   - Traversable via the Great River Stone Bridge at $(X: 5500, Y: 5280)$.
2. **The Sunfall Oasis**: A spring-fed desert pool $(X: 7680, Y: 6720)$ sustaining the southern caravan routes.

