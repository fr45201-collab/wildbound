# WILDBOUND — AUDIO ASSET CATALOG

This catalog indexes all 68 original procedural audio assets synthesized in Phase 18. All files are 44.1 kHz, 16-bit PCM WAV.

---

## 1. Original Procedural Music Tracks (10 Tracks)
Stored in `assets/audio/music/`.

| Asset ID | Title | File | Dur (s) | Format | Mood & Context |
|---|---|---|---|---|---|
| `mus_main_menu` | Main Menu Theme | `mus_main_menu.wav` | 10.0 | Stereo | Majestic fantasy intro, celestial arpeggios, warm chordal drone |
| `mus_char_select` | Character Selection | `mus_char_select.wav` | 8.0 | Stereo | Heroic anticipation, marching bassline, rhythmic harp staccato |
| `mus_wardrobe` | Armory & Wardrobe | `mus_wardrobe.wav` | 8.0 | Stereo | Gentle crafting theme, metallic chime accents, light plucked strings |
| `mus_inventory` | Inventory & Loot | `mus_inventory.wav` | 6.0 | Stereo | Mystical contemplation, crystalline harmonics |
| `mus_lobby` | Battle Lobby | `mus_lobby.wav` | 12.0 | Stereo | Steady marching pulse, tense string swells, low percussion |
| `mus_match_results` | Match Results | `mus_match_results.wav` | 8.0 | Stereo | Uplifting brass-like fanfare, victorious ascending arpeggios |
| `mus_game_over` | Defeat / Game Over | `mus_game_over.wav` | 6.0 | Stereo | Somber minor descent, fading twilight bell resonance |
| `mus_forest` | Forest Exploration | `mus_forest.wav` | 12.0 | Stereo | Acoustic wooden timbre, gentle woodland pulse, verdant atmosphere |
| `mus_desert` | Desert Dunes | `mus_desert.wav` | 10.0 | Stereo | Phrygian scale, shimmering heat shimmer overtones |
| `mus_snow` | Frozen Tundra | `mus_snow.wav` | 10.0 | Stereo | Glacial crystalline pads, high-frequency bell reverberations |

---

## 2. Environmental Ambience Loops (7 Tracks)
Stored in `assets/audio/ambience/`.

| Asset ID | Title | File | Dur (s) | Bus | Synthesis Details |
|---|---|---|---|---|---|
| `amb_forest_birds` | Forest Birdsong | `amb_forest_birds.wav` | 6.0 | Env | Modulated sine chirps with gentle filtered pink noise wind |
| `amb_forest_wind` | Forest Canopy Wind | `amb_forest_wind.wav` | 8.0 | Env | Resonant low-pass noise sweeping gently between 180 Hz - 700 Hz |
| `amb_desert_wind` | Desert Whispering Wind | `amb_desert_wind.wav` | 8.0 | Env | Granular filtered noise simulating blowing arid sand grains |
| `amb_snow_blizzard` | Winter Blizzard Howl | `amb_snow_blizzard.wav` | 8.0 | Env | Deep multi-octave howling noise with sweeping whistling resonance |
| `amb_swamp_insects` | Swamp Insects & Crickets | `amb_swamp_insects.wav` | 6.0 | Env | Rapid pulsed square-wave chirps and low subterranean hum |
| `amb_mountain_peaks` | Mountain Ridge Draft | `amb_mountain_peaks.wav` | 8.0 | Env | High-altitude thin wind gusts with echoing acoustic reflection |
| `amb_fire_crackle` | Campfire Crackle | `amb_fire_crackle.wav` | 6.0 | Env | Random micro-transients simulating popping embers over warm saw pad |

---

## 3. UI Audio Effects (15 SFX)
Stored in `assets/audio/sfx/ui/`.

| Asset ID | Title | File | Dur (s) | Character & Harmonic Profile |
|---|---|---|---|---|
| `sfx_ui_click` | Button Click | `sfx_ui_click.wav` | 0.05 | High crisp tick transient with rapid 25ms exponential decay |
| `sfx_ui_hover` | Button Hover | `sfx_ui_hover.wav` | 0.04 | Subtle sine ping at 780 Hz with soft low-pass filter |
| `sfx_ui_confirm` | Action Confirm | `sfx_ui_confirm.wav` | 0.18 | Ascending two-tone triad (523 Hz -> 659 Hz, C5 to E5) |
| `sfx_ui_cancel` | Action Cancel | `sfx_ui_cancel.wav` | 0.15 | Descending two-tone interval (440 Hz -> 330 Hz, A4 to E4) |
| `sfx_ui_tab` | Tab Switch | `sfx_ui_tab.wav` | 0.08 | Crisp slide transient simulating index tab click |
| `sfx_ui_panel_open` | Window Open | `sfx_ui_panel_open.wav` | 0.20 | Upward resonant filter sweep with wooden latch click |
| `sfx_ui_panel_close` | Window Close | `sfx_ui_panel_close.wav` | 0.18 | Downward damped sweep with leather closure feel |
| `sfx_ui_item_select` | Item Selected | `sfx_ui_item_select.wav` | 0.06 | Light glass bead click |
| `sfx_ui_cosmetic_apply` | Cosmetic Equip | `sfx_ui_cosmetic_apply.wav` | 0.22 | Metallic armor snap and fabric buckle transient |
| `sfx_ui_cosmetic_remove`| Cosmetic Unequip | `sfx_ui_cosmetic_remove.wav`| 0.16 | Leather unfasten and subtle fabric rustle |
| `sfx_ui_settings_adjust`| Slider Tick | `sfx_ui_settings_adjust.wav`| 0.04 | Mechanical micro-click for volume adjustments |
| `sfx_ui_error` | Action Error | `sfx_ui_error.wav` | 0.25 | Low dual square-wave dissonance (165 Hz + 175 Hz beat frequency) |
| `sfx_ui_success` | Level / Upgrade | `sfx_ui_success.wav` | 0.35 | Triple chime ascending chord (C5 -> E5 -> G5) |
| `sfx_ui_back` | Navigation Back | `sfx_ui_back.wav` | 0.10 | Soft wooden strike |
| `sfx_ui_modal_confirm` | Modal Affirmation | `sfx_ui_modal_confirm.wav` | 0.25 | Heavy brass chime resonance |

---

## 4. Player & Movement Audio Effects (14 SFX)
Stored in `assets/audio/sfx/player/`.

| Asset ID | Title | Terrains / Context | Sound Characteristic |
|---|---|---|---|
| `sfx_footstep_grass_001`| Footstep Grass A | Forest, Meadows | Soft leafy rustle with low dirt thud |
| `sfx_footstep_grass_002`| Footstep Grass B | Forest, Meadows | Alternating foliage snap |
| `sfx_footstep_dirt_001` | Footstep Dirt A  | Trails, Roads | Dry packed earth impact |
| `sfx_footstep_dirt_002` | Footstep Dirt B  | Trails, Roads | Subtle granular grit crunch |
| `sfx_footstep_stone_001`| Footstep Stone A | Village, Ruins | Crisp hard stone tap with micro-echo |
| `sfx_footstep_stone_002`| Footstep Stone B | Village, Ruins | Resonant cobblestone heel strike |
| `sfx_footstep_snow_001` | Footstep Snow A  | Snow Biome | Heavy crunchy powdered snow compaction |
| `sfx_footstep_snow_002` | Footstep Snow B  | Snow Biome | Crusty snowpack deformation |
| `sfx_footstep_sand_001` | Footstep Sand A  | Desert Biome | Sliding loose granular friction |
| `sfx_footstep_sand_002` | Footstep Sand B  | Desert Biome | Muffled sand compaction |
| `sfx_footstep_mud_001`  | Footstep Mud A   | Swamp Biome | Wet squelch and suction transient |
| `sfx_footstep_mud_002`  | Footstep Mud B   | Swamp Biome | Liquid earth displacement |
| `sfx_player_hurt`       | Player Hurt      | Combat Damage | Punchy physical impact with muffled grunt overtone |
| `sfx_player_death`      | Player Fallen    | Combat Defeat | Heavy collapse transient and dissipating harmonic |

---

## 5. Weapon & Combat Audio Effects (9 SFX)
Stored in `assets/audio/sfx/weapons/`.

| Asset ID | Title | Weapon Class | Acoustic Profile |
|---|---|---|---|
| `sfx_sword_swing_001` | Fast Sword Whoosh | Light Swords, Daggers | Whipping air displacement, high-pitch blade whoosh |
| `sfx_sword_swing_002` | Heavy Sword Cleave | Broadswords, Claymores | Deep resonant whoosh with mid-range bass punch |
| `sfx_sword_impact_flesh`| Blade Flesh Strike | Slashing Damage | Solid meat impact with wet blade edge slice |
| `sfx_sword_impact_metal`| Steel Blade Parry | Block / Armor Clash | High-frequency metal ring at 1.4 kHz with fast vibrato |
| `sfx_sword_impact_wood` | Wood Shield Block | Defensive Block | Heavy dull timber thud |
| `sfx_bow_draw` | Bowstring Tension | Bows, Crossbows | Creaking wooden limb and tightening string pitch rise |
| `sfx_bow_release` | Bowstring Release | Bow Attack | Sudden string twang with rapid decaying snap |
| `sfx_arrow_whistle` | Arrow in Flight | Projectile | High-speed doppler whistle |
| `sfx_arrow_impact` | Arrow Target Hit | Hit Detection | Sharp penetrative thwack into target |

---

## 6. Interaction & Environmental SFX (8 SFX)
Stored in `assets/audio/sfx/interactions/`.

| Asset ID | Title | Gameplay Interaction | Acoustic Description |
|---|---|---|---|
| `sfx_item_pickup` | Generic Loot Pickup | Ground Item Collection | Cheerful light chime |
| `sfx_item_drop` | Drop Item | Inventory Management | Soft canvas pack drop |
| `sfx_coin_pickup` | Gold Coin Pickup | Economy / Currency | Pure dual-coin clink (2.2 kHz & 2.6 kHz) |
| `sfx_gem_pickup` | Rare Gem Pickup | Valuable Gem Collection | Crystal bell harmonic resonance |
| `sfx_potion_drink` | Quaff Potion | Consumable | Bubbling liquid gulp and bottle cork pop |
| `sfx_door_open` | Wooden Door Open | Building Entry | Squeaking iron hinge and latch release |
| `sfx_door_close` | Wooden Door Shut | Building Exit | Solid wooden frame impact |
| `sfx_chest_open` | Treasure Chest Open | Loot Cache | Heavy iron lock turn and creaking lid lift |

---

## 7. Match & Event Stingers (5 SFX)
Stored in `assets/audio/sfx/events/`.

| Asset ID | Title | Match Event | Musical Composition |
|---|---|---|---|
| `sfx_match_countdown` | 3-2-1 Countdown | Pre-Match Countdown | Deep clockwork tick with rising anticipation pitch |
| `sfx_match_start` | Battle Commences | Match Start Horn | Resonant war horn call (G3 to C4) |
| `sfx_gameplay_alert` | Zone Shrinking Alert| BR Circle Movement | Warning siren pulses alerting player |
| `sfx_stinger_victory` | Victory Stinger | Match Won | Brilliant major fanfare chord with royal shimmer |
| `sfx_stinger_defeat` | Defeat Stinger | Match Eliminated | Dissonant minor lament chord fading to wind |

