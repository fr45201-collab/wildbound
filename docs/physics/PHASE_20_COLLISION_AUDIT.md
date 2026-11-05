# Wildbound — Phase 20: Collision & Physics System Audit

> **Document ID**: `WB-AUD-PHASE20-001`  
> **Status**: APPROVED & COMPLETE  
> **Date**: 2026-11-04  
> **Scope**: Pre-implementation audit of Wildbound's physics, spatial queries, collision structures, entity lifecycle, and combat hitboxes.

---

## 1. Executive Summary

Wildbound is a 2D top-down fantasy battle royale built on **C++20**, **raylib 5.5**, and **Lua 5.4**. Prior phases (Phases 1–19) established tilemaps, player movement with axis-separated wall-sliding, combat hitboxes/hurtboxes, enemy AI, modular character compositing, five biomes, village interiors, procedural audio, player UI, and gameplay asset integration (items, inventory, projectiles, armor mitigation).

This audit evaluates the codebase ahead of **Phase 20: Physics & Collision System Extension**. The goal is to extend Wildbound with a modular, data-driven collision architecture featuring:
1. Formal bitfield collision layers and bilateral masks.
2. Unified collider representations (physical vs. trigger AABBs).
3. Stateful trigger volume tracking (`Enter`, `Stay`, `Exit`).
4. Decoupled overlap detection.
5. Collision event dispatching with a deferred mutation queue to prevent iterator invalidation and dangling references.
6. A documented, deterministic simulation update sequence.
7. Enhanced collision debugging (visual overlays and DevUI inspector).

---

## 2. Existing Collision & Physics Architecture

### 2.1 Static World Geometry (`wildbound::world::World`)
- **Storage**: `std::vector<CollisionBox> m_staticCollisionCache` in `World.hpp` / `World.cpp`.
- **Generation**: `World::RebuildCollisionCache()` traverses:
  1. `m_tilemap->GetObjects()`: extracts environmental object footprints marked `HasCollision()`.
  2. Tilemap tile layers (`"Collision Metadata"`, `"Collision"`): generates `tw × th` (32×32 px) `CollisionBox` entries for non-zero GIDs.
- **Data Structure**: `struct CollisionBox`:
  ```cpp
  struct CollisionBox {
      Rectangle bounds{ 0.0f, 0.0f, 0.0f, 0.0f };
      CollisionType type = CollisionType::Solid; // Solid, Impassable, Water, Walkable, Trigger
      std::string tag;
      bool isTrigger = false;
  };
  ```
- **Queries**:
  - `World::GetCollisionBoxes(const Rectangle& queryArea)`: linear search returning intersecting boxes.
  - `World::CheckCollision(const Rectangle& queryBox)`: early-out test returning `true` on the first solid intersection.

### 2.2 Character & Dynamic Entity Collision (`wildbound::entities`)
- **Base Footprint**: `Entity::GetCollisionBounds()` and `Entity::GetWorldBounds()`.
- **Character Configuration**: `CharacterCollisionConfig`:
  - `width` (default 16.0f), `height` (default 12.0f), `offsetX` (0.0f), `offsetY` (0.0f), `enabled` (true).
  - Represents the feet footprint anchored at the character's bottom-center coordinate `(m_position.x, m_position.y)`.
- **Movement Resolution (`Player::ResolveMovement`)**:
  - Discrete axis-separated collision resolution against `world.GetCollisionBoxes()`:
    1. Steps `m_position.x += delta.x`, evaluates X penetration against solid boxes, and resolves position to the contact edge.
    2. Steps `m_position.y += delta.y`, evaluates Y penetration against solid boxes, and resolves position to the contact edge.
    3. Clamps `m_position` to world boundaries.
  - This design provides smooth 2D tile-based wall sliding without sticking on corners.

### 2.3 Combat Hit & Hurtbox Detection (`wildbound::combat`)
- **Melee Attack Hitboxes**: `HitDetection::ComputeAttackHitbox(attackerPos, facing, attackDef)` computes directional attack bounds (e.g. 28×24 px in front of the attacker).
- **Hurtboxes**: Derived from `Combatant::GetCollisionBounds()`.
- **Intersection**: Evaluated via `HitDetection::Intersects(attackHitbox, targetHurtbox)`.
- **Projectiles (`ProjectileSystem`)**: Simulates arrows traveling at 480 px/s with circular hit radius (`hitRadius = 14.0f`) tested against `combatant->GetCollisionBounds()`.

### 2.4 World Pickups & Loot Containers
- **Pickups (`World::ProcessPickups`)**: Euclidean distance test `Vector2Distance(playerPos, itemPos) <= 32.0f`.
- **Loot Containers (`World::GetLootContainerNear`)**: Radial search `Vector2Distance(playerPos, chestPos) <= radius`.
- **Building Doors (`BuildingManager::CheckEntrancePrompt`)**: AABB point containment check `CheckCollisionPointRec(playerPos, doorwayRect)`.

---

## 3. Entity Identity & Lifecycle Analysis

| Entity Type | Identifier Type | Generation / Stability | Destruction Strategy |
|---|---|---|---|
| `Player` | `uint32_t` (`m_id = 1`) | Static / Persistent | Never deleted during match |
| `Enemy` | `uint32_t` (`m_id = 100+`) | Monotonically increasing `m_nextEnemyId` in `EnemySpawner` | Deferred: `Enemy::IsPendingDestruction()` after death anim |
| `WorldItem` | `uint64_t` (`m_id = 1+`) | Monotonically increasing `m_nextWorldItemId` in `World` | Instant removal upon inventory insertion in `ProcessPickups` |
| `LootContainer` | `std::string` (`id`) | Authored map / script string ID | Static world entity |
| `Projectile` | `uint64_t` (`id = 1+`) | Monotonically increasing `m_nextProjectileId` in `ProjectileSystem` | Flagged `active = false` upon impact or lifetime expiration |

### Lifecycle Evaluation
- Entity IDs across all subsystems are stable and unique.
- Entities are created and destroyed during `Update()`. In existing code, enemy removal occurs via `m_enemies.erase(it)` inside `EnemySpawner::Update()`.
- **Integration Risk Identified**: If collision callbacks dispatch user code or gameplay events while a collision-pair container is being iterated, calling `DestroyEntity()` or `UnregisterCollider()` directly would invalidate iterators or cause dangling references.
- **Required Architectural Solution**: A **Deferred Mutation Queue** inside the collision subsystem. Any `AddCollider`, `RemoveCollider`, `SetColliderEnabled`, or entity destruction request received during simulation processing must be buffered in a staging queue and applied only at defined synchronization points.

---

## 4. Gap Analysis: Missing Capabilities in Existing Engine

1. **No Formal Collision Layers**:
   - Collision filtering is currently hardcoded using `CollisionType` (`Solid`, `Water`, etc.) or string tags (`"player"`, `"goblin"`, `"skeleton"`).
   - There is no unified, high-performance bitfield representation for categories like `World`, `Player`, `Enemy`, `Wildlife`, `Projectile`, `Pickup`, `Trigger`, `Building`, `Hazard`, `Interactable`.
2. **No Bilateral Collision Masks**:
   - Entities cannot configure which layers they collide with or ignore.
   - Hardcoded `if (box.IsSolid())` or `if (faction != targetFaction)` logic is fragmented across multiple subsystems.
3. **No Stateful Trigger Volumes**:
   - There is no subsystem tracking whether an entity entered, stayed inside, or exited an interaction or trigger volume. All current trigger queries are stateless single-frame point or box queries.
4. **No Centralized Overlap Detection**:
   - Overlap queries are re-implemented ad-hoc in `World`, `HitDetection`, `ProjectileSystem`, and `BuildingManager`.
5. **No Event-Driven Collision Callbacks**:
   - There are no `OnCollisionEnter`, `OnCollisionStay`, `OnCollisionExit`, `OnTriggerEnter`, `OnTriggerStay`, `OnTriggerExit` callbacks.
6. **No Deterministic Collision Update Order**:
   - Collision queries occur whenever individual entities update (`Player::UpdateWithWorld`, `EnemySpawner::Update`, `ProjectileSystem::Update`), leading to arbitrary execution order.
7. **Scattered Collision Debugging**:
   - `WorldRenderer` draws red rectangles for static boxes; `CombatSystem` draws blue/yellow hitboxes; `DevUI` has no dedicated collision tab or layer toggles.

---

## 5. Reusable Components & Non-Negotiable Invariants

1. **Preserve `Player::ResolveMovement`**:
   - The existing axis-separated wall-sliding logic in `Player.cpp` works reliably and has dedicated regression tests. Phase 20 must NOT replace or alter this core character sliding behavior.
2. **Preserve `CombatSystem` & Hitboxes**:
   - Combat hitboxes and hurtboxes must remain independent of physical blocking colliders. An attack hitbox is a temporary query volume, not a physical collision body.
3. **Preserve `World::RebuildCollisionCache`**:
   - Static map geometry extracted from TMX/JSON maps must feed directly into the `World` collision layer without altering map loading or asset pipelines.
4. **No Third-Party Physics Engines**:
   - Do not pull in Box2D, Chipmunk, Bullet, or PhysX. Wildbound's requirements are 100% satisfied with clean C++20 AABBs and bitmasks.

---

## 6. Recommended Phase 20 Implementation Architecture

```
src/physics/
├── CollisionTypes.hpp            (Layer enum bitflags, CollisionMask, CollisionShape, ColliderType)
├── CollisionFilter.hpp           (Bilateral mask evaluation, self-exclusion, utility helpers)
├── Collider.hpp                  (AABB collider representation, local offsets, layers, masks, triggers)
├── CollisionEvent.hpp            (CollisionEnter/Stay/Exit, TriggerEnter/Stay/Exit event structs)
├── OverlapDetector.hpp           (Pure mathematical AABB overlap and intersection helpers)
├── TriggerVolume.hpp             (Trigger volume component & tracker)
├── CollisionDebugRenderer.hpp    (Raylib debug visualization with layer color-coding)
├── CollisionManager.hpp          (Centralized coordinator, deferred mutation queue, deterministic tick)
└── CollisionManager.cpp          (Update pipeline, pair deduplication, event dispatching)
```

### Deterministic Update Sequence:
1. **Apply Queued Structural Mutations**: Drain pending adds, removes, enables, and disables.
2. **Synchronize Collider Bounds**: Transform local offsets by owning entity positions.
3. **Broad-Phase / Potential Pair Generation**: Iterate colliders, sort candidate pairs by `(min(idA, idB), max(idA, idB))` to eliminate order dependence.
4. **Collision Filtering**: Bilateral mask evaluation `(A.mask & B.layer) != 0 && (B.mask & A.layer) != 0`.
5. **Narrow-Phase Overlap Detection**: Fast AABB intersection tests.
6. **Event Generation & Deduplication**: Compare current overlaps against previous tick overlaps to classify `Enter`, `Stay`, `Exit`.
7. **Callback Dispatch**: Deliver events to registered listeners in deterministic order.
8. **Finalize State**: Store current overlap state for next frame comparison.

---

## 7. Mandatory Prerequisite Sign-Off

- [x] Stable entity IDs verified (`uint32_t`, `uint64_t`).
- [x] Safe entity lifecycle analyzed (deferred mutations required and designed).
- [x] All Phase 19 systems verified and passing (21/21 CTest suites green).
- [x] No missing dependencies or external physics libraries required.
- [x] Clear division between physical collision, triggers, and combat hitboxes established.

**Audit Status**: PREREQUISITES SATISFIED. Ready for Phase 20 implementation.

