# Wildbound — Building Asset Specification (Phase 10)

## 1. Architectural Overview & Design Philosophy

Buildings in Wildbound are landmark environmental structures that anchor settlements, delineate pathways, provide cover during combat, and define the fantasy identity of the Forest biome.

Unlike simple decorative props or natural vegetation, buildings require strict spatial and architectural standardization to support:
1. **Physical Navigability**: Decoupling visual roof overhangs from physical ground wall collisions.
2. **Dynamic Depth Occlusion ($Y$-Sorting)**: Correct layering as players walk in front of, behind, and beside structures.
3. **Future Interior Transitions**: Decoupled exterior artwork and interior room maps, supporting seamless door transitions in subsequent phases without refactoring existing assets.

---

## 2. Structural Layering & Occlusion Model

```
        +-----------------------------------------+
        |             Pitched Roof                |  <- Rendered above walls
        |          (Visual Overhang)              |     (Occludes entities behind)
        +-----------------------------------------+
        |                 Walls                   |  <- Primary structure
        |   [Window]     [Door]      [Window]     |     (Depth-sorted with player)
  ======+=========================================+======  Ground Level
        |       [ Physical Collision Box ]        |  <- Solid AABB footprint
        +-----------------------------------------+     (Walkable in front/doorway)
```

### 2.1 Single Sprite vs. Multi-Layer Evaluation
Wildbound evaluates buildings across three architectural patterns:
1. **Single Complete Sprite (Selected Standard for Phase 10)**:
   - *Rationale*: Integrates immediately into the existing `WorldRenderer` and `EnvironmentObject` pipeline without requiring multi-pass composite shaders or breaking existing map formats.
   - *Anchor Rule*: The ground anchor $(x_{\text{center}}, y_{\text{base}})$ is placed at the front wall baseline.
   - *Depth Sorting*: The entire building sorts dynamically against entities at `pos.y = anchor.y`. When the player is South of the anchor, the player renders in front of the building; when North of the anchor, the building naturally covers the player.
2. **Future Interior & Roof Cutaway (Roadmap for World Expansion)**:
   - For structures designated with `"interior_compatibility": { "supported": true }`, exterior doors act as spatial trigger zones (`EntryZone`).
   - Entering triggers either a map transition to `maps/interiors/<map_reference>.json` or an alpha fade of the roof layer, revealing an internal tilemap layer.

---

## 3. Building Inventory & Spatial Data

All 18 buildings specify standardized spatial coordinates in their companion JSON metadata:

| Asset ID | Building Type | Canvas $(W \times H)$ | Ground Anchor | Collision $(W \times H)$ | Door Location | Roof Dimensions | Interior Reference |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| `house_cottage_001` | Small Wooden Cottage | $80 \times 80$ px | $(40, 76)$ | $64 \times 40$ px | $(40, 74)$ | $76 \times 34$ px | `interior_house_cottage_001` |
| `house_stone_small_001` | Small Stone Cottage | $80 \times 80$ px | $(40, 76)$ | $64 \times 40$ px | $(40, 74)$ | $76 \times 34$ px | `interior_house_stone_small_001` |
| `house_village_001` | Village Residence | $96 \times 88$ px | $(48, 84)$ | $76 \times 44$ px | $(48, 82)$ | $88 \times 38$ px | `interior_house_village_001` |
| `house_residence_large_001` | Large Manor House | $112 \times 96$ px | $(56, 92)$ | $92 \times 48$ px | $(46, 90)$ | $106 \times 42$ px | `interior_house_residence_large_001` |
| `house_twostory_001` | Two-Story House | $120 \times 112$ px | $(60, 108)$ | $96 \times 54$ px | $(50, 106)$ | $110 \times 46$ px | `interior_house_twostory_001` |
| `cabin_forest_001` | Forest Log Cabin | $72 \times 72$ px | $(36, 68)$ | $56 \times 36$ px | $(28, 66)$ | $66 \times 28$ px | `interior_cabin_forest_001` |
| `cabin_hunting_001` | Hunting Cabin | $72 \times 72$ px | $(36, 68)$ | $56 \times 36$ px | $(28, 66)$ | $66 \times 28$ px | `interior_cabin_hunting_001` |
| `cabin_woodland_001` | Woodland Porch Cabin | $80 \times 76$ px | $(40, 72)$ | $64 \times 38$ px | $(32, 70)$ | $74 \times 32$ px | `interior_cabin_woodland_001` |
| `shop_general_001` | General Goods Store | $96 \times 88$ px | $(48, 84)$ | $76 \times 44$ px | $(36, 82)$ | $88 \times 34$ px | `interior_shop_general_001` |
| `shop_market_001` | Market Merchant Shop | $88 \times 80$ px | $(44, 76)$ | $70 \times 40$ px | $(32, 74)$ | $82 \times 32$ px | `interior_shop_market_001` |
| `blacksmith_forge_001` | Village Blacksmith | $112 \times 96$ px | $(56, 92)$ | $90 \times 48$ px | $(66, 90)$ | $102 \times 36$ px | `interior_blacksmith_forge_001` |
| `storage_shed_001` | Wooden Storage Shed | $64 \times 64$ px | $(32, 60)$ | $50 \times 32$ px | $(32, 58)$ | $58 \times 28$ px | `interior_storage_shed_001` |
| `storage_warehouse_001` | Supply Warehouse | $96 \times 88$ px | $(48, 84)$ | $78 \times 44$ px | $(48, 82)$ | $86 \times 36$ px | `interior_storage_warehouse_001` |
| `watchtower_wood_001` | Wooden Watchtower | $64 \times 112$ px | $(32, 108)$ | $40 \times 36$ px | $(32, 106)$ | $48 \times 20$ px | `interior_watchtower_wood_001` |
| `watchtower_stone_001` | Stone Watchtower | $64 \times 120$ px | $(32, 116)$ | $44 \times 40$ px | $(32, 114)$ | $48 \times 22$ px | `interior_watchtower_stone_001` |
| `ruin_wall_broken_001` | Broken Stone Wall | $64 \times 40$ px | $(32, 36)$ | $52 \times 16$ px | None | None | None (Open exterior) |
| `ruin_building_abandoned_001`| Abandoned Stone Ruin | $96 \times 80$ px | $(48, 76)$ | $74 \times 40$ px | $(48, 74)$ | None | `interior_ruin_building_001` |
| `ruin_tower_small_001` | Small Ruined Tower | $64 \times 88$ px | $(32, 84)$ | $44 \times 36$ px | $(32, 82)$ | None | None (Open exterior) |

---

## 4. Architectural Details by Category

### 4.1 Residential Houses
- **Cottages (`house_cottage_001`, `house_stone_small_001`, `house_village_001`)**: Single-story buildings with $35^\circ$ pitched gables. Roofs extend $6$ px beyond exterior walls to form eaves, sheltering doorways from rain.
- **Large Residences (`house_residence_large_001`, `house_twostory_001`)**: Manor and town structures featuring cross-gables, timber framing, multi-pane casement windows, and second-floor jetties that project over the ground footprint.

### 4.2 Cabins
- Built using horizontal chinked roundwood logs.
- Chimneys are anchored on the lateral exterior wall ($x = +18$ to $+24$ px from anchor), keeping the roofline clean while providing distinctive vertical silhouettes.

### 4.3 Commercial & Production Buildings
- **General Store (`shop_general_001`)**: Features a dual-color fabric awning over a display window, inviting interaction.
- **Village Blacksmith (`blacksmith_forge_001`)**: Combines a fortified stone hearth, an oversized flue chimney with heavy soot gradient, and an exterior covered lean-to housing an iron anvil and a coal bunker.

### 4.4 Defensive Watchtowers
- **Wooden Watchtower (`watchtower_wood_001`)**: Heavy timber post frame with cross-braces, elevated enclosed sentry deck at $y = -70$ px, and perimeter railing.
- **Stone Watchtower (`watchtower_stone_001`)**: Tapered ashlar granite turret with arrow slits and crenellated roof parapet.
- *Collision Footprint Note*: The collision footprint applies exclusively to the ground base ($40 \times 36$ px), allowing players to walk freely around the tower base without getting blocked by the elevated lookout platform.

---

## 5. Metadata Schema for Buildings

Every building includes specialized fields inside its companion JSON:

```json
{
  "id": "house_cottage_001",
  "name": "Wooden Cottage",
  "category": "building",
  "subcategory": "houses",
  "biomes": ["forest"],
  "sprite": "environment/buildings/houses/house_cottage_001.png",
  "dimensions": { "width": 80, "height": 80 },
  "anchors": { "ground": [40, 76] },
  "collision": {
    "enabled": true,
    "shape": "rectangle",
    "offset": [8, 36],
    "size": [64, 40]
  },
  "rendering": {
    "layer": "solid_structures",
    "y_sort": true
  },
  "entrance": [40, 76],
  "door": [40, 74],
  "windows": [[24, 60], [56, 60]],
  "roof": { "width": 76, "height": 34 },
  "interior_compatibility": {
    "supported": true,
    "map_reference": "interior_house_cottage_001"
  },
  "provenance": {
    "creator": "Wildbound Development Studio",
    "source": "Wildbound Phase 10 Production Environment Asset",
    "license": "CC0 1.0 Universal"
  }
}
```

