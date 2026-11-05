# WILDBOUND — GAMEPLAY SYSTEM DEPENDENCY MAP

## 1. System Interaction Architecture

```
                                  ┌────────────────────────┐
                                  │      Engine Loop       │
                                  │       (Game.cpp)       │
                                  └───────────┬────────────┘
                                              │
               ┌──────────────────────────────┼──────────────────────────────┐
               ▼                              ▼                              ▼
      ┌─────────────────┐            ┌─────────────────┐            ┌─────────────────┐
      │   World State   │            │  Player State   │            │ Audio / UI Loop │
      └────────┬────────┘            └────────┬────────┘            └────────┬────────┘
               │                              │                              │
       ┌───────┴───────┐              ┌───────┴───────┐              ┌───────┴───────┐
       ▼               ▼              ▼               ▼              ▼               ▼
┌─────────────┐ ┌─────────────┐┌─────────────┐ ┌─────────────┐┌─────────────┐ ┌─────────────┐
│  Tilemap /  │ │ WorldItem / ││  Inventory  │ │  Equipment  ││  UIManager  │ │ AudioSystem │
│ Environment │ │  Container  ││   System    │ │   System    ││ (Player UI) │ │(Events/Bus) │
└──────┬──────┘ └──────┬──────┘└──────┬──────┘ └──────┬──────┘└──────┬──────┘ └──────┬──────┘
       │               │              │               │              │               │
       │               └──────────────┼───────────────┘              │               │
       │                              ▼                              │               │
       │                     ┌─────────────────┐                     │               │
       │                     │  ItemDefinition │                     │               │
       │                     │  & ItemInstance │                     │               │
       │                     └────────┬────────┘                     │               │
       │                              │                              │               │
       ▼                              ▼                              ▼               ▼
┌────────────────────────────────────────────────────────────────────────────────────────────┐
│                             Asset Management System (AMS)                                  │
│                 Textures • Spritesheets • Audio Buffers • Shaders • Fonts                  │
└────────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Subsystem Dependency & Authority Matrix

| Subsystem | Authoritative Owner | Dependencies | Consumers / Observers |
|---|---|---|---|
| **`ItemDefinition`** | `ItemManager` | JSON Metadata (`data/items/item_catalog.json`) | Inventory, WorldItem, LootContainer, UI |
| **`ItemInstance`** | `Inventory` / `WorldItem` | `ItemDefinition` | Inventory, WorldItem, Combat, Tooltips |
| **`Inventory`** | `Player` | `ItemInstance`, `ItemDefinition` | UIManager (Inventory Screen), HUD, Pickup |
| **`EquipmentSystem`** | `Player` | `Inventory`, `ModularCharacter`, `WeaponConfig` | DamageSystem, Renderer, UIManager |
| **`WorldItem`** | `World` | `ItemInstance`, AMS (World Sprites) | `PickupSystem`, `WorldRenderer` |
| **`PickupSystem`** | `Game` / `World` | `WorldItem`, `Player` (Inventory), `AudioSystem` | Player Movement / Interaction |
| **`LootContainer`** | `World` | `ItemInstance`, `ItemDefinition`, `AudioSystem` | Player Interaction, WorldRenderer |
| **`WeaponSystem`** | `Player` / `Combat` | `WeaponConfig`, `CombatSystem`, `AudioSystem` | `AttackSystem`, `HitDetection`, Projectiles |
| **`ProjectileSystem`** | `CombatSystem` | Raylib Physics/AABB, `Combatant`, `AudioSystem` | `WorldRenderer`, `DamageSystem` |
| **`UIManager`** | `Game` | Authoritative Gameplay State (`Player`, `Inventory`) | Player Window Canvas |
| **`AudioSystem`** | `Game` | Audio Catalog, Event Dispatcher, raylib stream | All Gameplay Subsystems |

---

## 3. Data Integrity & Invariant Rules

1. **No Duplicate State**:
   * UI components NEVER hold copy-on-write balances or independent item counts.
   * `InventoryScreen` reads directly from `Player::GetInventory()` slots.
2. **Transactional Transfer**:
   * World Item -> Player Inventory: The item is inserted into inventory first. Only upon successful return is the `WorldItem` marked for destruction and pickup audio dispatched.
   * Inventory -> World Drop: The item is subtracted from inventory first; only then is `WorldItem` spawned at the player's ground anchor.
3. **Equip Atomicity**:
   * Equipping an armor or weapon removes or swaps the item instance in inventory, calculates defense / weapon statistics, and updates modular appearance layers simultaneously.

