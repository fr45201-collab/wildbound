# Wildbound — Settlement & Village Layout

## 1. Overview & Architectural Hierarchy

Wildbound features 5 major settlement clusters situated strategically across the island's primary biomes. Each settlement possesses a unique architectural identity, functional district layout, defensible sightlines, and multiple approaches.

---

## 2. Settlement Profiles

### 2.1 Greenwood Village (Verdant Forest)
- **Coordinates**: $(X: 4160.0, Y: 5920.0)$ | **Radius**: $450.0\text{ px}$
- **Architectural Style**: Rustic Timber & Fieldstone Masonry
- **Structures**:
  - `Greenwood_Town_Hall` (`house_residence_large_001`, $128 \times 112\text{ px}$) — Civic leadership seat and central banquet hall.
  - `Greenwood_General_Store` (`shop_general_001`, $96 \times 80\text{ px}$) — Trading post and supply depot.
  - `Greenwood_Blacksmith` (`blacksmith_forge_001`, $112 \times 96\text{ px}$) — Working forge with anvil and water trough.
  - `Greenwood_Warehouse` (`storage_warehouse_001`, $96 \times 80\text{ px}$) — Grain and goods storage.
  - `Greenwood_Watchtower` (`watchtower_wood_001`, $48 \times 112\text{ px}$) — High wooden sentry tower overlooking north road.
  - Residential cottages, two-story houses, stone wells, street benches, direction signs.
- **Combat Flow**: Excellent close-range tactical cover; players can rotate through back alleys between houses to break pursuer line of sight.

### 2.2 Sunfall Outpost (Sunscorched Wastes)
- **Coordinates**: $(X: 7680.0, Y: 7520.0)$ | **Radius**: $400.0\text{ px}$
- **Architectural Style**: Sandstone Ashlar & Desert Fortifications
- **Structures**:
  - `Sunfall_Watchtower` (`watchtower_stone_001`, $48 \times 112\text{ px}$) — High stone vantage point monitoring the dunes.
  - `Sunfall_Bazaar_Shop` (`shop_market_001`, $96 \times 80\text{ px}$) — Open-air caravan trading pavilion.
  - `Sunfall_Residency_1 & 2` (`house_stone_small_001`, $80 \times 72\text{ px}$) — Thick-walled stone habitations resistant to desert heat.
  - `Sunfall_Storage` (`storage_shed_001`, $64 \times 56\text{ px}$) — Water and spice depot.
  - `Sunfall_Water_Cistern` (`well_stone_001`, $32 \times 36\text{ px}$) — Deep stone well tapping the subterranean aquifer.
- **Combat Flow**: Open desert street layout; watchtower provides dominant vertical suppression over caravan approaches.

### 2.3 Frostpeak Village (Frostfall Taiga)
- **Coordinates**: $(X: 7840.0, Y: 2560.0)$ | **Radius**: $400.0\text{ px}$
- **Architectural Style**: Heavy Spruce Timber & Sod-Insulated Cabins
- **Structures**:
  - `Frostpeak_Longhouse` (`cabin_woodland_001`, $80 \times 72\text{ px}$) — Central gathering hall with blazing stone hearth.
  - `Frostpeak_Lodge_1 & 2` (`cabin_hunting_001`, `cabin_forest_001`, $80 \times 72\text{ px}$) — Hunter and tracker quarters.
  - `Frostpeak_Watchtower` (`watchtower_wood_001`, $48 \times 112\text{ px}$) — Blizzard sentry post.
  - `Frostpeak_Storehouse` (`storage_shed_001`, $64 \times 56\text{ px}$) — Furred pelts and salted meat rations.
  - `Frostpeak_Campfire` (`smoke_campfire_anim`, $32 \times 32\text{ px}$) — Central communal bonfire.
- **Combat Flow**: Circular cabin arrangement surrounding the bonfire; dense perimeter pines provide flanking cover.

### 2.4 Mirewood Settlement (Murkwood Mire)
- **Coordinates**: $(X: 2400.0, Y: 2720.0)$ | **Radius**: $380.0\text{ px}$
- **Architectural Style**: Stilted Bog Huts & Elevated Duckboards
- **Structures**:
  - `Mirewood_Elder_Hut` (`cabin_forest_001`, $80 \times 72\text{ px}$) — Overgrown moss-covered timber dwelling.
  - `Mirewood_Hut_1 & 2` (`cabin_hunting_001`, `cabin_woodland_001`, $80 \times 72\text{ px}$) — Stilted wetland shelters.
  - `Mirewood_Watchpost` (`watchtower_wood_001`, $48 \times 112\text{ px}$) — Sentry platform rising above the bog mist.
  - `Mirewood_Lantern_1 & 2` (`lamp_post_001`, $16 \times 48\text{ px}$) — Guiding lanterns illuminating duckboard junctions.
- **Combat Flow**: Movement is channeled onto wooden duckboard walkway paths; falling into the surrounding mud exposes players to slow movement or dead ends.

### 2.5 Highstone Village (Iron Crag Highlands)
- **Coordinates**: $(X: 5120.0, Y: 2080.0)$ | **Radius**: $420.0\text{ px}$
- **Architectural Style**: Granite Fortress Masonry & Alpine Chasm Bridges
- **Structures**:
  - `Highstone_Fortress_Keep` (`house_residence_large_001`, $128 \times 112\text{ px}$) — Mountain stronghold guarding the northern crags.
  - `Highstone_Cottage_1 & 2` (`house_stone_small_001`, $80 \times 72\text{ px}$) — Miner and stonecutter cottages.
  - `Highstone_High_Watchtower` (`watchtower_stone_001`, $48 \times 112\text{ px}$) — Eagle-eye stone tower monitoring the mountain pass.
  - `Highstone_Bridge` (`bridge_wood_large_001`, $64 \times 48\text{ px}$) — Timber span spanning the waterfall gorge.
- **Combat Flow**: Highstone acts as the ultimate highland fortress with narrow cliff approach routes.

