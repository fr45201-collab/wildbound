# Wildbound — Phase 23 Repository Audit

## Loot and Inventory System Architecture Audit

**Engineer**: Senior C++20 Gameplay Systems Engineer  
**Date**: November 2026  
**Status**: Completed & Verified  

---

## 1. Executive Audit Overview

Before implementing Phase 23, an exhaustive source-level audit was conducted across all existing inventory, item, world loot, combat, interaction, UI, audio, and asset management systems.

The objective is to identify existing data structures, determine reusable components, avoid duplication, and construct a robust, unified Loot & Inventory architecture.

---

## 2. Existing Systems & Codebase Verification

### 2.1 Item Definitions & Data (`src/items/`)
* **`ItemConfig` / `ItemDefinition` (`src/items/ItemData.hpp`)**:
  - Exists. Represents static data definitions loaded from `data/items/item_catalog.json` (40 Phase 16 items) and `data/weapons/weapon_catalog.json` (11 weapons + 1 arrow ammunition).
  - Fields: `id`, `displayName`, `category`, `subcategory`, `description`, `visuals` (`iconTextureId`, `worldTextureId`), `biomeTags`, `gameplayTags`, `rarityTier`, `worldDimensions`, `stackable`, `maxStackSize`, `weight`, `consumable` (`healAmount`, `staminaAmount`, `effectType`), `weapon`, `armor`.
  - `ItemCategory` enum currently includes: `Collectible`, `Consumable`, `CraftingMaterial`, `NaturalResource`, `SpecialItem`, `Weapon`, `Armor`, `Clothing`. Needs explicit `Ammunition` category.
* **`ItemInstance` (`src/items/ItemInstance.hpp`)**:
  - Exists. Decouples runtime mutable state (`instanceId`, `definitionId`, `quantity`, `durability`) from static definition.
  - Implements `CanStackWith(const ItemInstance& other, int maxStackSize)`.
* **`ItemManager` (`src/items/ItemManager.hpp` / `.cpp`)**:
  - Exists. Loads catalog JSON, indexes definitions by ID, provides category/biome lookups, and validates assets with AMS.

### 2.2 Inventory System (`src/items/Inventory.hpp` / `.cpp`)
* **`Inventory` Class**:
  - Exists and actively used by `Player` and `LootContainer`.
  - Fixed capacity vector of `std::optional<ItemInstance>` (default 24 slots).
  - Methods: `AddItem`, `AddItemById`, `RemoveItem`, `RemoveItemById`, `SwapSlots`, `SplitStack`, `MoveSlot`, `GetItemCount`, `HasItem`, `FindFirstSlotWithItem`, `Clear`, `IsFull`.
  - Transactional: `AddItem` returns remaining unadded count; slot modifications happen only when validated.
* **Missing in `Inventory`**:
  - Item dropping (`DropItem`): Spawning world item and safely removing from inventory.
  - Contextual item usage (`UseItem`): Delegating to a dedicated `ConsumableSystem`.
  - Stack merging helper (`MergeStacks`).

### 2.3 Ground Loot & World Representations (`src/world/`)
* **`WorldItem` (`src/world/WorldItem.hpp` / `.cpp`)**:
  - Exists. Represents physical items on the ground with sinusoidal floating bob and contact drop shadow.
  - Holds `uint64_t m_id`, `Vector2 m_position`, `ItemInstance m_item`, `float m_pickupRadius` (36px).
  - Renders 32x32 world sprite via `TextureManager` with fallback text.
* **`World` (`src/world/World.hpp` / `.cpp`)**:
  - Manages `std::vector<std::unique_ptr<WorldItem>> m_worldItems`.
  - Spawning API: `SpawnWorldItem(position, item)` and `SpawnWorldItem(definitionId, position, quantity)`.
  - Cleanup: `UpdateWorldItems(dt)` cleans up items marked collected.
  - Legacy proximity pickup loop: `ProcessPickups(player, itemMgr, audioSys)`.
* **`LootContainer` (`src/world/LootContainer.hpp` / `.cpp`)**:
  - Exists. Represents chests containing an internal `Inventory` instance.
  - Implements `LootAll(targetInv, itemMgr, audioSys)` with atomic item transfer.
  - Integrated with Phase 22 `InteractionManager`.

### 2.4 Interaction System Integration (Phase 22)
* **`InteractionManager` & `InteractionExecutor` (`src/interaction/`)**:
  - Category `InteractionCategory::Collectible` maps to ground `WorldItem`s.
  - Supports `PickupPolicy::Manual` (contextual `[E] Collect` prompt) and `PickupPolicy::Automatic` (immediate proximity transfer).
  - Transaction safety: `AddItem` is committed first; only if accepted is `worldItem->MarkCollected()` or quantity decremented.

### 2.5 Combat & Ammunition Systems (`src/combat/`, `src/weapons/`)
* **`ProjectileSystem` (`src/combat/ProjectileSystem.hpp` / `.cpp`)**:
  - `SpawnArrow(origin, direction, speed, damage, ownerId, faction)` simulates physical arrow ballistics.
* **`Player::TryAttack` (`src/entities/Player.cpp`)**:
  - Currently spawns an arrow if a bow is equipped, but **does not verify or consume arrows from inventory**.
* **Arrow Ammunition Definition**:
  - Registered in `data/weapons/weapon_catalog.json` (`"id": "arrow_iron_001"`).

### 2.6 Player Health & Consumable Usage (`src/combat/`, `src/entities/`)
* **`HealthComponent` (`src/combat/HealthComponent.hpp` / `.cpp`)**:
  - Manages `m_currentHealth`, `m_maxHealth`, `IsDead()`, `IsAlive()`, `Heal(amount)`, `TakeDamage(amount)`.
  - `Heal(amount)` clamps to `m_maxHealth`.
* **`Player::UseConsumable` (`src/entities/Player.cpp`)**:
  - Currently contains rudimentary inline healing logic. Needs extraction into a clean, reusable `ConsumableSystem`.

### 2.7 UI System (`src/ui/UIManager.hpp` / `.cpp`)
* **`UIManager::RenderInventory`**:
  - Renders 24-slot grid (6x4) with category tabs, selection highlights, and details card.
  - Action buttons currently only have "USE" (calls `player->UseConsumable`) and "EQUIP".
  - Missing: "DROP ONE", "DROP ALL", "SPLIT STACK", capacity indicator, feedback notifications, and arrow ammo counter.

---

## 3. Gap Analysis & Missing Functionality

| Capability | Current Status | Required Action for Phase 23 |
| :--- | :--- | :--- |
| **Ground Loot Spawning** | Basic `World::SpawnWorldItem` exists | Formalize ground loot API, integrate drop positions, support drop velocity/scatter |
| **Item Dropping** | Completely missing | Implement `LootDropSystem` / `Inventory::DropItem` with 2-phase transactional safety |
| **Consumable System** | Rudimentary logic in `Player.cpp` | Build dedicated `ConsumableSystem` supporting Healing, Stamina, Food, and Antidotes |
| **Healing Validation** | Unconditional `Heal()` | Prevent healing if already at full health; return explicit `ItemUseResult` |
| **Ammunition System** | Arrows simulated, but not consumed | Enforce ammunition requirement in `Player::TryAttack`; consume arrows from inventory |
| **Item Stacking & Splitting**| Basic `SplitStack` and `MoveSlot` | Enhance with `MergeStacks`, strict max-stack enforcement, and UI controls |
| **Inventory UI Actions** | Only USE and EQUIP | Add DROP ONE, DROP ALL, SPLIT, capacity readouts, and feedback banners |
| **Modal Context Control** | `OpenInventory` opens screen | Wire Phase 21 `InputContextType::Inventory` to suppress gameplay input |
| **Developer Debugger** | No inventory debug tab | Add DevUI Tab 16 ("Loot & Inventory") with item inspection and test loot spawner |

---

## 4. Reusable Components & Integration Plan

1. **Reuse `items::Inventory`**: Extend with dropping and consumable delegation rather than rewriting.
2. **Reuse `world::WorldItem`**: Maintain as the authoritative world representation for ground loot.
3. **Reuse `items::ItemManager`**: Centralize definitions for all 40 items + weapons + ammunition.
4. **Reuse Phase 22 `InteractionManager`**: Retain `InteractionCategory::Collectible` for manual/automatic pickup.
5. **Reuse `combat::HealthComponent`**: Authoritative destination for healing consumables.
6. **Reuse `combat::ProjectileSystem`**: Target for arrow consumption and firing.
7. **Reuse `ui::UIManager`**: Enhance native raylib inventory window without Dear ImGui.
8. **Reuse `audio::AudioSystem`**: Route `"Item.Pickup"`, `"Item.Drop"`, `"Item.Consume"`, `"Item.Equip"`, `"Combat.OutOfAmmo"`.

---

## 5. Duplicate System Risks & Prevention

* **Risk 1: Separate Ground Loot Manager vs `World::m_worldItems`**:
  - *Mitigation*: Do not introduce a parallel `GroundLootManager`. Extend `WorldItem` and use `World` as the single owner of in-game ground loot entities.
* **Risk 2: UI Maintaining Local Quantities**:
  - *Mitigation*: The player's `Inventory` instance is the single authoritative source of truth. UI only inspects and triggers operations through `Inventory` APIs.
* **Risk 3: Weapons Modifying Inventory Directly**:
  - *Mitigation*: `Player::TryAttack` queries ammunition through `Inventory::RemoveItemById("arrow_iron_001", 1)` via an explicit ammunition query helper.

---

## 6. Implementation Strategy & Roadmap

1. **Step 1: Item Data & Ammunition Classification**:
   - Add `ItemCategory::Ammunition` to `ItemData.hpp`.
   - Register arrow ammunition definition in `ItemManager`.
2. **Step 2: Core Consumable Framework (`src/items/ConsumableSystem.hpp` / `.cpp`)**:
   - Implement `ItemUseResult` and data-driven effect application (Health, Stamina, Food).
3. **Step 3: Inventory Extension & Stacking (`src/items/Inventory.hpp` / `.cpp`)**:
   - Add `MergeStacks`, `DropItem`, and capacity checks.
4. **Step 4: Item Dropping System**:
   - Implement two-phase drop logic: spawn `WorldItem` in front of player $\to$ remove from inventory slot.
5. **Step 5: Ammunition Integration with Ranged Combat**:
   - Wire arrow verification and consumption into `Player::TryAttack` for Bows.
6. **Step 6: Enhanced Player-Facing Inventory UI (`UIManager.cpp`)**:
   - Add DROP, SPLIT, USE, capacity counters, and modal input context synchronization.
7. **Step 7: Developer Debugging Tools (DevUI Tab 16)**:
   - Provide real-time inventory inspection, ground loot list, and test loot spawning.
8. **Step 8: Automated CTest Suite (`tests/inventory/test_inventory.cpp`)**:
   - Author Test 25 covering all 11 required test areas.
9. **Step 9: Complete Documentation & Implementation Report**:
   - 11 documents in `docs/inventory/` and root report.

