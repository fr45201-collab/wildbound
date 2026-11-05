# Wildbound — Item Metadata Specification

## 1. Overview & Architecture

Wildbound's Item and Collectible system uses structured JSON metadata adhering to strict engine conventions. Each item has:
1. An individual definition file located at `data/items/<category>/<id>.json`.
2. Inclusion in the master catalog located at `data/items/item_catalog.json`.
3. Asset registration in the Asset Management System (AMS) manifest at `assets/manifests/asset_manifest.json`.

---

## 2. JSON Schema & Field Specifications

### Individual Item Definition (`data/items/<category>/<id>.json`)

```json
{
  "id": "con_health_potion_small_001",
  "display_name": "Minor Health Potion",
  "category": "consumable",
  "subcategory": "potion",
  "description": "A small glass vial filled with glowing crimson curative fluid.",
  "visuals": {
    "icon": "tex_con_health_potion_small_001_icon",
    "world": "tex_con_health_potion_small_001_world"
  },
  "biome_tags": [
    "universal"
  ],
  "rarity": "common",
  "world_dimensions": {
    "width": 32,
    "height": 32
  }
}
```

### Field Definitions

| Field | Type | Required | Description |
| :--- | :--- | :--- | :--- |
| `id` | `string` | **Yes** | Unique identifier matching convention: `<cat_prefix>_<name>_<version>` (e.g., `con_health_potion_small_001`). |
| `display_name` | `string` | **Yes** | Player-facing name rendered in UI tooltips, logs, and inspect panels. |
| `category` | `string` | **Yes** | Primary category: `"collectible"`, `"consumable"`, `"material"`, `"resource"`, or `"special"`. |
| `subcategory` | `string` | **Yes** | Finer taxonomy classification (e.g., `"currency"`, `"potion"`, `"ore"`, `"flora"`, `"key"`). |
| `description` | `string` | **Yes** | Flavor text and descriptive summary. |
| `visuals.icon` | `string` | **Yes** | Asset identifier registered in AMS manifest pointing to the 64×64 inventory icon. |
| `visuals.world` | `string` | **Yes** | Asset identifier registered in AMS manifest pointing to the 32×32 world sprite. |
| `biome_tags` | `array<string>` | **Yes** | Biomes where this item naturally spawns or is relevant (`"forest"`, `"desert"`, `"snow"`, `"swamp"`, `"mountain"`, or `"universal"`). |
| `rarity` | `string` | **Yes** | Rarity tier: `"common"`, `"uncommon"`, `"rare"`, `"epic"`, or `"legendary"`. |
| `world_dimensions` | `object` | **Yes** | Width and height bounding dimensions in world pixels (standard `32`×`32`). |

---

## 3. Master Catalog Format (`data/items/item_catalog.json`)

The master catalog aggregates all 40 items under a single root array for atomic, high-performance loading during engine startup:

```json
{
  "version": "1.0.0",
  "total_items": 40,
  "items": [
    {
      "id": "col_gold_coin_001",
      "display_name": "Gold Coin",
      "category": "collectible",
      ...
    }
  ]
}
```

---

## 4. AMS Manifest Registration (`assets/manifests/asset_manifest.json`)

Textures are registered in the AMS manifest under the `textures` array with explicit width, height, and relative file path:

```json
{
  "id": "tex_col_gold_coin_001_icon",
  "file": "items/icons/col_gold_coin_001_icon.png",
  "category": "items/icons",
  "width": 64,
  "height": 64
},
{
  "id": "tex_col_gold_coin_001_world",
  "file": "items/world/col_gold_coin_001_world.png",
  "category": "items/world",
  "width": 32,
  "height": 32
}
```

Total AMS assets registered after Phase 16: **412**.

---

## 5. C++ Runtime Integration & Validation

### Data Structures (`src/items/ItemData.hpp`)
* `enum class ItemCategory`: Type-safe enum representation (`Collectible`, `Consumable`, `CraftingMaterial`, `NaturalResource`, `SpecialItem`).
* `struct ItemVisuals`: Stores string IDs for AMS resolution.
* `struct ItemConfig`: Complete in-memory structure with `IsValid()` and `HasBiomeTag()` helpers.

### Subsystem Operations (`src/items/ItemManager.hpp` / `src/items/ItemManager.cpp`)
* `Initialize(catalogPath)`: Reads catalog JSON via `nlohmann::json`, parses all items, builds lookup index.
* `GetItem(id)` / `HasItem(id)`: $O(1)$ lookup via hash map.
* `GetItemsByCategory(category)`: Filtered query for inventory screens and DevUI.
* `GetItemsByBiome(biomeTag)`: Spatial query for environmental loot tables and biome spawners.
* `ValidateAssets(assetMgr, outErrors)`: Automated integrity validation verifying every referenced icon and world texture exists in the AMS registry and resolves on the filesystem.

