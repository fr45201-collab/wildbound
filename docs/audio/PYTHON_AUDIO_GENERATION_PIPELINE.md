# Python Procedural Audio Generation Pipeline

## 1. Overview & Philosophy

Wildbound enforces a strict, zero-third-party-copyright audio policy. Every single sound effect, footstep, ambient layer, creature vocalization, and musical track is synthesized procedurally in Python using mathematical formulations, digital signal processing (DSP) filters, oscillators, and deterministic pseudo-random seeds.

---

## 2. Pipeline Directory Structure

```
tools/audio_generation/
├── effects/
│   ├── envelopes.py      # ADSR, exponential decay, linear fades
│   ├── filters.py        # 1-pole lowpass, highpass, bandpass DSP filters
│   ├── mixing.py         # Dynamic range normalization, 16-bit PCM WAV export
│   ├── modulation.py     # Amplitude modulation (tremolo), frequency modulation (vibrato)
│   ├── noise.py          # White noise, pink noise (Paul Kellet 7-pole), brownian walk
│   ├── oscillators.py    # Sine, square (duty cycle), triangle, sawtooth, chirp
│   └── reverb.py         # Delay lines and multi-tap comb reflections
├── generators/
│   ├── combat.py         # Player/enemy hurt, deaths, crits, melee flesh impacts
│   ├── creatures.py      # Goblin, Skeleton, Wolf, Boar, Bat, Wildlife
│   ├── environmental.py   # 6 Biome loops, rivers, waterfalls, village, one-shots
│   ├── events.py         # Match start horns, alerts, victory/defeat stingers
│   ├── footsteps.py      # 8 surfaces x 3 variations = 24 footstep WAVs
│   ├── interactions.py   # Chests, doors, item pickups, drops, coins, gems, potions
│   ├── music.py          # 11 original stereo ambient & battle compositions
│   └── ui.py             # 15 UI navigation clicks, tabs, panels, notifications
├── presets/
│   └── generation_presets.json
├── validation/
│   ├── analyze_audio.py  # Computes RMS, peak dB, duration metrics
│   ├── validate_audio.py # Validates sample rate, bit depth, channel count, silence
│   └── validate_loops.py # Verifies loop boundary continuity
├── generate_all.py       # Master pipeline orchestrator
└── README.md
```

---

## 3. Core Synthesis Techniques

### 3.1 Waveform Generation
- **Sine Wave:** $\sin(2\pi f t)$ for clean bell tones, chimes, and fundamental basses.
- **Triangle Wave:** $2 \cdot |2(ft - \lfloor ft + 0.5 \rfloor)| - 1$ for hollow acoustic plucks.
- **Sawtooth Wave:** $2(ft - \lfloor ft + 0.5 \rfloor)$ for brass horns, harsh creature snarls, and blade swings.
- **Square Wave:** Pulse modulation for chiptune tones and cricket buzzes.
- **Chirp:** Linear frequency sweep $f(t) = f_1 + \frac{t}{T}(f_2 - f_1)$ for UI hover sweeps and bubble pops.

### 3.2 Noise Synthesis
- **White Noise:** Uniform random distribution $\mathcal{U}(-1.0, 1.0)$ for impacts and sword whooshes.
- **Pink Noise:** Kellet filter tree approximating a $-3\text{ dB/octave}$ pink falloff for water streams and wind.
- **Brown Noise:** Integrated random walk approximating a $-6\text{ dB/octave}$ brown falloff for waterfall roars and stone rumbles.

### 3.3 Dynamic Shaping & Filtering
- **ADSR Envelopes:** Attack, Decay, Sustain, Release piecewise linear envelope shaping.
- **Exponential Decay:** $e^{-\lambda t}$ for crisp percussion thuds and metallic strikes.
- **Single-Pole Lowpass Filter:**
  $$y[n] = y[n-1] + \alpha \cdot (x[n] - y[n-1])$$
  used to warm acoustic sounds and tame harsh high frequencies.

---

## 4. Execution & Automated Manifest Generation

Running:
```bash
python tools/audio_generation/generate_all.py
```
Performs the full end-to-end build:
1. Synthesizes 128 distinct audio WAV files into `assets/audio/`.
2. Generates `data/audio/audio_catalog.json` with all 8 audio buses.
3. Generates `data/audio/biome_audio.json` mapping all 6 biomes.
4. Registers newly added entries into `assets/manifests/asset_manifest.json` under `CC0 1.0 Universal`.
5. Executes automated integrity validation via `validate_audio.py`.

