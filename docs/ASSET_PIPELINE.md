# Wildbound Asset Pipeline

## 1. Technical Standards

### Graphical Assets
- **Format**: Lossless PNG with RGBA channels (32-bit).
- **Pixel Grid**: Base grid 16x16 or 32x32 tiles.
- **Filtering**: Strictly Point / Nearest-Neighbor (`TEXTURE_FILTER_POINT`). No bilinear filtering or blurring.
- **Color Palette**: Consistent indexed palette across biomes and characters to preserve visual coherence.
- **Spritesheets**: Uniform cell sizes, JSON/Lua metadata descriptors defining frame rectangles, anchor points (origins), and animation timing.

### Audio Assets
- **Background Music (BGM)**: OGG Vorbis (`.ogg`), 44.1 kHz, stereo, compressed for streaming.
- **Sound Effects (SFX)**: Uncompressed WAV (`.wav`) or OGG, 44.1 kHz, 16-bit, mono or stereo, pre-loaded into memory.

## 2. Directory Hierarchy
- `assets/characters/` - Character base sprites and animations.
- `assets/environment/` - Props, trees, rocks, vegetation, water tiles.
- `assets/tilesets/` - Autotiles, rule tiles, terrain sets.
- `assets/costumes/` - Armor, hats, clothing layers for paperdoll rendering.
- `assets/weapons/` - Handheld gear, bows, swords, staves.
- `assets/items/` - Inventory icons and ground pickup sprites.
- `assets/ui/` - 9-slice frames, buttons, cursors, font atlases.
- `assets/audio/` - Music tracks and SFX clips.
- `assets/manifests/` - Manifest definitions linking assets to engine IDs.

