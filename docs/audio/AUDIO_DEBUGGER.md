# Wildbound Audio Debugger Specification

## Overview

The Audio Debugger is integrated into the Developer User Interface (`DevUI`) under **Tab 12: "Audio & Sound"**. It provides real-time monitoring, diagnostic telemetry, interactive audition controls, and testing facilities for all Wildbound audio subsystems.

The debugger operates during active gameplay and editor/test modes, exposing controls for mixing buses, biome ambience, spatial attenuation, procedural footsteps, combat soundboards, and raw asset auditioning.

---

## Architecture & Integration

- **Source Location**: `src/ui/DevUI.cpp` (`DevUI::DrawAudioTab`)
- **Backend Subsystem**: `wildbound::audio::AudioSystem`
- **Key Access**: F1 toggles DevUI; Tab 12 hosts the Audio Debugger interface.
- **Dependencies**: Dear ImGui (strictly restricted to developer UI).

```
+-------------------------------------------------------------+
| DevUI (F1) -> Tab 12: Audio & Sound                         |
+-------------------------------------------------------------+
|  1. Global Master & Bus Mixers (8 Buses with Volume Sliders) |
|  2. Active Biome Ambience Switcher & Dynamic Track Swapping |
|  3. Spatial Audio 2D Attenuation & Stereo Panning Tester     |
|  4. Creature Soundboard (Goblin, Skeleton, Wolf, Boar, Bat) |
|  5. Footstep & Surface Emulation Sandbox                     |
|  6. Raw SFX Catalog Audition List                            |
+-------------------------------------------------------------+
```

---

## Debugger Features and Controls

### 1. 8-Bus Mixing Console

Allows real-time balancing of the entire game mix across all 8 independent buses:
- **Master**: Overall project volume multiplier $[0.0, 1.0]$.
- **Music**: Background ambience and battle themes $[0.0, 1.0]$.
- **UI**: Menu clicks, tabs, button hovers, inventory transactions $[0.0, 1.0]$.
- **SFX**: General sound effects, interactables, explosions $[0.0, 1.0]$.
- **Environment**: Biome loops, river, waterfall, weather $[0.0, 1.0]$.
- **Combat**: Weapons, impacts, hit markers, critical stingers $[0.0, 1.0]$.
- **Creatures**: Monster roars, groans, alerts, attacks, deaths $[0.0, 1.0]$.
- **Voice**: Character vocalizations and callouts $[0.0, 1.0]$.

**Global Controls**:
- **Mute Toggle**: Instantly silences all game audio without resetting individual slider volumes.
- **Persist Settings**: Serializes volume configurations to `data/audio/audio_settings.json`.

---

### 2. Biome Ambience & Music Switcher

Provides manual override of the game's active biome context to test environmental loops and music track transitions.
- **Available Biomes**:
  - `Forest`: `mus_forest` + `env_forest_loop` (birds, rustling wind).
  - `Desert`: `mus_desert` + `env_desert_loop` (howling dunes, sand shift).
  - `Snow`: `mus_snow` + `env_snow_loop` (blizzard gust, ice cracking).
  - `Swamp`: `mus_swamp` + `env_swamp_loop` (murky bubbling, nocturnal frogs).
  - `Mountain`: `mus_mountain` + `env_mountain_loop` (echoing ridge wind, falling rocks).
  - `Village`: `mus_village` + `env_village_market` / `env_village_blacksmith`.
- **Dynamic Transition**: Cross-fades or smoothly transitions active music tracks and environment layers.

---

### 3. Spatial Audio 2D Tester

Simulates an in-world audio emitter relative to the player listener to verify stereo panning and quadratic distance attenuation.
- **Listener Coordinates**: Automatically synced to the active `Player` entity position in world space.
- **Simulated Emitter Position**:
  - Horizontal offset slider: $\Delta X \in [-1000, +1000]$ pixels.
  - Vertical offset slider: $\Delta Y \in [-1000, +1000]$ pixels.
- **Diagnostics Displayed**:
  - Calculated Euclidean Distance ($d = \sqrt{\Delta x^2 + \Delta y^2}$).
  - Volume attenuation factor: $(1.0 - d / d_{\max})^2$.
  - Stereo pan value: $[0.0 \text{ (Full Left)}, 0.5 \text{ (Center)}, 1.0 \text{ (Full Right)}]$.
- **Emitter Triggers**: Test triggers for `"combat.arrow.flyby"`, `"creature.wolf.howl"`, `"creature.goblin.groan"`.

---

### 4. Creature Soundboard

Direct trigger buttons for each creature archetype to evaluate vocalization frequency, priority, and variation:
- **Goblin**: Alert (`creature.goblin.alert`), Attack (`creature.goblin.attack`), Hurt (`creature.goblin.hurt`), Death (`creature.goblin.death`), Idle (`creature.goblin.groan`).
- **Skeleton**: Alert (`creature.skeleton.alert`), Attack (`creature.skeleton.attack`), Hurt (`creature.skeleton.hurt`), Death (`creature.skeleton.death`), Step (`creature.skeleton.step`).
- **Wolf**: Alert (`creature.wolf.alert`), Attack (`creature.wolf.attack`), Hurt (`creature.wolf.hurt`), Death (`creature.wolf.death`), Howl (`creature.wolf.howl`).
- **Boar**: Alert (`creature.boar.alert`), Attack (`creature.boar.attack`), Hurt (`creature.boar.hurt`), Death (`creature.boar.death`), Snort (`creature.boar.snort`).
- **Bat**: Flutter (`creature.bat.flutter`), Screech (`creature.bat.screech`).

---

### 5. Footstep & Surface Emulation

Allows triggering surface footstep sound events directly to inspect variation cycling and pitch shifting:
- Surfaces: `Grass`, `Dirt`, `Stone`, `Snow`, `Sand`, `Mud`, `Wood`, `Water`.
- Inspects variation alternation across `_01`, `_02`, `_03` variants to verify natural non-repetitive playback.

---

### 6. SFX Catalog Audition List

A searchable and filterable list of all 128 registered audio assets loaded from `data/audio/audio_catalog.json`.
- Displays asset ID, bus assignment, relative path, duration, and channel count.
- Dedicated "Play" button next to each asset entry for instant auditioning.

