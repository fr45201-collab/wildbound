# Ambient Music Specification

## 1. Overview & Musical Direction

Wildbound's music system features 11 completely original, synthesized procedural compositions designed to establish mood, enhance geographic identity, and heighten combat tension without relying on third-party tracks or copyrighted motifs.

All tracks are exported in 16-bit PCM, 44.1 kHz stereo, and feature crossfaded loop boundaries.

---

## 2. Music Track Catalog & Compositional Profiles

| Track ID | Title | Key / Scale | Duration | Loop | Musical Character |
|---|---|---|---|---|---|
| `mus_main_menu` | Main Menu Theme | D Major ($293.7\text{ Hz}$) | $10.0\text{s}$ | True | Majestic fantasy motif with warm pads and plucked lute melody. |
| `mus_lobby` | Lobby Theme | G Major ($196.0\text{ Hz}$) | $8.0\text{s}$ | True | Relaxed campfire acoustic lute, warm drone, and gentle pizzicato. |
| `mus_forest` | Forest Exploration | A Natural ($220.0\text{ Hz}$) | $12.0\text{s}$ | True | Woodland flutes, whispering strings, and pastoral arpeggios. |
| `mus_desert` | Desert Exploration | E Phrygian Dominant | $10.0\text{s}$ | True | Shifting desert lute, modal drone, and arid tension. |
| `mus_snow` | Snow Exploration | C Minor / Pentatonic | $10.0\text{s}$ | True | Crystalline bell chimes, cold atmospheric sub-drones, and spacious echoes. |
| `mus_swamp` | Swamp Exploration | D Chromatic Minor | $10.0\text{s}$ | True | Murky bass pulse, dissonant bell harmonics, and eerie resonance. |
| `mus_mountain` | Mountain Exploration | G Dorian ($196.0\text{ Hz}$) | $10.0\text{s}$ | True | Resonant brass horn echoes across high alpine peaks. |
| `mus_village` | Village Warmth | C Pastoral Major | $10.0\text{s}$ | True | Rustic acoustic plucked chords, pastoral flute, and welcoming room tone. |
| `mus_combat` | Combat Intensity | A Minor Pentatonic | $8.0\text{s}$ | True | High-intensity rhythmic war percussion, bass pulses, and brass stabs. |
| `mus_victory` | Victory Fanfare | C Major Triumphant | $4.5\text{s}$ | False | Triumphant ascending fanfare chord sequence with celebratory chimes. |
| `mus_defeat` | Defeat Elegy | E Minor Somber | $4.5\text{s}$ | False | Somber descending minor chord progression with fading cello drone. |

---

## 3. Streaming and Transition Mechanics

### 3.1 Streaming via raylib `Music`
- Music tracks are streamed incrementally from disk via raylib's `Music` structure to avoid consuming tens of megabytes of RAM.
- In `AudioSystem::Update(dt)`, `UpdateMusicStream(m_loadedMusic[id])` buffers audio frames continuously.

### 3.2 Volume Fading & Crossfading
- When switching tracks via `PlayMusic(musicId, loop, fadeTime)`:
  1. The outgoing track is stopped cleanly.
  2. The incoming track starts at volume $0.0$.
  3. `m_musicFadeTimer` advances from $0.0$ to `fadeTime` (default $0.5\text{s}$ to $1.0\text{s}$), interpolating volume linearly up to the target effective volume:
     $$V(t) = V_{effective} \times \min\left(1.0, \frac{t_{fade}}{T_{duration}}\right)$$

### 3.3 Loop Boundary Smoothing
All looping tracks (`loop = true`) undergo automated boundary crossfading during Python synthesis:
- The last $0.15\text{s}$ of the track is linearly blended into the first $0.15\text{s}$ of the audio buffer:
  $$S[i] = S[i] \cdot \left(\frac{i}{N_{fade}}\right) + S[N - N_{fade} + i] \cdot \left(1.0 - \frac{i}{N_{fade}}\right)$$
- This ensures zero clicks, pops, or volume discontinuities when the audio stream wraps around.

