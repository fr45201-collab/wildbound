# WILDBOUND — PROCEDURAL AUDIO GENERATION PIPELINE

## 1. Pipeline Overview & Motivation

To guarantee absolute legal safety, 100% royalty freedom, and total ownership, Wildbound replaces traditional sampled audio with a procedural mathematical synthesis pipeline implemented in pure Python (`tools/audio_generation/`).

* **Zero External Dependencies**: Implemented using pure Python standard libraries (`math`, `struct`, `wave`, `random`). No heavy C extensions, `scipy`, `numpy`, or proprietary audio plugins required.
* **Format**: 44,100 Hz sample rate, 16-bit signed integer linear PCM, exported directly to standard RIFF WAV files.
* **Deterministic Reproducibility**: Seeded pseudorandom generators ensure identical waveform outputs on every run.

---

## 2. Generator Core (`synth.py`)

The synthesizer engine provides fundamental DSP building blocks:

### Waveform Generators
* **Sine Wave**: $\sin(2\pi f t)$ — pure tones, sub-basses, clean bells, flutes.
* **Triangle Wave**: Integrated square — warm retro chiptune leads, gentle string pads.
* **Sawtooth Wave**: Ramp waveform — brass fanfares, aggressive synth sweeps, bow string resonance.
* **Square Wave**: Pulse waveform — hollow reed sounds, retro game clicks, dissonant alarms.
* **White & Pink Noise**: Random Gaussian distribution — footsteps, wind gusts, explosions, water flow.

### DSP Modulators & Filters
* **ADSR Envelope**: Attack, Decay, Sustain, Release linear and exponential contouring.
* **One-Pole Low-Pass Filter**: Exponential moving average smoothing high frequencies:
  $$y[n] = y[n-1] + \alpha (x[n] - y[n-1])$$
* **Resonant Filter Sweeps**: Dynamically varying $\alpha$ over time for sweeps and wind gusts.
* **Stereo Ping-Pong Delay**: Feedback delay buffer producing spatial width and acoustic ambience.

---

## 3. Automation Scripts

1. `tools/audio_generation/generate_all.py`:
   * Master execution script generating all 68 audio files across 7 directories (`assets/audio/music/`, `assets/audio/ambience/`, `assets/audio/sfx/ui/`, `assets/audio/sfx/player/`, `assets/audio/sfx/weapons/`, `assets/audio/sfx/interactions/`, `assets/audio/sfx/events/`).
   * Generates master metadata catalog `data/audio/audio_catalog.json`.
   * Automatically registers every asset into `assets/manifests/asset_manifest.json` for AMS resolution.
2. `tools/audio_generation/validate_audio.py`:
   * Inspects every generated `.wav` file on disk, validates RIFF chunk headers, sample rate (44.1 kHz), bit depth (16-bit), channels (mono/stereo), non-zero byte size, and duration.
   * Produces a comprehensive validation summary ensuring zero corrupt or clipped files.

