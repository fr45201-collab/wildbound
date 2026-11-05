# Collision Update Order & Determinism Specification

## 1. Overview

Deterministic simulation is essential in Wildbound to guarantee that combat resolutions, projectile impacts, loot collection, and environmental trigger interactions execute identically regardless of iteration order, memory layout, or target platform.

The `CollisionManager` enforces a strict 13-stage collision update pipeline executed once per fixed simulation tick (`Game::Update(float dt)`).

---

## 2. The 13-Stage Simulation Update Pipeline

```
  [Stage 1: Pre-Tick Deferred Mutation Flush]
                   │
                   ▼
  [Stage 2: Transform & Bounds Synchronization]
                   │
                   ▼
  [Stage 3: Active & Enabled Filtering]
                   │
                   ▼
  [Stage 4: Canonical Pair Generation & Deduplication]
                   │
                   ▼
  [Stage 5: Bilateral Layer & Mask Filtering]
                   │
                   ▼
  [Stage 6: Zero-Allocation Narrow-Phase Overlap Testing]
                   │
                   ▼
  [Stage 7: Lifecycle Event Classification (Enter/Stay/Exit)]
                   │
                   ▼
  [Stage 8: Stateful Trigger Volume Transition Tracking]
                   │
                   ▼
  [Stage 9: Deterministic Event Sequence Sorting]
                   │
                   ▼
  [Stage 10: Event Dispatch & Callback Invocation]
                   │
                   ▼
  [Stage 11: Overlap State Transition (`previous = current`)]
                   │
                   ▼
  [Stage 12: Post-Tick Deferred Mutation Flush]
                   │
                   ▼
  [Stage 13: Simulation Performance Metrics Recording]
```

### Stage 1: Pre-Tick Deferred Mutation Flush
Any collider additions, removals, layer updates, or mask changes queued outside the tick or during frame setup are processed before physics calculations commence.

### Stage 2: Transform & Bounds Synchronization
Dynamic colliders (e.g., `Player`, `Enemy`, `Projectile`) synchronize their world-space positions. `Collider::SyncTransform(position)` recomputes `m_bounds`:
$$\text{worldBounds.x} = \text{position.x} + \text{offset.x}$$
$$\text{worldBounds.y} = \text{position.y} + \text{offset.y}$$

### Stage 3: Active & Enabled Filtering
Disabled colliders (`collider.IsEnabled() == false`) are bypassed immediately. Dynamic colliders are separated from static map colliders to optimize pair iteration.

### Stage 4: Canonical Pair Generation & Deduplication
To ensure order-independence, all interaction pairs are represented canonically:
$$\text{Pair} = (\min(id_A, id_B), \max(id_A, id_B))$$
Pairs are collected into a pre-allocated vector and deduplicated. Pairs where $id_A = id_B$ and `allowSelfCollision == false` are immediately discarded.

### Stage 5: Bilateral Layer & Mask Filtering
Before testing geometric intersections, the bilateral consent filter is evaluated:
```cpp
bool ShouldTestPair(const Collider& a, const Collider& b) {
    if (a.GetId() == b.GetId() && !a.AllowsSelfCollision()) return false;
    return (a.GetMask() & b.GetLayer()) != 0 &&
           (b.GetMask() & a.GetLayer()) != 0;
}
```
If either entity withholds consent, the pair is skipped with zero arithmetic overhead.

### Stage 6: Zero-Allocation Narrow-Phase Overlap Testing
`OverlapDetector::CheckOverlap(a.GetBounds(), b.GetBounds())` performs an optimized, branchless AABB separation test:
```cpp
bool CheckOverlap(const Rectangle& a, const Rectangle& b) {
    return (a.x < b.x + b.width) &&
           (a.x + a.width > b.x) &&
           (a.y < b.y + b.height) &&
           (a.y + a.height > b.y);
}
```
If an overlap occurs, the pair is recorded in `m_currentOverlaps`.

### Stage 7: Lifecycle Event Classification
`m_currentOverlaps` is compared against `m_previousOverlaps`:
* **Enter**: Present in `m_currentOverlaps` but absent in `m_previousOverlaps`.
* **Stay**: Present in both `m_currentOverlaps` and `m_previousOverlaps`.
* **Exit**: Present in `m_previousOverlaps` but absent in `m_currentOverlaps`.

### Stage 8: Stateful Trigger Volume Transition Tracking
For all pairs involving a `ColliderType::Trigger`, the associated `TriggerVolume` updates its internal tracking set:
* `OnEntityEnter(otherEntityId)`
* `OnEntityStay(otherEntityId)`
* `OnEntityExit(otherEntityId)`

### Stage 9: Deterministic Event Sequence Sorting
Before any callbacks or listeners are invoked, all generated events in the frame are sorted using a strict lexical comparator:
```cpp
std::sort(events.begin(), events.end(), [](const CollisionEvent& a, const CollisionEvent& b) {
    if (a.type != b.type) return a.type < b.type;
    if (a.colliderA != b.colliderA) return a.colliderA < b.colliderA;
    return a.colliderB < b.colliderB;
});
```
This guarantees that event delivery order is identical across platforms and test executions.

### Stage 10: Event Dispatch & Callback Invocation
The manager sets `m_isIterating = true` and dispatches events to:
1. Registered global listeners implementing `ICollisionListener`.
2. Collider-specific individual callbacks `m_callbacks[colliderId]`.

Any calls to `AddCollider`, `RemoveCollider`, `SetLayer`, or `SetMask` triggered from within these callbacks are queued into `m_deferredMutations`. Iterator invalidation and dangling pointer bugs are mathematically prevented.

### Stage 11: Overlap State Transition
The previous overlap set is updated:
```cpp
m_previousOverlaps = std::move(m_currentOverlaps);
```

### Stage 12: Post-Tick Deferred Mutation Flush
The manager sets `m_isIterating = false` and executes all mutations queued during Stage 10 callbacks in FIFO order.

### Stage 13: Simulation Performance Metrics Recording
Simulation metrics are captured for developer tooling:
* Total registered colliders (static vs dynamic).
* Potential vs actual narrow-phase tests performed.
* Active overlap count.
* Events dispatched per event type (Enter, Stay, Exit).
* Total tick execution time in microseconds.

---

## 3. Guarantees & Invariants

1. **Re-entrancy Safety**: The collision system cannot enter an undefined state if a projectile deletes itself or spawns a particle on collision.
2. **Order Independence**: Changing entity allocation order in game pools does not alter the collision event dispatch sequence.
3. **No Double-Counting**: Bilateral filtering and canonical pair normalization guarantee every pair is tested at most once per tick.
4. **Zero-Allocation Steady State**: Memory buffers for active pairs, current overlaps, and event queues are reused between frames, preventing heap fragmentation.

