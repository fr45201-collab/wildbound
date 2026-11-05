# Cosmetics Store & Wardrobe Debugger (Phase 26)

## 1. Overview & Tooling Purpose

The **Cosmetics Store & Wardrobe Debugger** is integrated into `DevUI` under **Tab 17: Cosmetics Store** (`src/ui/DevUI.cpp`).
It provides developers and QA engineers with full introspection and control over the cosmetic catalog, local ownership records, prototype unlocks, filtering, and live appearance synchronization.

```
+---------------------------------------------------------------------------------------+
| Wildbound Developer Suite (F1)                                                        |
+---------------------------------------------------------------------------------------+
| [Metrics] [Combat] [World] ... [Inventory] [Cosmetics Store (Tab 17)]                 |
+---------------------------------------------------------------------------------------+
| Phase 26 Cosmetics Store & Wardrobe Debugger                                          |
| Total Cosmetics: 98 | Owned: 12 | Locked: 86                                          |
|---------------------------------------------------------------------------------------|
| Ownership State Actions (Local Prototype):                                            |
| [Unlock All (Prototype)]  [Reset to Defaults]  [Save Ownership]  [Reload Ownership]   |
|---------------------------------------------------------------------------------------|
| Interface Mode:  [Store View           v]                                             |
| Category:        [All                  v]                                             |
| Search Filter:   [hood                ]  [Clear]                                      |
|---------------------------------------------------------------------------------------|
| Character Preview Controls:                                                           |
| [Rotate Left]  [Rotate Right]  [Anim: IDLE]  [Confirm Changes]  [Reset Preview]       |
|---------------------------------------------------------------------------------------|
| Displayed Cosmetics: 4                                                                |
| ID                | Name             | Slot     | Rarity    | Status | Actions        |
|-------------------|------------------|----------|-----------|--------|----------------|
| hat_explorer_hood | Explorer Hood    | Headwear | Rare      | OWNED  | [Prev] [Equip] |
| hat_straw_001     | Straw Hat        | Headwear | Common    | LOCKED | [Prev] [Unlock]|
+---------------------------------------------------------------------------------------+
```

---

## 2. Feature Matrix

### 2.1 Ownership Metrics & Overview
- Displays active statistics: Total registered cosmetics in catalogue, Total owned cosmetics count, and Total locked count.
- Updates dynamically whenever an unlock or reset transaction occurs.

### 2.2 Global Testing Actions
- **Unlock All (Prototype)**: Iterates every registered cosmetic definition in `CosmeticRegistry` and grants it with acquisition tag `"dev_unlock_all"`.
- **Reset to Defaults**: Invokes `CosmeticsOwnershipManager::ResetOwnership()`, revoking all acquired items and restoring starter equipment.
- **Save Ownership**: Forces immediate atomic disk serialization to `user_data/cosmetics/cosmetic_ownership.json`.
- **Reload Ownership**: Hot-reloads ownership records directly from disk.

### 2.3 Live Filtering & Search
- **Category Filter Combo**: Allows instant filtering across Featured, All, Hairstyles, Clothing, Armor, Headwear, Face, Backpacks, Footwear, and Special.
- **Search Query Input**: Real-time substring query matching across name, description, ID, category, and rarity.

### 2.4 Interactive Inspection Table
- Full scrolling ImGui table listing every matching cosmetic.
- Displays colored rarity badges matching canonical tier colors.
- Provides per-row one-click actions:
  - `[Preview]`: Previews cosmetic immediately on the character doll.
  - `[Equip]`: Equips owned item to player character.
  - `[Revoke]`: Re-locks an owned item for testing lock gating.
  - `[Unlock]`: Grants a single locked item via prototype transaction.

