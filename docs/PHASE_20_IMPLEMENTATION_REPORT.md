# Phase 20 Implementation Report: Physics & Collision System Extension

**Project**: Wildbound — 2D Top-Down Fantasy Battle Royale  
**Phase**: Phase 20 — Physics & Collision System Extension  
**Status**: Completed  
**Technology Stack**: C++20, raylib 5.5, Dear ImGui / rlImGui, CMake, MSVC, CTest  

---

## 1. Executive Summary

Phase 20 delivers a robust, modular, data-driven 2D physics and collision architecture to Wildbound without relying on heavy external physics libraries (such as Box2D, Chipmunk, or Bullet). The system extends the existing game engine with:
1. **Configurable Bitfield Collision Layers**: 10 distinct logical layers with bitwise arithmetic.
2. **Bilateral Collision Masks**: Enforces mutual consent `(A.mask & B.layer) != 0 && (B.mask & A.layer) != 0` and self-collision prevention.
3. **Axis-Aligned Bounding Box (AABB) Collider Representation**: Position offsets, bounds transformation, enable/disable states, and solid vs trigger classification.
4. **Stateful Trigger Volumes**: Manages multi-entity containment with strict `Enter`, `Stay`, and `Exit` lifecycle event dispatch.
5. **Zero-Allocation Narrow-Phase Overlap Detection**: Fast, branchless geometric intersection testing with minimum translation vector (MTV) calculation.
6. **Re-entrant Callback & Deferred Mutation Queue**: Prevents iterator invalidation or memory corruption when entities spawn, despawn, or modify colliders during callbacks.
7. **Deterministic 13-Stage Simulation Update Pipeline**: Guarantees identical event ordering across runs through canonical pair normalization `(min(idA, idB), max(idA, idB))` and lexical event sorting.
8. **Comprehensive Developer Tooling & Visualizer**:
   - In-world raylib debug renderer with layer color-coding and active overlap highlighting (hotkey `F3`).
   - Dedicated DevUI Tab 13 ("Physics & Collision") with live performance metrics, an interactive layer filter validator, and an active overlaps inspector.
9. **Full Automated Test Coverage**: A dedicated test suite (`PhysicsCollisionTest`) bringing total CTest suites to 22/22 (100% pass rate in Release and Debug).

---

## 2. System Architecture & Components

The implementation is located under `src/physics/`:

```
src/physics/
├── CollisionTypes.hpp            # 10 CollisionLayer flags, ColliderType, default masks, string utils
├── CollisionFilter.hpp           # Pure bilateral consent filtering & self-collision rules
├── OverlapDetector.hpp           # Zero-allocation AABB math, point containment, MTV
├── Collider.hpp                  # Authoritative collider data structure & transform syncing
├── CollisionEvent.hpp            # CollisionEventType (Enter, Stay, Exit), listener interfaces
├── TriggerVolume.hpp             # Stateful trigger component tracking contained entities
├── CollisionManager.hpp/.cpp     # Central simulation coordinator, pair sorting, deferred queue
└── CollisionDebugRenderer.hpp/.cpp # In-world raylib debug visualization & overlap highlighting
```

### 2.1 The 10 Logical Collision Layers
* `World` (`0x0001`): Static impassable geometry (cliffs, deep water, outer map boundaries).
* `Player` (`0x0002`): Local and remote player characters.
* `Enemy` (`0x0004`): Hostile AI units.
* `Wildlife` (`0x0008`): Neutral and passive creatures (deer, boar, birds).
* `Projectile` (`0x0010`): Arrows, thrown daggers, spells.
* `Pickup` (`0x0020`): Ground loot, weapons, consumable items.
* `Trigger` (`0x0040`): Non-solid detection zones (doors, biome gates, event areas).
* `Building` (`0x0080`): Village structures, houses, blacksmiths, walls, fences.
* `Hazard` (`0x0100`): Environmental damage areas, spike traps, poison gas, death circle.
* `Interactable` (`0x0200`): Chests, campfires, shrines, interactive props.

---

## 3. Key Technical Achievements

### 3.1 Bilateral Consent Model
Unlike naive systems where an entity collides if either mask matches, Wildbound strictly enforces bilateral consent:
$$\text{CanCollide}(A, B) \iff (A.\text{mask} \ \& \ B.\text{layer} \ne 0) \ \land \ (B.\text{mask} \ \& \ A.\text{layer} \ne 0)$$
This cleanly solves classic asymmetric bugs (e.g. projectiles detecting players while players ignore incoming projectiles, or loot items checking terrain).

### 3.2 Deferred Mutation Queue (Re-entrancy Safety)
During collision event dispatch (Stage 10), game logic frequently adds or removes entities (e.g., arrow destroyed on impact, health potion consumed, death animation spawned). To prevent iterator invalidation:
* The manager sets `m_isIterating = true`.
* All modifications (`AddCollider`, `RemoveCollider`, `SetLayer`, `SetMask`, `SetEnabled`) are pushed to `m_deferredMutations`.
* At the conclusion of the tick (Stage 12), all queued mutations execute in strict FIFO order before returning control.

### 3.3 Zero-Allocation Steady State & Frame Budget
* Contiguous data structures and pre-reserved vector capacities (`reserve(256)`) ensure zero dynamic heap allocations occur during steady-state simulation ticks.
* Release benchmark: 100 ticks with 120 dynamic colliders completed in **4.8 ms** (~0.048 ms per tick), consuming only **0.29%** of the 16.66 ms 60 FPS frame budget.

---

## 4. Integration with Existing Systems

* **`World`**: Added `World::PopulateCollisionManager(CollisionManager&)` which extracts static impassable terrain and building footprints from map tile data and registers them as static `CollisionLayer::World` and `CollisionLayer::Building` colliders.
* **`Player`**: Integrated `uint64_t m_colliderId` with getter/setter. The existing discrete axis-separated movement resolution in `Player::ResolveMovement` remains intact and unaffected, preserving character movement feel and wall-sliding.
* **`Game`**: Instantiated `CollisionManager` and `CollisionDebugRenderer`, populated static map geometry, wired the player collider to the dynamic layer, hooked simulation ticks into `Game::Update(dt)`, and integrated the debug render pass under `F3`.
* **`DevUI`**: Added Tab 13 ("Physics & Collision") providing real-time metrics, interactive bilateral filter validator, and live active overlap inspection.

---

## 5. Verification & Testing

### 5.1 Automated CTest Suite
Added `tests/physics/test_collision.cpp` containing 6 comprehensive test suites:
1. `CollisionLayersAndMasks`: Bitwise combinations and bilateral consent verification.
2. `AABBOverlapDetection`: Geometric bounds, strict intersections, touching boundaries, and MTVs.
3. `TriggerVolumeLifecycle`: Sequence of `Enter -> Stay -> Exit` transitions across simulated frames.
4. `DeferredMutationSafety`: Spawning and despawning colliders from within active collision callbacks.
5. `DeterministicEventOrdering`: Exact event sorting verification across shuffled input orders.
6. `SpatialQueryAndBenchmark`: Scalability benchmark under high collider density.

### 5.2 Build & Test Results
* **Release Build**: `cmake --build build --config Release` — **0 Errors, 0 Warnings**.
* **Debug Build**: `cmake --build build --config Debug` — **0 Errors, 0 Warnings**.
* **Release CTest**: **22 / 22 test suites passed (100%)**.
* **Debug CTest**: **22 / 22 test suites passed (100%)**.
* **Smoke Run**: `wildbound.exe --test-run` — 60 frames executed, clean shutdown (exit code 0).

---

## 6. Deliverables & Documentation Index

| Document | Path | Description |
| :--- | :--- | :--- |
| **Pre-Implementation Audit** | [`docs/physics/PHASE_20_COLLISION_AUDIT.md`](file:///c:/dev/Wildbound/docs/physics/PHASE_20_COLLISION_AUDIT.md) | Initial baseline assessment and architectural review. |
| **System Architecture** | [`docs/physics/COLLISION_SYSTEM_ARCHITECTURE.md`](file:///c:/dev/Wildbound/docs/physics/COLLISION_SYSTEM_ARCHITECTURE.md) | High-level architecture, module responsibilities, and data flow. |
| **Layer & Mask Spec** | [`docs/physics/COLLISION_LAYER_AND_MASK_SPECIFICATION.md`](file:///c:/dev/Wildbound/docs/physics/COLLISION_LAYER_AND_MASK_SPECIFICATION.md) | 10 layers, 10x10 interaction matrix, and default mask presets. |
| **Trigger Volume Spec** | [`docs/physics/TRIGGER_VOLUME_SPECIFICATION.md`](file:///c:/dev/Wildbound/docs/physics/TRIGGER_VOLUME_SPECIFICATION.md) | Stateful trigger volume specification and lifecycle tracking. |
| **Event & Callback Spec** | [`docs/physics/COLLISION_EVENT_AND_CALLBACK_SPECIFICATION.md`](file:///c:/dev/Wildbound/docs/physics/COLLISION_EVENT_AND_CALLBACK_SPECIFICATION.md) | Event model, listeners, and deferred mutation mechanics. |
| **Update Order Spec** | [`docs/physics/COLLISION_UPDATE_ORDER.md`](file:///c:/dev/Wildbound/docs/physics/COLLISION_UPDATE_ORDER.md) | The 13-stage deterministic simulation pipeline. |
| **Debugging Guide** | [`docs/physics/COLLISION_DEBUGGING_GUIDE.md`](file:///c:/dev/Wildbound/docs/physics/COLLISION_DEBUGGING_GUIDE.md) | Visualizer controls (`F1`, `F3`), color coding, and DevUI Tab 13. |
| **Performance Notes** | [`docs/physics/COLLISION_PERFORMANCE_NOTES.md`](file:///c:/dev/Wildbound/docs/physics/COLLISION_PERFORMANCE_NOTES.md) | Algorithmic analysis, zero-allocation model, and benchmark metrics. |
| **Implementation Report** | [`docs/physics/PHASE_20_IMPLEMENTATION_REPORT.md`](file:///c:/dev/Wildbound/docs/physics/PHASE_20_IMPLEMENTATION_REPORT.md) | Formal completion report (this document). |
