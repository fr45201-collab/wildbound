# Audio Asset Metadata Specification

## 1. Metadata Schema

Every procedural and integrated audio file in Wildbound is tracked via comprehensive JSON metadata in both `data/audio/audio_catalog.json` and the Asset Management System (AMS) manifest (`assets/manifests/asset_manifest.json`).

### Master Catalog Schema
```json
{
    "id": "player.footstep.grass",
    "title": "Footstep Grass 1",
    "category": "player",
    "file": "audio/sfx/player/sfx_footstep_grass_001.wav",
    "duration": 0.13,
    "sample_rate": 44100,
    "channels": 1,
    "bit_depth": 16,
    "looping": false,
    "bus": "sfx",
    "default_volume": 0.55,
    "description": "Player footstep cadence on grass variation 1",
    "seed": 3001
}
```

### AMS Manifest Schema
```json
{
    "id": "sfx_footstep_grass_001",
    "file": "audio/sfx/player/sfx_footstep_grass_001.wav",
    "type": "audio",
    "category": "audio/player",
    "source": "Wildbound Procedural Audio Generator",
    "author": "Wildbound Audio Team",
    "license": "CC0 1.0 Universal",
    "commercial_use": true,
    "attribution_required": false,
    "checksum": "c_sfx_footstep_grass_001_audio",
    "duration": 0.13,
    "status": "approved"
}
```

---

## 2. Field Specifications

| Field | Type | Validation Constraint | Description |
|---|---|---|---|
| `id` | `string` | Unique, lowercase snake_case | Stable programmatic identifier |
| `title` | `string` | Non-empty human readable | Display name in developer tools |
| `category` | `string` | Valid category enum | Category grouping (`player`, `weapons`, `combat`, `creatures`, `ui`, `ambience`, `music`, `interactions`, `events`) |
| `file` | `string` | Valid relative path | Path relative to `assets/` root |
| `duration` | `float` | $> 0.0$ | Exact playback length in seconds |
| `sample_rate`| `integer`| Exactly `44100` | Target PCM sample rate |
| `channels` | `integer`| `1` (Mono) or `2` (Stereo) | Channel count |
| `bit_depth` | `integer`| Exactly `16` | Signed 16-bit linear PCM (`int16_t`) |
| `looping` | `boolean`| `true` for music/ambience | Seamless loop flag |
| `bus` | `string` | One of the 8 valid buses | Routing destination in `AudioSystem` |
| `default_volume`| `float` | $[0.0..1.0]$ | Baseline volume attenuation |
| `seed` | `integer`| $> 0$ | Deterministic PRNG seed for synthesis reproducibility |

