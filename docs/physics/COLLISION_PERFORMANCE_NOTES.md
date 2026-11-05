# Collision Performance & Optimization Notes

## 1. Overview & Frame Budget

Wildbound targets a stable 60 FPS update rate (16.66 ms total frame budget). The physics and collision subsystem is budgeted for:
* **Target Collision Budget**: $\le 1.50\text{ ms}$ per tick in production builds.
* **Measured Release Performance**: $\approx 0.04 - 0.08\text{ ms}$ for standard game scenes ($\approx 120\text{ colliders}$).
* **CPU Cache Efficiency**: High data locality with contiguous collider storage and branchless intersection logic.

---

## 2. Complexity & Pair Scaling Analysis

### 2.1 Partitioned Dynamic vs Static Testing
In a naive collision engine with $N$ objects, checking all pairs requires:
$$\text{Pairs}_{\text{naive}} = \frac{N(N - 1)}{2} = \mathcal{O}(N^2)$$

In Wildbound, colliders are partitioned into **Static** (walls, buildings, environmental obstacles) and **Dynamic** (player, enemies, projectiles, wildlife, pickups).
* Static objects never move and do not collide with other static objects.
* Thus, static-to-static pair checks are omitted entirely:
$$\text{Pairs}_{\text{partitioned}} = N_{\text{dyn}} \cdot N_{\text{stat}} + \frac{N_{\text{dyn}}(N_{\text{dyn}} - 1)}{2}$$

For a typical scene with $N_{\text{stat}} = 300$ and $N_{\text{dyn}} = 30$:
* Naive check: $\frac{330 \times 329}{2} = 54,285$ checks.
* Partitioned check: $(30 \times 300) + \frac{30 \times 29}{2} = 9,000 + 435 = 9,435$ checks ($82.6\%$ reduction).

---

## 3. Micro-Optimizations & Memory Layout

### 3.1 Bilateral Consent Early-Out
The bilateral filter executes before any floating-point arithmetic or AABB coordinate lookups:
```cpp
if ((a.mask & b.layer) == 0 || (b.mask & a.layer) == 0) continue;
```
Because bitwise `AND` instructions execute in a single CPU clock cycle, uncollidable pairs (such as `Pickup` vs `World` or `Wildlife` vs `Hazard`) are rejected almost instantaneously.

### 3.2 Branchless AABB Overlap Detection
The bounding box test uses a standard 4-way comparison that compiles into fast SIMD or conditional moves without branch mispredictions:
```cpp
inline bool CheckOverlap(const Rectangle& a, const Rectangle& b) noexcept {
    return (a.x < b.x + b.width) &&
           (a.x + a.width > b.x) &&
           (a.y < b.y + b.height) &&
           (a.y + a.height > b.y);
}
```

### 3.3 Zero-Allocation Steady State
To eliminate heap fragmentation and memory allocator lock contention:
* `m_currentOverlaps`, `m_deferredMutations`, and `m_activePairs` retain their allocated storage across ticks using `.clear()`.
* Internal vectors are pre-reserved with capacity for 256–1024 elements upon engine initialization.
* Event arrays are recycled across simulation ticks.

### 3.4 Data Cache Locality
* The core `Collider` structure is aligned and fits in 64 bytes (exact cache line size):
  * `m_bounds` (`Rectangle` = 16 bytes)
  * `m_offset` (`Vector2` = 8 bytes)
  * `m_id` (`uint64_t` = 8 bytes)
  * `m_entityId` (`uint32_t` = 4 bytes)
  * `m_layer` (`uint32_t` = 4 bytes)
  * `m_mask` (`uint32_t` = 4 bytes)
  * `m_type`, `m_enabled`, `m_isStatic`, `m_allowSelfCollision` (4 bytes flags)
  * Padding / alignment: 16 bytes.
* 1,000 active colliders consume only $\approx 64\text{ KB}$ of RAM, fitting completely within modern CPU L1/L2 caches.

---

## 4. Benchmark Measurements

The following metrics were captured using MSVC 19.44 on an x86_64 system running Windows 11 with 120 dynamic colliders executing 100 simulation ticks:

| Metric | Debug Build (`/Od`, Iterator Debugging) | Release Build (`/O2`, Whole Program Opt) |
| :--- | :--- | :--- |
| **100 Ticks Execution Time** | $\approx 271\text{ ms}$ | $\approx 4.8\text{ ms}$ |
| **Per-Tick Time** | $\approx 2.71\text{ ms}$ | $\approx 0.048\text{ ms}$ |
| **Frame Budget Consumed (60 FPS)** | $16.3\%$ | **$0.29\%$** |
| **Max Heap Allocations / Tick** | $0$ (steady state) | $0$ (steady state) |

### Scalability Ceiling
At $0.048\text{ ms}$ for 120 colliders, the current partitioned narrow phase can comfortably scale to $500+$ simultaneous dynamic entities without exceeding $1.0\text{ ms}$ per tick. A spatial grid or BVH structure can be seamlessly dropped into the broad phase if entity density exceeds 1,000 active combatants in future Battle Royale expansion phases.

