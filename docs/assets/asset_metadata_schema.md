# Wildbound Asset Metadata Schema Specification (v1.1)

**Document:** Canonical Asset Metadata Schema  
**Target Environment:** Wildbound Asset Management System (AMS)  
**Standard Version:** 1.1.0  
**Status:** APPROVED  
**Date:** 2026-11-08

---

## 1. Scope & Objective

This specification defines the canonical metadata format for all assets managed in the Wildbound game repository (`assets/manifests/asset_manifest.json`). It standardizes naming conventions, data types, mandatory vs. optional fields, and licensing attribution attributes across all asset classes (visual, audio, configuration, and data).

---

## 2. JSON Manifest Structure

The master asset manifest is stored as a UTF-8 JSON file containing a top-level object:

```json
{
  "version": "1.1.0",
  "generated_at": "ISO-8601 UTC timestamp",
  "total_assets": 701,
  "assets": [
    { ... },
    { ... }
  ]
}
```

---

## 3. Asset Record Schema

Each object within the `"assets"` array must conform to the following schema:

| Field Name | Type | Presence | Description | Allowed Values / Examples |
|---|---|---|---|---|
| `id` | `string` | **Mandatory** | Unique string identifier used across the engine and scripts. | `tex_player`, `sfx_chime`, `mus_main_menu`, `tex_storm_overlay` |
| `file` | `string` | **Mandatory** | Canonical relative file path from repository `assets/` root. Must use forward slashes (`/`). | `characters/player/player_spritesheet.png`, `audio/music/mus_main_menu.wav` |
| `type` | `string` | **Mandatory** | Semantic asset classification. | `"texture"`, `"spritesheet"`, `"font"`, `"sound"`, `"music"`, `"data"` |
| `category` | `string` | **Mandatory** | Hierarchical organizational category. | `characters/player`, `environment/trees`, `audio/music`, `audio/sfx/combat` |
| `source` | `string` | **Mandatory** | Provenance or origin of the asset. | `Original`, `Wildbound Audio Lab`, `OpenGameArt.org`, `Game-icons.net` |
| `author` | `string` | **Mandatory** | Creator, artist, or team responsible for the asset. | `Wildbound Art Team`, `Wildbound Sound Architect`, `Kenney`, `Lorc` |
| `license` | `string` | **Mandatory** | Formal legal license applicable to the asset. | `CC0 1.0 Universal`, `CC BY 3.0`, `MIT`, `Proprietary In-House` |
| `commercial_use` | `boolean` | **Mandatory** | Flag indicating whether commercial distribution is permitted. | `true`, `false` |
| `attribution_required` | `boolean` | **Mandatory** | Flag indicating whether author attribution must be displayed in-game. | `true`, `false` |
| `checksum` | `string` | **Mandatory** | Cryptographic hash or build-pipeline checksum for integrity verification. | `f606a7`, `c_sfx_combat_hit_01`, `sha256:...` |
| `width` | `integer` | **Conditional** | Pixel width for visual textures/spritesheets. `0` for audio and data assets. | $\ge 0$ |
| `height` | `integer` | **Conditional** | Pixel height for visual textures/spritesheets. `0` for audio and data assets. | $\ge 0$ |
| `duration` | `number` | **Conditional** | Playback length in seconds for audio clips. `0.0` for visual and data assets. | $\ge 0.0$ |
| `status` | `string` | **Mandatory** | Lifecycle status within the Asset Management System. | `"approved"`, `"license_review_required"`, `"missing"`, `"deprecated"` |
| `metadata` | `object` | **Optional** | Subsystem-specific attributes (e.g. wrap mode, audio bus, loop flags). | `{"wrap": "repeat", "bus": "environment", "loop": true}` |

---

## 4. Field Normalization & Deprecation Rules

To ensure backward compatibility with historical assets while establishing strict forward standards:

1. **`file` vs. `path`**:
   - Canonical: `"file"`
   - Deprecated: `"path"`
   - *Rule*: The C++ parser supports `"path"` as a fallback, logging a diagnostic warning. All serialized manifests and pipeline generators MUST emit `"file"`.

2. **Licensing Keys Case Convention**:
   - Canonical: `"commercial_use"`, `"attribution_required"`
   - Deprecated: `"commercialUse"`, `"attributionRequired"`
   - *Rule*: Snake_case is the canonical convention. CamelCase is accepted by the C++ parser with a warning and automatically normalized upon rewrite.

3. **Audio Type Classification**:
   - Canonical: `"sound"` (for sound effects, ambience, UI audio) or `"music"` (for musical tracks)
   - Deprecated: `"audio"`
   - *Rule*: The generic type `"audio"` is automatically mapped to `AssetType::Sound` in C++ and should be normalized in manifests based on category (`audio/music` $\to$ `"music"`, others $\to$ `"sound"`).

4. **Visual Dimensions**:
   - Canonical: Root-level `"width"` and `"height"`.
   - Deprecated: Storing dimensions solely inside a nested `"metadata"` dictionary without root definitions.

---

## 5. Examples

### 5.1. Texture Asset
```json
{
  "id": "tex_oak_tree",
  "file": "environment/trees/oak_tree.png",
  "type": "texture",
  "category": "environment/trees",
  "source": "Wildbound Environment Artist",
  "author": "Wildbound Art Team",
  "license": "CC0 1.0 Universal",
  "commercial_use": true,
  "attribution_required": false,
  "checksum": "d4e21a8c9b",
  "width": 64,
  "height": 96,
  "duration": 0.0,
  "status": "approved"
}
```

### 5.2. Audio Asset (Sound Effect)
```json
{
  "id": "sfx_storm_damage",
  "file": "audio/sfx/combat/sfx_storm_damage.wav",
  "type": "sound",
  "category": "audio/sfx/combat",
  "source": "Wildbound Audio Lab",
  "author": "Wildbound Sound Architect",
  "license": "CC0 1.0 Universal",
  "commercial_use": true,
  "attribution_required": false,
  "checksum": "c_sfx_storm_damage",
  "width": 0,
  "height": 0,
  "duration": 0.85,
  "status": "approved",
  "metadata": {
    "format": "wav",
    "bus": "sfx",
    "loop": false
  }
}
```

### 5.3. External Licensed Icon
```json
{
  "id": "icon_rune_64",
  "file": "processed/ui/icons/icon_rune_64.png",
  "type": "texture",
  "category": "ui/icons",
  "source": "Game-icons.net",
  "author": "Lorc",
  "license": "CC BY 3.0",
  "commercial_use": true,
  "attribution_required": true,
  "checksum": "e7b1a2",
  "width": 64,
  "height": 64,
  "duration": 0.0,
  "status": "approved"
}
```

