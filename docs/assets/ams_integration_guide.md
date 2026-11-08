# Wildbound — AMS Integration & Runtime Technical Standards

## 1. Objective

This technical guide establishes how processed assets transition into the Asset Management System (AMS) and rendering pipelines while respecting system boundaries.

---

## 2. Technical System Boundaries

```
[Development-Time Asset Processing]
  - Python / Pillow scripts
  - assets/external/ & assets/processed/
              |
              v
[C++ Client Runtime]
  - AssetManager (AMS)
  - TextureManager (Raylib cache)
  - AnimationController (Clips & state machine)
  - WorldRenderer & ModularCharacter
              |
              | Network Protocol (Entity IDs, appearance hashes)
              v
[Dedicated Server / Coordinator]
  - Authoritative validation of cosmetic IDs and ownership
              |
              v
[Go Backend & PostgreSQL]
  - Persistent ownership tables (user_id, cosmetic_id)
  - NO raw PNG files, NO texture serving
```

---

## 3. AMS Registration Workflow

To integrate an asset into the AMS:
1. **Manifest Registration**: Add an entry into `assets/manifests/asset_manifest.json`:
   ```json
   {
     "id": "tex_desert_tiles_pilot",
     "file": "processed/biomes/desert/desert_tiles.png",
     "type": "texture",
     "category": "tilesets/desert",
     "source": "Kenney",
     "author": "Kenney",
     "license": "CC0 1.0 Universal",
     "commercial_use": true,
     "attribution_required": false,
     "width": 256,
     "height": 256,
     "status": "approved"
   }
   ```
2. **Spritesheet Slicing**: If the texture is a multi-frame spritesheet:
   ```json
   {
     "id": "sheet_desert_tiles_pilot",
     "file": "processed/biomes/desert/desert_tiles.json",
     "type": "spritesheet",
     "category": "tilesets/desert",
     "width": 32,
     "height": 32,
     "status": "approved"
   }
   ```
3. **Animation Registration**: If animated, define the clip in JSON and load via `AssetManager::RegisterAnimation`:
   ```json
   {
     "name": "anim_slime_bounce",
     "spritesheet": "sheet_slime_pilot",
     "frames": [0, 1, 2, 3],
     "frame_duration": 0.125,
     "loop": true
   }
   ```

---

## 4. Tiled Map Integration Standards

- New tilesets must be created as standalone external `.tsx` files referencing the processed tileset PNG in `assets/processed/`.
- Existing maps (`maps/world/battle_royale_island.json`, `maps/forest/forest_01.json`) must **never be modified** during pilot testing.
- Pilot tilesets will be tested in a dedicated pilot test map (`maps/test/pilot_test_map.json`).

