# Phase 28 — Audio System & Sound Integration: Repository Audit

**Audit Date:** November 5, 2026  
**Auditor:** Wildbound Systems Engineering  
**Scope:** Complete inspection of C++ audio codebase, raylib integration, data catalogues, gameplay hooks, asset manifest, and Python procedural generation tools.

---

## 1. Existing Audio Infrastructure

The repository currently features a foundational audio system introduced in Phase 18:
- **Core Engine Binding:** Raylib `raudio` C-API (`InitAudioDevice`, `IsAudioDeviceReady`, `CloseAudioDevice`, `LoadSound`, `PlaySound`, `SetSoundVolume`, `SetSoundPitch`, `SetSoundPan`, `LoadMusicStream`, `PlayMusicStream`, `UpdateMusicStream`, `SetMusicVolume`, `StopMusicStream`).
- **Core Classes:**
  - `wildbound::audio::AudioSystem` (`src/audio/AudioSystem.hpp`, `src/audio/AudioSystem.cpp`): Implements audio lifecycle, resource caching, bus attenuation, event dispatching, and biome setting.
  - `wildbound::audio::AudioSettings` (`src/audio/AudioData.hpp`): Struct encapsulating bus volumes, mute state, and JSON serialization.
  - `wildbound::audio::AudioAssetInfo` (`src/audio/AudioData.hpp`): Metadata container for sample rate, channels, bit depth, default volume, bus association, and duration.
- **Developer Tools:**
  - `DevUI` Tab 12 ("Audio & Sound") in `src/ui/DevUI.cpp`: Controls for master/bus sliders, mute toggle, music track playback, biome switching, test event dispatch, and sound effect auditioning.
- **Build & Tests:**
  - CMake registers `test_audio.cpp` as Test 18 (`AudioSystemTest`), validating catalog loading, bus volume calculations, and setting persistence.

---

## 2. Existing AudioManager Capabilities

`AudioSystem` currently provides:
1. **Device Management:** Initializes raylib audio hardware safely; falls back to silent operation if an audio device is unavailable without crashing.
2. **Bus Attenuation:** 6 buses (`Master`, `Music`, `UI`, `SFX`, `Environment`, `Voice`) with individual volume multipliers multiplied by master volume.
3. **Music Streaming:** Single active music stream with linear fade-in and volume adjustments.
4. **Sound Effect Caching:** Lazy loading of sound effects by asset ID into a `std::unordered_map<std::string, ::Sound>`.
5. **Event-Driven Dispatch:** Simple string-to-asset mapping table (`m_eventMap["UI.ButtonClick"] = "sfx_ui_click"`).

### Deficiencies in Current `AudioSystem`:
- **Audio Buses:** Only 6 buses exist. Phase 28 requires 8 dedicated buses: `Master`, `Music`, `UI`, `SFX`, `Environment`, `Combat`, `Creatures`, `Voice`.
- **Spatial Audio:** Completely absent. No distance attenuation, no stereo panning, no world-space listener positioning.
- **Footstep Handling:** No surface detection or animation footstep synchronization in player code.
- **Event Throttling / Cooldowns:** None. Rapid damage ticks or repeated button clicks can cause sound stacking and audio buffer saturation.
- **Creature Audio:** Zero creature audio event mappings exist in `m_eventMap`.
- **Music Transitions:** Only supports a single fading stream; no crossfading or clean transition between biome exploration and combat states.

---

## 3. Existing Sound Asset Catalogue

The repository currently contains **68 procedural WAV audio files** generated via Python (`tools/audio_generation/generate_all.py`), all 16-bit PCM, 44.1 kHz:
- **Music (10 tracks, Stereo):**
  - `mus_main_menu`, `mus_lobby`, `mus_forest`, `mus_desert`, `mus_snow`, `mus_swamp`, `mus_mountain`, `mus_combat`, `mus_victory`, `mus_defeat`.
- **UI (15 sounds, Mono):**
  - `sfx_ui_hover`, `sfx_ui_click`, `sfx_ui_confirm`, `sfx_ui_cancel`, `sfx_ui_tab`, `sfx_ui_panel_open`, `sfx_ui_panel_close`, `sfx_ui_item_select`, `sfx_ui_cosmetic_apply`, `sfx_ui_cosmetic_remove`, `sfx_ui_settings_adjust`, `sfx_ui_error`, `sfx_ui_success`, `sfx_ui_back`, `sfx_ui_modal_confirm`.
- **Player (14 sounds, Mono):**
  - Footsteps: `sfx_footstep_grass_001` & `002`, `sfx_footstep_dirt_001` & `002`, `sfx_footstep_stone_001` & `002`, `sfx_footstep_snow_001` & `002`, `sfx_footstep_sand_001` & `002`, `sfx_footstep_mud_001` & `002`.
  - Player Hurt & Death: `sfx_player_hurt`, `sfx_player_death`.
- **Weapons (9 sounds, Mono):**
  - Swords: `sfx_sword_swing_001` & `002`, `sfx_sword_impact_metal`, `sfx_sword_impact_wood`, `sfx_sword_sheathe`.
  - Bows: `sfx_bow_draw`, `sfx_bow_release`, `sfx_arrow_flight`, `sfx_arrow_impact`.
- **Interactions (8 sounds, Mono):**
  - `sfx_item_pickup`, `sfx_item_drop`, `sfx_coin_pickup`, `sfx_gem_pickup`, `sfx_potion_drink`, `sfx_door_open`, `sfx_door_close`, `sfx_chest_open`.
- **Ambience (7 loops, Stereo):**
  - `amb_forest_wind`, `amb_forest_birds`, `amb_desert_wind`, `amb_snow_blizzard`, `amb_swamp_insects`, `amb_mountain_wind`, `amb_indoor_fireplace`.
- **Match Events (5 sounds, Mono):**
  - `sfx_match_countdown`, `sfx_match_start`, `sfx_gameplay_alert`, `sfx_stinger_victory`, `sfx_stinger_defeat`.

---

## 4. Missing Audio Assets

Based on the Phase 28 requirements, the following sound assets are missing:

1. **Footsteps:**
   - Wood surface footsteps (`sfx_footstep_wood_001`, `002`, `003`).
   - Shallow water footsteps (`sfx_footstep_water_001`, `002`, `003`).
   - Third variation (`_003`) for grass, dirt, stone, snow, sand, and mud to prevent repetitive cadence.
2. **Weapons & Combat:**
   - Bow equip / draw holster (`sfx_bow_equip`).
   - Arrow impact stone (`sfx_arrow_impact_stone`).
   - Arrow impact wood (`sfx_arrow_impact_wood`).
   - Arrow impact enemy flesh (`sfx_arrow_impact_flesh`).
   - Shield block / parry (`sfx_shield_block`, `sfx_parry_clink`).
   - Critical hit stinger (`sfx_combat_critical_hit`).
   - General combat impact variations (`sfx_impact_flesh_001`, `002`).
3. **Creature Audio (5 Archetypes + Wildlife):**
   - **Goblin:** idle grunt (`sfx_creature_goblin_idle`), movement shuffle (`sfx_creature_goblin_step`), club swing attack (`sfx_creature_goblin_attack`), hurt grunt (`sfx_creature_goblin_hurt`), death screech (`sfx_creature_goblin_death`).
   - **Skeleton:** bone rattle movement (`sfx_creature_skeleton_step`), bone clatter attack (`sfx_creature_skeleton_attack`), bone crack hurt (`sfx_creature_skeleton_hurt`), collapse death (`sfx_creature_skeleton_death`).
   - **Wolf:** idle snarl (`sfx_creature_wolf_idle`), aggressive growl (`sfx_creature_wolf_growl`), bite attack (`sfx_creature_wolf_attack`), yelp hurt (`sfx_creature_wolf_hurt`), death howl/whimper (`sfx_creature_wolf_death`).
   - **Wild Boar:** snorting grunt (`sfx_creature_boar_idle`), hooves trotting (`sfx_creature_boar_step`), gore/charge attack (`sfx_creature_boar_attack`), squeal hurt (`sfx_creature_boar_hurt`), death squeal (`sfx_creature_boar_death`).
   - **Bat:** wing flutter flight (`sfx_creature_bat_flight`), high screech attack (`sfx_creature_bat_attack`), hurt screech (`sfx_creature_bat_hurt`), death thump (`sfx_creature_bat_death`).
   - **Wildlife:** bird wing takeoff (`sfx_wildlife_bird_takeoff`), bird landing flutter (`sfx_wildlife_bird_landing`), subtle insect/butterfly flutter (`sfx_wildlife_butterfly_flutter`).
4. **Environmental & Biome Sounds:**
   - River stream flow (`amb_forest_river`, loop).
   - Waterfall roar (`amb_waterfall_heavy`, loop).
   - Desert sand drift (`amb_desert_sand_shift`).
   - Cold ice cracking (`sfx_env_ice_crack`).
   - Swamp mud bubble pop (`sfx_env_swamp_bubble`).
   - Mountain rockfall echo (`sfx_env_mountain_rockfall`).
   - Village market / town tone (`amb_village_market`, loop).
   - Blacksmith forge hammer anvil (`amb_village_blacksmith`, loop / periodic).
5. **Music:**
   - Village exploration theme (`mus_village`, stereo, loopable).

---

## 5. Missing Audio Events

The current `m_eventMap` misses standard IDs requested by Phase 28:
- `player.footstep.wood`, `player.footstep.water`
- `weapon.sword.swing`, `weapon.sword.impact`, `weapon.bow.draw`, `weapon.bow.release`, `weapon.arrow.flight`, `weapon.arrow.impact`, `weapon.arrow.impact_flesh`, `weapon.arrow.impact_stone`, `weapon.arrow.impact_wood`, `weapon.shield.block`
- `combat.player.hurt`, `combat.player.death`, `combat.enemy.hurt`, `combat.enemy.death`, `combat.impact`, `combat.critical_hit`
- `creature.goblin.*`, `creature.skeleton.*`, `creature.wolf.*`, `creature.wild_boar.*`, `creature.bat.*`
- `environment.forest.river`, `environment.forest.waterfall`, `environment.village.forge`, `environment.ice.crack`, `environment.swamp.bubble`
- `music.village`

---

## 6. Missing System Functionality

1. **Footstep System:**
   - Needs surface detection using tile GID from `Tilemap` at player's foot coordinates $(x, y)$.
   - Cadence timer tied to velocity ($V > 1.0\text{ px/s}$): Walk step interval $\sim 0.38\text{s}$, Run step interval $\sim 0.24\text{s}$.
   - No audio when idle or in air/roll.
2. **Spatial Audio Manager:**
   - Positional playback for world events (`PlaySoundSpatial`).
   - Formula:
     $$\text{Volume} = \text{BaseVol} \cdot \left(1.0 - \frac{d}{d_{max}}\right)^2$$
     $$\text{Pan} = \text{clamp}\left(\frac{x_{source} - x_{listener}}{d_{pan}}, -1.0, 1.0\right)$$
   - Uses `SetSoundVolume` and `SetSoundPan`.
3. **Ambient Audio Manager:**
   - Periodic random one-shot ambience (e.g. distant bird chirp every 4-10s, cricket burst, wind howl).
   - Simultaneous layer mixing.
4. **Bus Architecture:**
   - Must extend `AudioBus` to 8 buses: `Master`, `Music`, `UI`, `SFX`, `Environment`, `Combat`, `Creatures`, `Voice`.
   - Update settings JSON schema and UI sliders.
5. **Cooldown & Voice Throttling:**
   - Prevents stacking of identical sounds within $< 0.1\text{s}$ (or $< 0.3\text{s}$ for hurt screams).

---

## 7. Integration Gaps

- **Player Movement:** `Player::UpdateWithWorld` does not currently invoke any footstep audio.
- **Weapon System:** `WeaponController` does not dispatch attack swings or bow charge/release sounds.
- **Combat System:** `CombatSystem::ProcessAttack` and `DamageSystem::ApplyDamage` dispatch data events to `CombatEventDispatcher`, but `AudioSystem` is not subscribed to these events.
- **Creature AI:** `Enemy::UpdateWithWorld` updates state machines, but creature vocalizations and attacks are never routed to audio.
- **Biome Management:** `Game.cpp` initializes audio to "forest", but moving between biomes in `BiomeManager` does not trigger music crossfades or update active ambience layers.

---

## 8. Performance & Resource Considerations

- Raylib `Sound` structures hold raw uncompressed PCM in memory. With 68 existing + $\sim 45$ new assets, total RAM footprint is $< 35\text{ MB}$.
- Music and environmental ambience loops must use streaming (`::Music` / `LoadMusicStream`) rather than fully buffered `Sound` to minimize memory pressure.
- Caching already exists via `m_loadedSounds` and `m_loadedMusic`. We must ensure clean unloading in `Shutdown()`.

---

## 9. Licensing and Provenance

- All 68 existing audio assets were synthesized using `tools/audio_generation/generate_all.py` with custom NumPy / mathematical formulations (sin, triangle, square, sawtooth, filtered noise, ADSR).
- All new Phase 28 assets will be generated strictly via the Python procedural synthesis suite with recorded deterministic seeds and registered under `CC0 1.0 Universal` in `asset_manifest.json`.
- Zero third-party, commercial, or copyrighted assets are present or will be introduced.

---

## 10. Recommended Implementation Sequence

1. **Python Procedural Generation Suite:**
   - Expand `tools/audio_generation/` with modular generators (`footsteps.py`, `weapons.py`, `combat.py`, `creatures.py`, `environmental.py`, `music.py`).
   - Generate all missing sound assets (footsteps variations, weapon impacts, creature vocalizations, environmental loops, village music).
   - Validate integrity, sample rates, channels, RMS, and loop points with `validate_audio.py`.
   - Update `data/audio/audio_catalog.json` and `data/audio/biome_audio.json`.
2. **Audio System Architecture Extensions:**
   - Expand `AudioBus` to 8 buses (`Master`, `Music`, `UI`, `SFX`, `Environment`, `Combat`, `Creatures`, `Voice`).
   - Implement `SpatialAudioManager` for 2D distance attenuation and stereo panning.
   - Implement `AmbientAudioManager` for randomized periodic environmental layers.
   - Implement event cooldown throttling in `AudioSystem`.
3. **Gameplay Audio Integrations:**
   - Integrate Footstep Surface Detection in `Player::UpdateWithWorld`.
   - Integrate Weapon Swing / Bow Draw in `WeaponController` & `Player`.
   - Subscribe `AudioSystem` to `CombatEventDispatcher` in `CombatSystem` for player/enemy damage and deaths.
   - Integrate creature vocalizations in `Enemy` state machine and combat.
   - Link `BiomeManager` to `AudioSystem::SetBiome` for map/biome transitions.
4. **Developer Tools & Diagnostics:**
   - Extend `DevUI` Tab 12 to expose all 8 buses, creature audio triggers, spatial audio tester, and active voice counts.
5. **Testing & Verification:**
   - Extend `tests/audio/test_audio.cpp` to verify new buses, spatial attenuation math, surface footstep mapping, and creature event lookups.
   - Run CTest across all test suites in Debug and Release.
   - Verify engine smoke-test `--test-run`.
6. **Documentation:**
   - Author the remaining 16 comprehensive markdown specification manuals in `docs/audio/`.

