# Environmental Audio Specification

## 1. Overview & Mechanics

The Environmental Audio Subsystem renders immersive, layered, geographic atmospheres across all 6 biomes in Wildbound:
1. **Forest**
2. **Desert**
3. **Snow**
4. **Swamp**
5. **Mountain**
6. **Village & Buildings**

The environmental audio model combines **continuous seamless ambient loops** (wind, flowing rivers, thunder, crickets) with **randomized periodic one-shots** (distant bird takeoffs, glacial ice fractures, marsh gas bubbles, rockfalls).

---

## 2. Biome Sound Design Specifications

### 2.1 Forest
- **Continuous Ambience:**
  - `amb_forest_wind`: Filtered brown/pink noise breeze through high tree canopies ($8.0\text{s}$ seamless stereo loop).
  - `amb_forest_birds`: High-frequency chirps ($2.4\text{ kHz} - 3.2\text{ kHz}$) with subtle ambient reverb.
  - `amb_forest_river`: Water flow turbulence with gentle resonant bubbling.
- **Periodic One-Shots:**
  - `sfx_wildlife_bird_takeoff`: Sudden wing flutters ($0.35\text{s}$) triggered every $7\text{s}-14\text{s}$.
  - `sfx_wildlife_butterfly_flutter`: Delicate low-amplitude wing murmur.

### 2.2 Desert
- **Continuous Ambience:**
  - `amb_desert_wind`: Shifting sandstorm gusts with wide dynamic modulation.
- **Periodic One-Shots:**
  - `sfx_env_desert_sand_shift`: Whispering dune slip face movement ($0.65\text{s}$).

### 2.3 Snow
- **Continuous Ambience:**
  - `amb_snow_blizzard`: Whistling sub-zero winds and high-frequency atmospheric chill.
- **Periodic One-Shots:**
  - `sfx_env_ice_crack`: Sharp glacial fracturing snap ($2.2\text{ kHz}$ impulse with rapid decay).

### 2.4 Swamp
- **Continuous Ambience:**
  - `amb_swamp_insects`: Dense cricket oscillations ($4.5\text{ kHz}$) interwoven with low-frequency bullfrog ribbits ($120\text{ Hz}$).
- **Periodic One-Shots:**
  - `sfx_env_swamp_bubble`: Rising-frequency pitch chirp pop ($180\text{ Hz} \to 530\text{ Hz}$) representing marsh gas escape.

### 2.5 Mountain
- **Continuous Ambience:**
  - `amb_mountain_wind`: High-altitude gale force winds echoing off alpine crags.
  - `amb_waterfall_heavy`: Dense, torrential brown noise roar of plunging mountain cascades.
- **Periodic One-Shots:**
  - `sfx_env_mountain_rockfall`: Distant tumbling scree and rumbling boulder impacts ($0.85\text{s}$).

### 2.6 Village & Interiors
- **Continuous Ambience:**
  - `amb_village_market`: Muffled crowd murmurs, shuffling footwear, and distant rustic settlement tones.
  - `amb_village_blacksmith`: Rhythmic metallic anvil strikes ($1.85\text{ kHz}$) over a warm forge fire drone.
  - `amb_indoor_fireplace`: Realistic crackling pine wood embers and soft low-frequency heat resonance.
- **Periodic One-Shots:**
  - `sfx_env_wood_creak`: Timber settlement structural stress creaks ($0.40\text{s}$).

---

## 3. Playback Rules & Scheduling

1. **Layer Mixing:** Biomes define an array of active ambient loops (e.g. `["amb_forest_wind", "amb_forest_birds", "amb_forest_river"]`).
2. **Periodic Interval Generator:** In `AudioSystem::Update(dt)`, a random timer interval is chosen:
   $$T_{interval} = 7.0\text{s} + \text{rand}(0..7.0\text{s})$$
   When the timer expires, an appropriate environmental one-shot is triggered at low volume ($-6\text{ dB}$ to $-10\text{ dB}$).
3. **Biome Transitions:** Moving into a new biome switches the active music and resets the ambient one-shot timer to ensure clean atmospheric handoffs.

