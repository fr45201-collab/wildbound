# Wildbound — Inventory System Architecture

## 1. Architectural Scope & Principles
The Wildbound Loot & Inventory System establishes a modular, deterministic, data-driven framework managing the player character's items, ground loot physical representations, stacking logic, consumable usage, and ammunition mechanics in C++20 and raylib 5.5.

```
+-----------------------------------------------------------------------------------+
|                              GAMEPLAY LOOP                                        |
|                                                                                   |
|  +--------------------+     Proximity / Manual [E]      +-----------------------+ |
|  |     Ground Loot    | -----------------------------> |   Player Inventory    | |
|  |    (`WorldItem`)   | <----------------------------- |      (24 Slots)       | |
|  +--------------------+     Two-Phase Drop Commit       +-----------------------+ |
|                                                              |         |          |
|                                       Consume / Use          |         | Attack   |
|                                             v                |         v          |
|                                   +-------------------+      |   +-------------+  |
|                                   | ConsumableSystem  |      |   | Bow Combat  |  |
|                                   | (Health, Stamina) |      |   | (Ammo Check)|  |
|                                   +-------------------+      |   +-------------+  |
|                                                              v                    |
|                                                    +--------------------+         |
|                                                    |     Player UI      |         |
|                                                    |   (raylib Modal)   |         |
|                                                    +--------------------+         |
+-----------------------------------------------------------------------------------+
```

### Key Architectural Tenets:
1. **Single Source of Truth**: Core simulation state is owned strictly by C++ components (`wildbound::items::Inventory`, `wildbound::world::WorldItem`, `wildbound::items::ConsumableSystem`). UI and developer tooling merely inspect and invoke authoritative methods.
2. **Transactional State Transitions**: All transfers (ground $\leftrightarrow$ inventory, container $\to$ inventory, inventory $\to$ ground) execute as atomic, two-phase commits. If world spawning fails or target capacity is exceeded, no source items are lost or duplicated.
3. **Modal Input Suppression**: When opening the inventory interface, the input manager pushes `InputContextType::Inventory`, blocking gameplay movement, attack, and sprint inputs while keeping UI action keys available.
4. **Data-Driven Configuration**: Item definitions, stack limits, consumable values, and ammunition properties are loaded from structured JSON specifications and cached within `wildbound::items::ItemManager`.

---

## 2. Component Taxonomy
- `Inventory`: Authoritative storage array of 24 fixed `std::optional<ItemInstance>` slots.
- `ItemInstance`: Lightweight runtime value struct with `instanceId`, `definitionId`, and `quantity`.
- `ItemConfig`: Static metadata blueprint containing categories, max stack limits, weight, rarity, and category-specific configs.
- `ConsumableSystem`: Static utility class validating usage rules (e.g. over-healing prevention) and applying health/stamina restoration.
- `WorldItem`: Interactive ground entity with pickup radius, floating bob animation, and elliptical shadow.
- `LootContainer`: In-world chest/container supporting open/lock states and transactional `LootAll` transfers.
- `UIManager`: Custom raylib rendering pass displaying the 24-slot inventory grid, item detail card, and action buttons.
- `DevUI`: Developer debugging suite tab providing real-time slot inspection, ground loot registry tables, and debug spawners.

