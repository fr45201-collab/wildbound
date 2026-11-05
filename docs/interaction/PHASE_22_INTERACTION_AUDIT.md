# Wildbound — Phase 22 World Interaction System Audit

**Audit Date**: November 2026  
**Auditor**: Senior C++20 Gameplay Systems Architect  
**Workspace**: `c:\dev\Wildbound`  
**Status**: Prerequisites Verified — Proceeding with Implementation

---

## 1. Executive Summary

A comprehensive repository audit was conducted across Wildbound's C++20 codebase, physics and collision systems (Phase 20), input management architecture (Phase 21), gameplay asset integrations (Phase 19), UI system (Phase 18), and world systems (Phases 12–14).

All core architectural prerequisites for the **World Interaction System** are present, verified, and functioning. No required previous-phase systems are missing, eliminating any need for architectural stop conditions or duplicate systems.

---

## 2. Dependency & Subsystem Audit

| Subsystem / Feature | Source Location | Implementation Status | Integration Assessment |
|:---|:---|:---|:---|
| **Physics & Trigger Volumes** | `src/physics/TriggerVolume.hpp`, `CollisionManager.hpp` | Fully Implemented (Phase 20) | Supports stateful `Enter`, `Stay`, `Exit` events, AABB spatial queries (`QueryAABB`), and bilateral filtering. |
| **Collision Layers** | `src/physics/CollisionTypes.hpp` | Fully Implemented (Phase 20) | `CollisionLayer::Interactable` (`1u << 9`), `CollisionLayer::Pickup` (`1u << 5`), `CollisionLayer::Trigger` (`1u << 6`). |
| **Spatial Overlap Queries** | `src/physics/CollisionManager.cpp` | Fully Implemented (Phase 20) | `QueryAABB(box, mask)` returns all overlapping colliders with zero heap churn during queries. |
| **Collision Callbacks** | `src/physics/CollisionManager.hpp` | Fully Implemented (Phase 20) | `RegisterCallback(fn)` and `ICollisionListener` provide clean lifecycle observation. |
| **Logical Input Action** | `src/input/InputAction.hpp` | Fully Implemented (Phase 21) | `InputAction::Interact` (ID 11, category `General`, default: `E` / Gamepad `X/Square`). |
| **Input Contexts & Modality** | `src/input/InputContext.hpp` | Fully Implemented (Phase 21) | `Gameplay` context allows `Interact`; modal contexts (`Menu`, `Inventory`, `Wardrobe`, `Settings`, `Paused`) strictly block it. |
| **Input Buffering** | `src/input/InputBuffer.hpp` | Fully Implemented (Phase 21) | `ConsumeBufferedAction(InputAction::Interact)` handles 150 ms tolerance with single-consumption deduplication. |
| **Item & Inventory Engine** | `src/items/Inventory.hpp`, `ItemManager.hpp` | Fully Implemented (Phase 16/19) | 24-slot container with atomic auto-stacking, empty slot allocation, and overflow validation. |
| **World Pickups** | `src/world/WorldItem.hpp`, `World.cpp` | Fully Implemented (Phase 19) | 32×32 px world representation with floating bob, contact shadow, and `CanPickup(pos)`. |
| **Loot Containers** | `src/world/LootContainer.hpp`, `LootContainer.cpp`| Fully Implemented (Phase 19) | 12-slot container supporting locked states, required keys, `Open()`, and transactional `LootAll()`. |
| **Building Transitions** | `src/world/BuildingManager.hpp`, `BuildingData.hpp` | Fully Implemented (Phase 13) | Seamless zero-bleed transition between exterior map and 6 enterable village interiors. |
| **Centralized Audio System**| `src/audio/AudioSystem.hpp` | Fully Implemented (Phase 18) | Events for `"Environment.DoorOpen"`, `"Environment.DoorClose"`, `"Interact.ChestOpen"`, `"Interact.ItemPickup"`. |
| **UI Rendering Primitives** | `src/ui/UIManager.hpp`, `UIManager.cpp` | Fully Implemented (Phase 18) | 640×360 virtual canvas renderer with `DrawPanel`, `DrawButton`, `DrawIcon`, and gameplay HUD. |
| **Developer HUD** | `src/ui/DevUI.hpp`, `DevUI.cpp` | Fully Implemented (Dear ImGui) | 14 active tabs for real-time inspection, monitoring, and debugging. |

---

## 3. Existing Architecture & Reusability Analysis

1. **Ad-Hoc Interaction Fragmentation in `Game.cpp`**:
   - Currently, lines 521–546 in `src/core/Game.cpp` handle interactions via an imperative `if (interact)` block that checks `m_world->GetLootContainerNear(...)` and `m_buildingManager->CheckEntrancePrompt(...)`.
   - **Remedy**: Replace this fragmented block with a unified call to `InteractionManager::Update(...)` and `InteractionManager::TryInteract(...)`.

2. **Pickup Policy Synchronization**:
   - `World::ProcessPickups` currently executes automatic proximity pickups unconditionally.
   - **Remedy**: Introduce configurable `PickupPolicy` (`Manual` vs `Automatic`). Under `Manual` policy (the default for immersive tactical gameplay), ground items register as `InteractableCategory::Collectible` candidates requiring explicit `[E] Collect` interaction.

3. **Door Systems**:
   - Doors in the current settlement are implicit transitions inside `BuildingManager`.
   - **Remedy**: Introduce a dedicated, authoritative `Door` entity model in `src/interaction/` or `src/world/` supporting physical open/close/locked states, visual sprite toggles, and solid collision box synchronization with `CollisionManager`.

4. **Mechanisms & Environmental Interactables**:
   - Campfires, shrines, levers, switches, and pressure plates need generic representation.
   - **Remedy**: Create `InteractableComponent` and `InteractionDefinition` allowing any entity or prop to register interaction callbacks, cooldowns, and activation states.

---

## 4. Technical Risks & Mitigation

| Technical Risk | Likelihood | Impact | Mitigation Strategy |
|:---|:---:|:---:|:---|
| **Nondeterministic Target Selection** | Low | High | Enforce strict 5-stage selection ordering: (1) availability, (2) distance, (3) player facing angle, (4) interaction priority, (5) stable EntityId tie-breaker. |
| **Per-Frame Heap Allocations** | Medium | Medium | Pre-allocate candidate vectors; query spatial radius via bounded AABB without dynamic vector resizing in steady-state. |
| **Action Bleed Across Modals** | Low | High | Leverage Phase 21's `InputContextManager`. Interaction requests are strictly ignored unless `InputContextType::Gameplay` is active. |
| **Item Duplication on Pickup** | Low | Critical | Strict two-phase transactional commit: validate inventory capacity first, insert item, and only mark `WorldItem` collected upon confirmed insertion. |
| **Door Collision Desynchronization** | Medium | High | Maintain authoritative door state in C++. Toggle solid box collider in `CollisionManager` synchronously with door state changes. |

---

## 5. Recommended Implementation Order

1. **Core Data Structures & Registry (`src/interaction/`)**:
   - `InteractionTypes.hpp`: Categories (`Door`, `Building`, `Container`, `Collectible`, `Mechanism`, `Environmental`, `Special`), results (`Success`, `OutOfRange`, `TargetUnavailable`, etc.), policies.
   - `InteractionDefinition.hpp`: Data-driven prompt, range, priority, requirements, cooldowns.
   - `InteractableComponent.hpp`: Component attached to interactable entities.
   - `InteractionEvent.hpp`: Event structures and observer listeners.
2. **Interaction Pipeline**:
   - `InteractionValidator.hpp` / `.cpp`: Comprehensive requirement and range checks.
   - `InteractionExecutor.hpp` / `.cpp`: Behavior delegation to doors, buildings, containers, items, and mechanisms.
   - `InteractionManager.hpp` / `.cpp`: Central coordinator, proximity discovery, candidate ordering, active prompt provider.
3. **World Entities & Domain Interactions**:
   - Doors (`Door.hpp` / `.cpp`): States (`Closed`, `Open`, `Locked`), collision enable/disable toggling, audio feedback.
   - Mechanisms (`Mechanism.hpp` / `.cpp`): Levers, switches, pressure plates (automatic triggers vs manual).
   - Integration with `BuildingManager`, `LootContainer`, `WorldItem`, and `Inventory`.
4. **UI Prompt & Device Glyphs**:
   - Connect active prompt to `UIManager::RenderGameplayHUD`, displaying localized key glyph (`[E]` on keyboard, `[X]` on gamepad).
5. **Developer Tooling**:
   - Add Tab 15 ("World Interactions") to DevUI with candidate telemetry and visual overlays.
6. **Testing & Verification**:
   - Implement `tests/interaction/test_interaction.cpp` (`InteractionSystemTest`).
   - Run Release and Debug CTest across all 24 test suites.
7. **Documentation**:
   - Author 11 documents in `docs/interaction/` and copy completion report.

