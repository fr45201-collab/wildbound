# Biome Audio Configuration Specification

## 1. Overview & Data Schema

Wildbound uses data-driven JSON configurations located at `data/audio/biome_audio.json` to define music tracks, ambient loops, and default volume levels across all 6 biomes.

---

## 2. Configuration Schema

```json
{
    "biomes": {
        "<biome_id>": {
            "music": "<music_track_id>",
            "ambient_layers": [
                "<ambient_layer_id_1>",
                "<ambient_layer_id_2>"
            ],
            "default_volume": 0.70
        }
    }
}
```

---

## 3. Active Biome Definitions

### 3.1 Forest (`forest`)
```json
{
    "music": "mus_forest",
    "ambient_layers": [
        "amb_forest_wind",
        "amb_forest_birds",
        "amb_forest_river"
    ],
    "default_volume": 0.70
}
```

### 3.2 Desert (`desert`)
```json
{
    "music": "mus_desert",
    "ambient_layers": [
        "amb_desert_wind"
    ],
    "default_volume": 0.75
}
```

### 3.3 Snow (`snow`)
```json
{
    "music": "mus_snow",
    "ambient_layers": [
        "amb_snow_blizzard"
    ],
    "default_volume": 0.70
}
```

### 3.4 Swamp (`swamp`)
```json
{
    "music": "mus_swamp",
    "ambient_layers": [
        "amb_swamp_insects"
    ],
    "default_volume": 0.75
}
```

### 3.5 Mountain (`mountain`)
```json
{
    "music": "mus_mountain",
    "ambient_layers": [
        "amb_mountain_wind",
        "amb_waterfall_heavy"
    ],
    "default_volume": 0.75
}
```

### 3.6 Village (`village`)
```json
{
    "music": "mus_village",
    "ambient_layers": [
        "amb_village_market",
        "amb_village_blacksmith",
        "amb_indoor_fireplace"
    ],
    "default_volume": 0.70
}
```

---

## 4. Runtime Transition Handling

When the player enters a new biome or crosses map borders:
1. `BiomeManager` calls `AudioSystem::SetBiome(newBiomeId)`.
2. `AudioSystem` queries `m_biomeConfigs[newBiomeId]`.
3. If the active music differs from the new biome's `musicTrackId`, `PlayMusic(cfg.musicTrackId, true, 1.0f)` initiates a 1-second smooth fade-in.
4. The ambient one-shot scheduler resets its countdown timer, switching to the new biome's distinctive environmental sound bank.

