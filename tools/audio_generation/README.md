# Wildbound Procedural Audio Generation Pipeline

This suite generates 100% original, deterministic, synthesized audio assets for Wildbound using pure Python, math, and NumPy/standard library wave routines. No third-party copyrighted sound packs or external assets are used.

## Architecture

```
tools/audio_generation/
├── effects/
│   ├── envelopes.py      # ADSR, exponential decay, linear fades
│   ├── filters.py        # Lowpass, highpass, bandpass filtering
│   ├── mixing.py         # Normalization, stereo duplicates, 16-bit PCM WAV export
│   ├── modulation.py     # Amplitude and frequency modulation
│   ├── noise.py          # White, pink, and brown noise generators
│   ├── oscillators.py    # Sine, square, triangle, sawtooth, chirp
│   └── reverb.py         # Delay lines, reflections, and simple reverb
├── generators/
│   ├── combat.py         # Player/enemy hurt, death, critical strikes, flesh impacts
│   ├── creatures.py      # Goblin, Skeleton, Wolf, Boar, Bat, Wildlife
│   ├── environmental.py   # 6 Biome loops, rivers, waterfalls, village, one-shots
│   ├── events.py         # Match countdowns, start horns, alerts, victory/defeat stingers
│   ├── footsteps.py      # 8 Surfaces x 3 variations (24 total footstep SFX)
│   ├── interactions.py   # Doors, chests, pickups, drops, coins, gems, potions
│   ├── music.py          # 11 original stereo ambient & combat music tracks
│   └── ui.py             # 15 UI navigation, button, and inventory sounds
├── presets/
│   └── generation_presets.json
├── validation/
│   ├── analyze_audio.py  # Computes RMS, peak dB, duration metrics
│   ├── validate_audio.py # Validates sample rate, bit depth, channel count, silence
│   └── validate_loops.py # Verifies loop boundary continuity
├── generate_all.py       # Master pipeline orchestrator
└── README.md
```

## Running the Pipeline

To regenerate all 128 procedural sound assets:
```bash
python tools/audio_generation/generate_all.py
```

To validate all WAV assets:
```bash
python tools/audio_generation/validation/validate_audio.py
```

To inspect loudness and dynamic range:
```bash
python tools/audio_generation/validation/analyze_audio.py
```

