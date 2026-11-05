# Item and Inventory Integration

## 1. Overview & Architecture

Wildbound's inventory architecture enforces strict decoupling between static catalog metadata and mutable runtime gameplay state:

- **[`ItemDefinition`](file:///c:/dev/Wildbound/src/items/ItemData.hpp)**: Immutable catalog template loaded by [`ItemManager`](file:///c:/dev/Wildbound/src/items/ItemManager.hpp) from `data/items/item_catalog.json`. Holds static properties: `id`, `displayName`, `category`, `rarityTier`, `stackable`, `maxStackSize`, `weight`, and visual texture references.
- **[`ItemInstance`](file:///c:/dev/Wildbound/src/items/ItemInstance.hpp)**: Mutable runtime entity representation containing `instanceId`, `definitionId`, `quantity`, and `durability`.
- **[`Inventory`](file:///c:/dev/Wildbound/src/items/Inventory.hpp)**: Authoritative container holding fixed-size slots (`std::vector<std::optional<ItemInstance>>`). Defaults to 24 slots for player character storage and 12 slots for world loot containers.

```
       +---------------------------------------------+
       |             ItemManager (AMS)               |
       |  Loads data/items/item_catalog.json (40 defs) |
       +----------------------+----------------------+
                              |
                     queries metadata
                              |
    +-------------------------v-------------------------+
    |                 Inventory Container                |
    |  - Authoritative 24 slots (fixed capacity)        |
    |  - Transactional AddItem, RemoveItem, Split, Swap |
    +------------+--------------------------+-----------+
                 |                          |
       Player Inventory              Loot Containers
     (Player::GetInventory)        (LootContainer::GetInv)
                 |                          ^
                 +--------- LootAll --------+
```

---

## 2. Inventory Operations & Invariants

### 2.1 Transactional Item Ingestion (`AddItem`)
When adding items to an `Inventory`:
1. **Existing Stack Scanning**: The container first searches for partially filled slots sharing the same `definitionId`. It increments the stack up to `maxStackSize` (from `ItemDefinition` or default).
2. **Empty Slot Insertion**: Any remaining count allocates the next available `std::nullopt` slot.
3. **Overflow Protection**: If the inventory is full and cannot accommodate the entire requested quantity, `AddItem` returns the remaining unallocated count ($> 0$). No items are lost or truncated silently.

### 2.2 Stack Splitting & Slot Organization
- `SplitStack(sourceSlot, targetSlot, count)`: Safely moves a sub-quantity from one slot into an empty target slot. Validates boundary conditions ($0 < count < source.quantity$).
- `SwapSlots(slotA, slotB)`: Atomically swaps two slot contents.
- `RemoveItem(slotIndex, quantity)`: Decrements quantity; if quantity reaches $\le 0$, the slot is reset to `std::nullopt`.

---

## 3. World Pickups Integration

Ground items are managed by [`WorldItem`](file:///c:/dev/Wildbound/src/world/WorldItem.hpp) and [`World`](file:///c:/dev/Wildbound/src/world/World.hpp):
- **World Rendering**: Rendered at world coordinates with contact drop shadow, vertical floating sine bobbing ($\pm 2\text{px}$), and quantity badges for stacks.
- **Transactional Collection (`World::ProcessPickups`)**:
  - Distance check: `CanPickup` evaluates distance between player and pickup ($\le 36\text{px}$).
  - Transfer: Items are ingested into `player.GetInventory().AddItem()`.
  - Resolution: If all units are accepted, the `WorldItem` is marked collected (`MarkCollected()`) and pruned during `UpdateWorldItems()`. If partially accepted, the ground stack is decremented to the remaining amount.
  - Audio: Dispatches `"Item.Pickup"` event via `AudioSystem`.

---

## 4. Loot Containers (Chests & Caches)

Containers are represented by [`LootContainer`](file:///c:/dev/Wildbound/src/world/LootContainer.hpp):
- **Container Types**: Wooden chests, iron strongboxes, ancient caches.
- **Locking & Key Requirements**: Containers support `isLocked = true` and `requiredKeyId` (e.g. `"key_dungeon_001"`).
- **Player Interaction**:
  - Proximity check: $R \le 48\text{px}$.
  - Prompt: HUD displays `"Press [E] or [F] to Open Chest"`.
  - Unlocking: Validates whether player inventory holds `requiredKeyId`. If present, unlocks and dispatches `"Environment.ChestOpen"`.
  - Transactional Looting (`LootAll`): Empties container slots into the player's inventory up to available free capacity. Remaining loot stays safely in the chest.

---

## 5. Verification & Tests

Covered extensively by [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp):
- `TestItemDefinitionsAndInstances()`: Catalog loading, definition validation, instance creation.
- `TestInventoryOperations()`: Auto-stacking, stack limits, overflow rejection, splitting, swapping.
- `TestWorldItemsAndPickups()`: Proximity queries, transactional pickup, inventory-full ground persistence.
- `TestLootContainers()`: Locked containers, key requirement verification, full container loot transfers.

