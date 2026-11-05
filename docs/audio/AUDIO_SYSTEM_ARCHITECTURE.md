# Audio System Architecture Specification

## 1. Architectural Overview

Wildbound features a centralized, event-driven, 8-bus audio subsystem built upon C++20 and raylib 5.5 (`raudio` / miniaudio WASAPI backend).

The architecture strictly separates:
1. **Audio Presentation / Engine Subsystem** (`wildbound::audio::AudioSystem`): Manages device hardware, bus mixing hierarchies, stream caching, spatial math, and playback.
2. **Procedural Sound Asset Pipeline** (`tools/audio_generation/`): Modular Python synthesis generators producing 100% original, deterministic 16-bit PCM WAV assets without third-party dependencies.
3. **Gameplay Event Dispatching**: Decoupled event invocation (`player.footstep.*`, `weapon.*`, `combat.*`, `creature.*`, `ui.*`, `environment.*`, `match.*`).

```
+---------------------------------------------------------------------------------+
|                                 Gameplay Systems                                |
|  (Player Entity, CombatSystem, WeaponController, UI, InteractionManager, Enemy) |
+---------------------------------------------------------------------------------+
         |                                                       |
         | DispatchEvent("player.footstep.grass")               | PlaySoundSpatial(...)
         v                                                       v
+---------------------------------------------------------------------------------+
|                            wildbound::audio::AudioSystem                        |
|                                                                                 |
|  [Event Registry & Cooldown Throttler]                                         |
|    - String Normalization ("Player.Footstep.Grass" -> "player.footstep.grass")  |
|    - Multi-Sound Variation Shuffling (_001, _002, _003)                        |
|    - Event Cooldown Clamping (prevents rapid audio stacking)                    |
|                                                                                 |
|  [Spatial 2D Audio Processor]                                                   |
|    - Quadratic Distance Attenuation: (1 - d / d_max)^2                          |
|    - Stereo Panning Engine: dx / 400.0 mapped to [0.0..1.0]                     |
|                                                                                 |
|  [8-Bus Mixing Matrix]                                                          |
|    - Master Bus: [0.0..1.0] (Global Attenuation & Mute)                        |
|    - Channel Sub-buses: Music, UI, SFX, Environment, Combat, Creatures, Voice  |
|                                                                                 |
|  [Stream & Voice Cache]                                                         |
|    - raylib ::Music (Streaming 44.1 kHz Stereo)                                 |
|    - raylib ::Sound (Buffered 16-bit PCM Mono / Stereo)                         |
+---------------------------------------------------------------------------------+
         |
         v
+---------------------------------------------------------------------------------+
|                         raylib / miniaudio Hardware Device                      |
|                         (WASAPI / DirectSound / ALSA / Pulse)                   |
+---------------------------------------------------------------------------------+
```

---

## 2. Audio Buses Hierarchy

Wildbound implements 8 dedicated mixing buses:

| Bus Name | Enum Value | Intended Audio Assets | Default Volume |
|---|---|---|---|
| **Master** | `AudioBus::Master` | Master output multiplier, mute-all override | `0.80` |
| **Music** | `AudioBus::Music` | Ambient biome music tracks, battle music, stingers | `0.70` |
| **UI** | `AudioBus::UI` | Button clicks, tab switches, dialogs, item movements | `0.80` |
| **SFX** | `AudioBus::SFX` | Player footsteps, door interactions, coin/gem pickups | `0.80` |
| **Environment**| `AudioBus::Environment` | Wind, rivers, waterfalls, campfire, biome loops | `0.70` |
| **Combat** | `AudioBus::Combat` | Flesh impacts, critical hit chimes, enemy hurt sounds | `0.80` |
| **Creatures** | `AudioBus::Creatures` | Goblin, skeleton, wolf, boar, bat vocalizations | `0.75` |
| **Voice** | `AudioBus::Voice` | Reserved for future voice communication / dialogue | `0.80` |

### Effective Volume Formulation
The final output volume $V_{effective}$ sent to the hardware device for any asset on bus $B$ is:

$$V_{effective} = \begin{cases} 0.0 & \text{if } \text{muteAll} == \text{true} \\ V_{master} \times V_{bus} \times V_{asset} \times V_{scale} \times A_{spatial} & \text{otherwise} \end{cases}$$

where:
- $V_{master} \in [0.0..1.0]$ is master attenuation.
- $V_{bus} \in [0.0..1.0]$ is the specific bus gain.
- $V_{asset}$ is the catalog default volume.
- $V_{scale}$ is caller-supplied multiplier (e.g. footstep volume).
- $A_{spatial} \in [0.0..1.0]$ is the quadratic distance attenuation factor.

---

## 3. Spatial Audio Mathematics

Sound events in world space (e.g., enemy attacks, weapon impacts, waterfalls, and footsteps) are positioned relative to the listener (the player character):

$$\Delta x = x_{source} - x_{listener}, \quad \Delta y = y_{source} - y_{listener}$$
$$d = \sqrt{\Delta x^2 + \Delta y^2}$$

### Distance Attenuation
If $d \ge d_{max}$, the voice is immediately culled to save CPU and audio buffer overhead. Otherwise:

$$A_{spatial} = \left( 1.0 - \frac{d}{d_{max}} \right)^2$$

### Stereo Panning
Horizontal displacement $\Delta x$ is mapped to stereo panning using a reference panning horizon $D_{pan} = 400.0\text{ units}$:

$$p_{norm} = \text{clamp}\left( \frac{\Delta x}{D_{pan}}, -1.0, 1.0 \right)$$
$$P_{raylib} = 0.5 + 0.5 \times p_{norm}$$

where $P_{raylib} = 0.0$ is full left, $0.5$ is dead center, and $1.0$ is full right. This maps directly to raylib's `SetSoundPan(sound, pan)` API.

---

## 4. Multi-Sound Variations and Cooldown Throttling

To prevent acoustic fatigue and repetitive machine-gun audio effects:
1. **Variation Cycling:** Events such as `player.footstep.grass` contain 3 distinct synthesized variations (`_001`, `_002`, `_003`). Each trigger cycles sequentially through the variations.
2. **Cooldown Clamping:** Each event has a minimum re-trigger cooldown (e.g., $0.15\text{s}$ for footsteps, $0.25\text{s}$ for hurt vocalizations). Dispatches occurring within the cooldown window are dropped silently without queuing.
