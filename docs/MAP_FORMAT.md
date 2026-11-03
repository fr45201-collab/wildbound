# Map Format Specification — Wildbound Engine

## 1. Supported Specifications

### A. Tiled JSON Map (`.json`, `.tmj`)
* Version: Tiled JSON 1.10+
* Encoding: Uncompressed array or comma-separated integer string (`data: [1, 2, ...]`).
* Coordinate System: Orthogonal grid, 32×32 pixels per tile.

Example structure:
```json
{
  "width": 80,
  "height": 60,
  "tilewidth": 32,
  "tileheight": 32,
  "tilesets": [
    {
      "firstgid": 1,
      "name": "forest_tileset",
      "tilewidth": 32,
      "tileheight": 32,
      "columns": 8,
      "tilecount": 64,
      "image": "tilesets/forest/forest_tileset.png"
    }
  ],
  "layers": [ ... ]
}
```

### B. Tiled TMX XML (`.tmx`)
* XML root `<map orientation="orthogonal" width="80" height="60" tilewidth="32" tileheight="32">`.
* Contains `<tileset firstgid="1" ...><image source="..." /></tileset>`.
* Contains `<layer id="..." name="..."><data encoding="csv">...</data></layer>`.
* Contains `<objectgroup id="..." name="..."><object id="..." name="..." type="..." x="..." y="..." width="..." height="..." /></objectgroup>`.

---

## 2. Automated Map Validation (`MapValidator`)
Every map is verified using `MapValidator::Validate(tilemap, assetMgr)`:
* **Geometry**: Width and height must be strictly positive (`> 0`).
* **Tile Size**: Tile width and height must match engine standard (`32 × 32`).
* **Tileset Completeness**: Every tileset referenced must exist in `AssetManager`.
* **GID Integrity**: Every non-zero GID in each layer must map to a valid tileset within `[firstGid, firstGid + tileCount)`.
* **Object Properties**: Validates that objects with collision enabled have positive collision bounds.
