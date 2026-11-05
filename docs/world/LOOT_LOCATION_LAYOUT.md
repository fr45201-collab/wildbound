# Wildbound — Loot Location & Tier Distribution Layout

## 1. Overview
The **Wildbound** Battle Royale island features 45 designated, persistent loot placement markers categorized across 5 container archetypes and 4 competitive loot tiers. These placements drive the early-to-mid-game exploration loop, incentivizing aggressive rotations toward high-risk, high-reward Points of Interest (POIs) while maintaining balanced peripheral scavenge density.

```
+------------------------------------------------------------------------------------+
|                                LOOT TIER SPECTRUM                                  |
|                                                                                    |
| [Tier 1: Poor]       [Tier 2: Standard]       [Tier 3: Rich]       [Tier 4: Legendary]
|  Outposts, Camps      Villages, Roads          Ruins, Watchtowers   Citadel, Peak   
|  Common/Uncommon     Uncommon/Rare            Rare/Epic            Epic/Legendary  
+------------------------------------------------------------------------------------+
```

---

## 2. Container Archetypes & Item Classes

| Container Type | Spawn Density | Primary Drop Profile | Associated Biomes / POIs |
| :--- | :--- | :--- | :--- |
| **`chest_wooden`** | 16 markers (35.6%) | Common-to-Rare equipment, low-tier potions, iron weapons, arrows. | Greenwood Village, Sunfall Outpost, roadside wagons. |
| **`chest_iron`** | 12 markers (26.7%) | Rare armor, steel weapons, medium health/mana consumables. | Highstone Village, Outposts, Watchtowers. |
| **`chest_golden`** | 6 markers (13.3%) | Epic weapons, high-tier armor, large elixirs, rare accessories. | Ancient Fortress, Frostpeak Sanctum, Crypt of the Nameless. |
| **`barrel_supplies`** | 7 markers (15.6%) | Field rations, bandages, raw ammunition, throwing items. | Market squares, fishing docks, bandit camps. |
| **`weapon_rack`** | 4 markers (8.9%) | Guaranteed melee weapons (swords, daggers, axes) and ranged bows. | Barracks, armories, blacksmith workshops. |

---

## 3. Spatial Allocation Across Points of Interest

### A. Central Citadel & High-Tier POIs
* **Ancient Fortress (`poi_ancient_fortress`)**:
  * 3 Golden Chests (`loot_fortress_throne`, `loot_fortress_armory`, `loot_fortress_dungeon`).
  * 2 Weapon Racks (`loot_fortress_courtyard_rack`).
  * Loot Tier: **Tier 4 (Legendary)**. High danger, center-of-map rotation bottleneck.
* **Frostpeak Sanctum (`poi_frostpeak_sanctum`)**:
  * 1 Golden Chest (`loot_sanctum_altar`).
  * 2 Iron Chests (`loot_sanctum_relic_vault`).
  * Loot Tier: **Tier 4 (Legendary)**. High altitude Taiga peak sanctuary.
* **Crypt of the Nameless (`poi_crypt_nameless`)**:
  * 1 Golden Chest (`loot_crypt_sarcophagus`).
  * 1 Iron Chest (`loot_crypt_alcove`).
  * Loot Tier: **Tier 3 (Rich)**. Deep Murkwood mire subterranean entrance.

### B. Major Settlements (Tier 2-3)
* **Greenwood Village**:
  * 4 Wooden Chests, 2 Supply Barrels, 1 Weapon Rack.
  * Tier: **Tier 2 (Standard)**. Beginner-friendly starting hub with consistent healing items.
* **Sunfall Outpost**:
  * 3 Wooden Chests, 2 Iron Chests, 1 Supply Barrel.
  * Tier: **Tier 2 (Standard)**. Desert trading post offering quick mobility gear.
* **Frostpeak Village**:
  * 3 Iron Chests, 1 Wooden Chest, 1 Supply Barrel.
  * Tier: **Tier 3 (Rich)**. Defensive northern shelter with cold-resistant gear.
* **Highstone Village**:
  * 3 Iron Chests, 1 Wooden Chest, 1 Weapon Rack.
  * Tier: **Tier 3 (Rich)**. Mining and forge settlement with heavy armor drops.
* **Mirewood Settlement**:
  * 3 Wooden Chests, 2 Supply Barrels.
  * Tier: **Tier 2 (Standard)**. Swamp stilt-village with potion components.

### C. Transit Camps & Wilderness Landmarks (Tier 1-2)
* **Oasis Ruins (`poi_desert_oasis`)**: 1 Iron Chest, 1 Supply Barrel.
* **Forgotten Watchtower (`poi_watchtower_south`)**: 1 Iron Chest, 1 Wooden Chest.
* **Bandit Stockade (`poi_bandit_stockade`)**: 2 Wooden Chests, 1 Supply Barrel.
* **Coastal Shipwreck (`poi_sunken_vessel`)**: 1 Iron Chest, 1 Wooden Chest.

---

## 4. Spacing, Collision, & Fair Play Guarantees
1. **No Collision Embedment**: Every loot marker coordinate has been tested against the static collision cache. No container overlaps buildings, tree trunks, or cliff geometry.
2. **Minimum Distance Spacing**: Chests in open fields are spaced $\ge 64$ world units apart; interior village clusters maintain $\ge 32$ units of separation to prevent click-overlap.
3. **Player Reachability**: All 45 loot markers have direct walkability verification on the 16px navigation grid.

