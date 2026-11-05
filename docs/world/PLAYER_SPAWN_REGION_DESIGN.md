# Wildbound — Player Spawn Region Design & Deployment Specification

## 1. Executive Summary
The deployment and spawn architecture for the **Wildbound** Battle Royale island establishes 12 distinct spawn zones housing 48 collision-safe spawn coordinates. The design supports solo, duo, and squad deployments up to 48 players per match, providing balanced initial loot access while preventing instantaneous spawn-fratricide.

```
+------------------------------------------------------------------------------------+
|                               ISLAND SPAWN ZONES (12)                              |
|                                                                                    |
| [SZ-01: NW Marsh]       [SZ-02: North Taiga]        [SZ-03: NE Mountain Peak]     |
| [SZ-04: West River]     [SZ-05: Greenwood Village]  [SZ-06: Eastern Taiga Foothill]|
| [SZ-07: SW Glade]       [SZ-08: South Coast Forest] [SZ-09: Sunfall Dune Basin]   |
| [SZ-10: Oasis Veranda]  [SZ-11: Crag Watch]         [SZ-12: Forgotten Anchorage]   |
+------------------------------------------------------------------------------------+
```

---

## 2. Spawn Region Topology & Allocation

| Zone ID | Name | Regional Biome | Coordinate Center | Safety Radius | Marker Count | Primary POI Proximity |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`spawn_reg_nw_mire`** | Northwest Mire Spawns | Murkwood Mire | (2080, 2400) | 160 px | 4 | Mirewood Settlement (320 px) |
| **`spawn_reg_north_taiga`** | High Taiga Spawns | Frostfall Taiga | (7360, 2240) | 160 px | 4 | Frostpeak Village (480 px) |
| **`spawn_reg_ne_peak`** | Northeast Crag Spawns | Iron Crag Highlands | (5440, 1600) | 160 px | 4 | Highstone Village (450 px) |
| **`spawn_reg_west_river`** | West River Delta Spawns | Verdant Forest | (3200, 4800) | 160 px | 4 | River Crossing (300 px) |
| **`spawn_reg_greenwood`** | Greenwood Village Outskirts | Verdant Forest | (4480, 6240) | 160 px | 4 | Greenwood Village (250 px) |
| **`spawn_reg_east_taiga`** | Eastern Taiga Basin Spawns | Frostfall Taiga | (8320, 3520) | 160 px | 4 | Coastal Frozen Watchtower (400 px) |
| **`spawn_reg_sw_glade`** | Southwest Glade Spawns | Verdant Forest | (2240, 7040) | 160 px | 4 | Old Oak Shrine (350 px) |
| **`spawn_reg_south_coast`** | South Shore Spawns | Verdant Forest | (5120, 8000) | 160 px | 4 | Southern Docks (280 px) |
| **`spawn_reg_sunfall`** | Sunfall Basin Spawns | Sunscorched Wastes | (8000, 7200) | 160 px | 4 | Sunfall Outpost (320 px) |
| **`spawn_reg_oasis`** | Desert Oasis Periphery | Sunscorched Wastes | (7360, 6400) | 160 px | 4 | Desert Oasis Pool (200 px) |
| **`spawn_reg_crag_watch`** | Crag Pass Spawns | Iron Crag Highlands | (4800, 2880) | 160 px | 4 | Mountain Chasm Bridge (380 px) |
| **`spawn_reg_anchorage`** | Forgotten Anchorage | Sunscorched Wastes | (8640, 8320) | 160 px | 4 | Sunken Vessel Shore (300 px) |

---

## 3. Deployment Safety & Fair Play Rules

1. **Boundary Buffer Rule**:
   * No spawn marker is positioned closer than 192 pixels (6 tiles) to the outer playable barrier (`Rectangle{ 768, 768, 8704, 8704 }`).
   * Prevents instant out-of-bounds storm/hazard exposure upon game start.
2. **Terrain Walkability Check**:
   * Every spawn marker is required to reside on a walkable tile ($GID > 0$) with zero intersecting static collision geometry.
   * Confirmed via `NavigationGrid::IsWalkable(marker.worldPos) == true`.
3. **Inter-Player Spacing**:
   * Within each spawn zone, markers maintain a minimum Euclidean distance of $64.0$ world units ($2$ full tiles).
   * Prevents collision overlap on simultaneous drop and guarantees initial movement freedom.
4. **POI Travel Budget**:
   * All 12 spawn zones are located within $200$ to $500$ world units of an initial weapon/item cluster ($1.5$ to $4.0$ seconds of standard sprinting).

