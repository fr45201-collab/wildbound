# Tiled Map Editor Integration Guide — Wildbound Engine

## 1. Supported Formats
Wildbound natively supports both major formats produced by [Tiled Map Editor](https://www.mapeditor.org/):
1. **JSON Map (`.json` or `.tmj`)**: Primary recommended format for clean serialization.
2. **TMX XML (`.tmx`)**: Standard Tiled XML format with CSV encoded tile data.

`MapLoader::LoadMap(filepath, assetMgr)` automatically determines the loader based on the file extension (`.json`, `.tmj`, or `.tmx`).

---

## 2. Authoring Configuration in Tiled
When creating a new map in Tiled:
* **Orientation**: Orthogonal.
* **Tile layer format**: CSV (Comma-Separated Values).
* **Tile render order**: Right Down.
* **Tile size**: 32 × 32 px.

---

## 3. Tileset Configuration
Tileset images should be placed in `assets/tilesets/<category>/`.
* Name the tileset matching the master asset ID (e.g. `forest_tileset`).
* When exporting, embed the tileset into the map or maintain relative image paths.
* `MapLoader` delegates all texture loading to `resources::AssetManager`, ensuring no redundant GPU textures are created.

---

## 4. Object Group Properties Specification
Environmental objects placed in object groups (such as `Environmental Objects`) support custom properties:

| Property Name | Type | Description |
|---|---|---|
| `collision` | `bool` | Whether the object creates a physical collision box (`true`/`false`). |
| `collision_type` | `string` | Collision class: `"solid"`, `"water"`, `"impassable"`, `"walkable"`, `"trigger"`. |
| `collision_width` | `float` | Custom collision box width in pixels. Defaults to object width. |
| `collision_height`| `float` | Custom collision box height in pixels. Defaults to lower 40-50% of entity height. |
