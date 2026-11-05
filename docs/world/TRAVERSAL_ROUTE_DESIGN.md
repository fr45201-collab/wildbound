# Wildbound — Traversal Route & Strategic Chokepoint Design

## 1. Route Topology & Connectivity
The island road network forms an interconnected transit lattice engineered specifically for 2D top-down Battle Royale pacing. Major routes link all 5 biome settlements to the central **Ancient Fortress**, with perimeter highways facilitating flanking rotations around the island's periphery.

```
                           [Highstone Village] (North)
                                    |
                            [Chasm Bridge]
                                    |
 [Mirewood Settlement] ----- [ANCIENT FORTRESS] ----- [Frostpeak Village]
        (West)                      |                        (NE)
                                    |
                           [Greenwood Bridge]
                                    |
                          [Greenwood Village] ======= [Sunfall Outpost]
                               (Southwest)           (Caravan Crossing, SE)
```

---

## 2. Primary Highways & Traversal Pacing

| Route ID | Route Name | Start Endpoint | End Endpoint | Width | Traversal (Run @ 120px/s) | Surface Type |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`route_forest_fortress`** | King's Forest Highway | Greenwood Village | Ancient Fortress | 4 tiles (128 px) | 12.3 seconds | Compact dirt / cobblestone |
| **`route_fortress_highstone`** | Mountain Scree Ascent | Ancient Fortress | Highstone Village | 4 tiles (128 px) | 22.7 seconds | Crag gravel & steps |
| **`route_fortress_sunfall`** | Great Caravan Trail | Ancient Fortress | Sunfall Outpost | 4 tiles (128 px) | 32.5 seconds | Sun-baked sandstone / planks |
| **`route_fortress_mirewood`** | Mirewood Causeway | Ancient Fortress | Mirewood Settlement | 4 tiles (128 px) | 30.6 seconds | Wood boardwalks / stilts |
| **`route_mountain_taiga_pass`** | Frostfall Mountain Pass | Highstone Village | Frostpeak Village | 4 tiles (128 px) | 23.3 seconds | Packed snow / carved stone |

---

## 3. Strategic Chokepoints

| Chokepoint ID | Name | Coordinate | Bottleneck Width | Strategic Function |
| :--- | :--- | :--- | :--- | :--- |
| **`cp_great_stone_bridge`** | Great Stone Bridge | (5504, 5280) | 96 px (3 tiles) | Primary eastern river crossing between Central Fortress and Desert Wastes. Flanked by deep water. |
| **`cp_mountain_chasm`** | Mountain Chasm Bridge | (5120, 2880) | 96 px (3 tiles) | Sole direct vehicular/foot path connecting Central Island to Iron Crag Highlands across the northern waterfall gorge. |
| **`cp_greenwood_timber`** | Greenwood Timber Bridge | (4640, 5376) | 96 px (3 tiles) | Primary western river crossing between Greenwood Village and Central Fortress. High foliage ambush territory. |

---

## 4. Tactical Rotation Dynamics
1. **Storm Rotations**: As the circle contracts toward the center, outer biome players must commit to bridge crossings or risk swim penalties / deep water barriers.
2. **Ambush & Counter-Play**: Chokepoints feature adjacent forest brush and rock outcroppings permitting defensive ambushes, while secondary mountain goat tracks and wide river fords offer stealthier, longer bypass options for observant squads.
3. **Pacing Balance**: No two adjacent major settlements require more than 35 seconds of uninterrupted running, ensuring players can loot a settlement and rotate across the first zone shrink without being caught by zone movement.

