# Wildbound — Points of Interest (POI) Catalog

## 1. Catalog Architecture & Distribution

The Isle of Wildbound features 14 strategic Points of Interest (POIs) designed to distribute player traffic across all geographical biomes, incentivize tactical rotations, and provide varied combat environments.

Each POI is assigned a permanent identifier, coordinate anchor, tactical classification, and loot tier rating.

---

## 2. Complete POI Catalog

| POI ID | Name | Biome | Coordinates $(X, Y)$ | Type | Landmark Class | Primary Loot Tier | Description |
|---|---|---|---|---|---|---|---|
| `poi_greenwood_village` | **Greenwood Village** | Forest | $(4160, 5920)$ | Settlement | Major Urban | Tier 3 (High) | Central temperate trading settlement featuring town hall, forge, shops, and houses. |
| `poi_sunfall_outpost` | **Sunfall Outpost** | Desert | $(7680, 7520)$ | Settlement | Sentry Fort | Tier 3 (High) | Sandstone desert fortress and trade outpost monitoring southern caravan trails. |
| `poi_frostpeak_village` | **Frostpeak Village** | Snow | $(7840, 2560)$ | Settlement | Alpine Haven | Tier 3 (High) | Northern timber longhouse settlement huddled around communal hearthfires. |
| `poi_mirewood_settlement` | **Mirewood Settlement** | Swamp | $(2400, 2720)$ | Settlement | Wetland Village | Tier 2 (Medium) | Stilted huts and duckboard walkways navigating the decaying murky bog. |
| `poi_highstone_village` | **Highstone Village** | Mountain | $(5120, 2080)$ | Settlement | Mountain Keep | Tier 3 (High) | Highland stonecutter fortress guarding alpine granite passes and waterfalls. |
| `poi_ancient_fortress` | **Ancient Citadel Fortress** | Forest | $(5120, 4800)$ | Fortress | Historic Citadel | Tier 4 (Epic) | Heavily contested central crossroads fortress featuring ruined ramparts and twin watchtowers. |
| `poi_desert_temple` | **Buried Temple of Sunfall** | Desert | $(7200, 6400)$ | Temple | Sacred Ruins | Tier 4 (Epic) | Ancient partially buried sandstone temple honoring forgotten desert deities. |
| `poi_desert_oasis` | **Sunfall Oasis & Bazaar** | Desert | $(7680, 6720)$ | Oasis | Natural Landmark | Tier 2 (Medium) | Freshwater oasis ringed by palm scrub, shaded trade stalls, and water cisterns. |
| `poi_forest_shrine` | **Forgotten Forest Shrine** | Forest | $(3200, 6800)$ | Shrine | Sacred Relic | Tier 3 (High) | Secluded woodland stone shrine with an ancient restorative well and offering altars. |
| `poi_frostfall_camp` | **Frostfall Expedition Camp** | Snow | $(6800, 3400)$ | Camp | Explorer Post | Tier 2 (Medium) | Abandoned northern survey encampment with hunting shelters and meat racks. |
| `poi_frostpeak_glacier` | **Frostpeak Glacial Cavern** | Snow | $(7200, 2200)$ | Landmark | Natural Rift | Tier 4 (Epic) | Sub-zero glacial ice chasm containing ancient preserved relic chests. |
| `poi_drowned_ruins` | **Drowned Mire Ruins** | Swamp | $(3000, 3800)$ | Ruins | Sunken Ruins | Tier 3 (High) | Sunken stone watchtower and broken walls partially swallowed by the bog void. |
| `poi_highstone_mine` | **Highstone Mine & Cave** | Mountain | $(4400, 2080)$ | Mine | Underground Portal | Tier 3 (High) | Abandoned iron mine entrance and cave archway portal into the deep granite crags. |
| `poi_river_crossing` | **Great River Stone Bridge** | Forest | $(5500, 5280)$ | Bridge | Tactical Chokepoint | Tier 2 (Medium) | Massive fortified stone bridge spanning the Great Forest River between Forest and Desert. |

---

## 3. Tactical Dynamics by POI Type

### 3.1 Settlements (Urban Combat)
- Multiple interior buildings providing hard cover against projectile snipers.
- High container density ($6 - 8$ loot caches per settlement).
- Dense street networks enabling flanking and tactical retreats.

### 3.2 Fortresses & Ruins (Contested High-Tier Hotspots)
- Central positioning (e.g. Ancient Citadel Fortress) makes them prime targets for initial drop contests and mid-game circle rotations.
- Concentrated Tier 4 (Epic/Ancient) loot caches with high weapon and armor rarity.

### 3.3 Bridges & Chokepoints (Funnel Ambush Zones)
- Players must commit to crossing narrow paths ($48\text{ px} - 64\text{ px}$ wide) or take extended swim/nature detour routes.
- Excellent positioning for scouting and ambush ambushes.

