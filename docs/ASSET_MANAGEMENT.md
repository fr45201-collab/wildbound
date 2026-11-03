# Wildbound Asset Management System (AMS)

## 1. Architectural Overview
The Wildbound Asset Management System (AMS) is a centralized, modular subsystem located in `src/resources/`. It provides single-point resource acquisition, deduplicated in-memory caching, validation, and lifecycle management.

```
                           +------------------------+
                           |  asset_manifest.json   |
                           +------------------------+
                                       |
                                       v
                           +------------------------+
                           |     AssetRegistry      |
                           |  (Catalog & Metadata)  |
                           +------------------------+
                                       |
                                       v
                           +------------------------+
                           |      AssetManager      |
                           |   (Central Facade)     |
                           +------------------------+
                           /           |            \
                          v            v             v
                 +---------------+ +---------------+ +---------------+
                 |TextureManager | |  FontManager  | | AudioManager  |
                 | - Deduplication| | - TTF / OTF   | | - WAV / OGG   |
                 | - Point filter| | - Raylib Font | | - Audio Dev   |
                 +---------------+ +---------------+ +---------------+
```

## 2. Core Subsystems

### `AssetRegistry` (`AssetRegistry.hpp`)
- In-memory database of registered assets.
- Loaded from `assets/manifests/asset_manifest.json`.
- Indexed by unique string IDs (e.g., `tex_player`, `tex_oak_tree`).
- Supports querying by category, type, and status.

### `TextureManager` (`TextureManager.hpp`)
- Thread-safe texture cache.
- Enforces strict nearest-neighbor point filtering (`TEXTURE_FILTER_POINT`) for all loaded pixel art.
- Fallback checkerboard texture returned when an asset file is missing or corrupted.
- Tracks estimated VRAM/RAM footprint (`width * height * 4` bytes).

### `SpriteSheet` (`SpriteSheet.hpp`)
- Supports grid-based and variable-sized spritesheets.
- Calculates source rectangle given frame index $(col, row)$, accounting for margins and spacing.
- Supports origin offsets (`top_left`, `center`, `bottom_center`, `custom`).

### `AnimationMetadata` (`AnimationMetadata.hpp`)
- Defines frame sequences, duration per frame, and looping flags.
- Computes the active frame index given elapsed time using modulo/clamping.

### `AssetValidator` (`AssetValidator.hpp`)
- Audits registered manifest entries against disk contents.
- Differentiates warnings (unreviewed licenses, non-standard extensions) from errors (missing files, malformed metadata).

