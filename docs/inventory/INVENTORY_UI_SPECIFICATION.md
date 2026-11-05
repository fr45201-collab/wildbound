# Wildbound — Inventory UI Specification

## 1. Visual Layout (640x360 Virtual Canvas)
The Player-facing Inventory UI is implemented in `wildbound::ui::UIManager::RenderInventory` using custom raylib rendering primitives:

```
+-----------------------------------------------------------------------------------+
|  [CLOSE]                     INVENTORY & EQUIPMENT              Capacity: 12 / 24 |
|-----------------------------------------------------------------------------------|
|  [All]  [Coins/Gems]  [Potions/Food]  [Materials]  [Resources]  [Weapons]  [Ammo] |
|-----------------------------------------------------------------------------------|
|  +--------------------------------+  +-----------------------------------------+  |
|  |  +----+ +----+ +----+ +----+   |  |  [Item Name]                            |  |
|  |  | 01 | | 02 | | 03 | | 04 |   |  |  Type: Consumable  | Rarity: Common     |  |
|  |  +----+ +----+ +----+ +----+   |  |  Quantity: 5 / 10                       |  |
|  |  +----+ +----+ +----+ +----+   |  |  [ 48x48 Icon Preview ]                 |  |
|  |  | 05 | | 06 | | 07 | | 08 |   |  |                                         |  |
|  |  +----+ +----+ +----+ +----+   |  |  Description lore and properties.       |  |
|  |  +----+ +----+ +----+ +----+   |  |                                         |  |
|  |  | 09 | | 10 | | 11 | | 12 |   |  |  [ USE / EQUIP ]  [ SPLIT ]  [ DROP 1 ]|  |
|  |  +----+ +----+ +----+ +----+   |  |                                         |  |
|  |  (4 rows x 6 columns = 24)     |  |  [ DROP ALL ]                           |  |
|  +--------------------------------+  +-----------------------------------------+  |
+-----------------------------------------------------------------------------------+
```

---

## 2. Interactive Features
- **Capacity Readout**: Displays `Capacity: Filled / 24` in top header.
- **Quantity Badges**: Slots containing stacks `> 1` render a high-contrast pill with count at bottom-right.
- **Filter Tabs**: Instant category filtering (`All`, `Coins/Gems`, `Potions/Food`, `Materials`, `Resources`, `Weapons/Gear`, `Ammo`).
- **Context Action Buttons**:
  - `USE`: Visible for consumables (e.g. restores health/stamina).
  - `EQUIP`: Visible for equippable gear (weapons, armor, shirts).
  - `SPLIT`: Visible when selected stack `> 1`; moves half into the first free slot.
  - `DROP 1`: Drops 1 unit onto the ground in front of player.
  - `DROP ALL`: Drops entire stack onto the ground in front of player.
- **Contextual Sound Feedback**: Tab switching (`"UI.TabSwitch"`), item selecting (`"UI.ItemSelect"`), item splitting (`"UI.ItemSplit"`), and dropping (`"Item.Drop"`).

