# Inventory UI Integration Specification

## 1. Overview & Mechanics

The Inventory UI (`wildbound::ui::InventoryUI`) provides a responsive, transactional 24-slot backpack management interface that seamlessly integrates with the Phase 23 Inventory and Loot subsystem.

It allows players to:
1. View and sort items across a $6 \times 4$ slot grid.
2. Filter item views by category tabs (All, Consumables, Equipment, Materials, Resources).
3. Inspect detailed item statistics, descriptions, and rarity in an adjacent inspector panel.
4. Perform safe transactional actions: Use / Consume, Equip, Unequip, Split Stack, Drop 1, and Drop All.

---

## 2. Layout & Grid Composition

Positioned centrally within the $640 \times 360$ canvas:
- **Panel Bounds**: $(X = 50, Y = 30, W = 540, H = 300)$.
- **Left Column (Grid)**: $(X = 70, Y = 75)$
  - 24 slots arranged as 6 columns by 4 rows.
  - Slot dimensions: $36 \times 36\text{ px}$ with $6\text{ px}$ padding.
  - Category filter tabs situated above the grid $(Y = 48)$.
- **Right Column (Inspector Tooltip)**: $(X = 330, Y = 75, W = 240, H = 220)$
  - Rarity banner, item icon, item name, type classification, quantity/durability.
  - Stat modifiers (Damage, Defense, Health Restore, Cooldown).
  - Contextual action buttons: `[E] Use / Equip`, `[X] Drop 1`, `[Z] Drop All`.

```
+-----------------------------------------------------------------------------------+
| INVENTORY (14/24 Slots)                                             [ESC] Close   |
| [ ALL ]  [ CONSUMABLES ]  [ EQUIPMENT ]  [ MATERIALS ]  [ RESOURCES ]             |
+-----------------------------------------------------------------------------------+
|  +----+ +----+ +----+ +----+ +----+ +----+  |  [ITEM INSPECTOR]                   |
|  | #1 | | #2 | | #3 | | #4 | | #5 | | #6 |  |  Health Potion                      |
|  +----+ +----+ +----+ +----+ +----+ +----+  |  Rarity: Uncommon                   |
|  | #7 | | #8*| | #9 | |#10 | |#11 | |#12 |  |  Quantity: 5                        |
|  +----+ +----+ +----+ +----+ +----+ +----+  |  Effect: +50 HP over 3.0s           |
|  |#13 | |#14 | |#15 | |#16 | |#17 | |#18 |  |                                     |
|  +----+ +----+ +----+ +----+ +----+ +----+  |  [ E ] Consume Potion               |
|  |#19 | |#20 | |#21 | |#22 | |#23 | |#24 |  |  [ X ] Drop 1     [ Z ] Drop All    |
|  +----+ +----+ +----+ +----+ +----+ +----+  +-------------------------------------+
+-----------------------------------------------------------------------------------+
```

---

## 3. Navigation & Item Focus

- **Grid Coordinates**: Focus is maintained as $(col, row) \in [0..5] \times [0..3]$.
- **Index Calculation**: $\text{slotIndex} = row \times 6 + col$.
- **Directional Navigation**:
  - `UI_LEFT` / `UI_RIGHT`: Moves column with edge wrapping.
  - `UI_UP` / `UI_DOWN`: Moves row with edge clamping.
- **Mouse Selection**: Hovering over any slot sets $(col, row)$ immediately; clicking selects the slot.

---

## 4. Transactional Drop & Action Integration

To prevent race conditions, item duplication, or orphaned game objects, the Inventory UI delegates all state mutations directly to `wildbound::items::InventoryManager`:

### 4.1 Using / Equipping Items
```cpp
bool success = inventory.UseItem(slotIndex, *player, audioSys);
if (success) {
    notificationQueue.Push("Item used successfully", NotificationType::Success);
}
```

### 4.2 Dropping Items into World Space
```cpp
// Drop 1
bool dropped = inventory.DropItem(slotIndex, 1, *world, playerPos, playerFacing, audioSys);

// Drop All
bool droppedAll = inventory.DropAll(slotIndex, *world, playerPos, playerFacing, audioSys);
```
- The `InventoryManager` handles removing the item from the container slot, creating the physical `WorldItem` at the player's facing position, and playing the `item.drop` sound effect.
- The UI immediately updates its slot graphics in the next frame.

