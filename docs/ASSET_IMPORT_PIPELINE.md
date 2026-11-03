# Wildbound Asset Import Pipeline

## 1. Import Workflow Overview
To maintain an orderly repository and prevent raw archives from polluting production directories, all incoming game assets follow a staged 8-step pipeline:

```
[ Raw Archive Download ]
          │
          ▼
[ Step 1: tools/asset_manager/imports/ ]
          │
          ▼
[ Step 2: License & Author Verification ]
          │
          ▼
[ Step 3: Format & Transparency Audit ]
          │
          ▼
[ Step 4: Dimension & Pixel Grid Check ]
          │
          ▼
[ Step 5: Placement in assets/<category>/ ]
          │
          ▼
[ Step 6: Metadata Authoring (Spritesheet/Animation) ]
          │
          ▼
[ Step 7: Registration in asset_manifest.json ]
          │
          ▼
[ Step 8: AssetValidator Automated Audit ]
```

## 2. Step-by-Step Instructions

1. **Ingest**: Download asset archives strictly into `tools/asset_manager/imports/`.
2. **Verify License**: Record license type, source URL, and attribution rules. If unclear, set `"status": "license_review_required"`.
3. **Format Conversion**: Ensure pixel art is 32-bit PNG with transparency. Ensure audio is 16-bit 44.1 kHz WAV or OGG Vorbis.
4. **Grid Alignment**: Check that sprites conform to 16x16, 32x32, or 48x48 cell grids.
5. **Move to Production**: Relocate into the appropriate `assets/` subfolder (e.g. `assets/characters/player/`).
6. **Register**: Add the asset block to `assets/manifests/asset_manifest.json`.
7. **Validate**: Run `AssetValidator::ValidateAll` or inspect via the Developer Asset Browser (`F1`).

