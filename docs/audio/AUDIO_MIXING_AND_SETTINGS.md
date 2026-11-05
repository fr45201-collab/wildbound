# WILDBOUND — AUDIO MIXING & SETTINGS SPECIFICATION

## 1. Logical Bus Hierarchy

Wildbound routes all audio through 6 distinct buses to give players and systems fine-grained mixing control:

1. **Master Bus**: Global attenuation factor $[0.0, 1.0]$. Directly scales all downstream buses.
2. **Music Bus**: Procedural background music streams (`mus_*`). Defaults to $0.70$.
3. **SFX Bus**: Gameplay action sounds: footsteps, weapon swings, combat impacts, interactions. Defaults to $0.85$.
4. **UI Bus**: Interface buttons, tab switches, dialog opening/closing, notifications. Defaults to $0.80$.
5. **Environment Bus**: Ambient biome loops (birds, wind, blizzards, crickets, fire crackle). Defaults to $0.65$.
6. **Voice Bus**: Character vocalizations and future voice lines. Defaults to $1.00$.

---

## 2. Dynamic Volume Calculation

The effective volume delivered to raylib's audio device backend is:

$$\text{EffectiveVolume}(\text{bus}) = \begin{cases} 0.0 & \text{if } \text{IsMuteAll()} == \text{true} \\ \text{Clamped}(\text{MasterVolume} \times \text{BusVolume}(\text{bus}), 0.0, 1.0) & \text{otherwise} \end{cases}$$

Whenever a bus volume changes or `SetMuteAll` is toggled, `AudioSystem::ApplyBusVolumes()` iterates over all currently playing streams (music and ambient loops) and immediately updates raylib's stream gain via `SetMusicVolume(stream, effectiveVolume)`.

---

## 3. JSON Persistence Schema

Audio settings are persisted to `data/config/audio_settings.json`.

```json
{
  "version": 1,
  "master_volume": 0.85,
  "mute_all": false,
  "buses": {
    "music": 0.70,
    "ui": 0.80,
    "sfx": 0.85,
    "environment": 0.65,
    "voice": 1.00
  }
}
```

The system automatically falls back to balanced defaults if the configuration file is missing, and writes atomically to prevent corruption.

