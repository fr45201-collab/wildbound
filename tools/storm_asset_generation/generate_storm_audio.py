#!/usr/bin/env python3
"""
Procedural Audio Generator for Wildbound Phase 31.
Synthesizes original WAV sound effects and ambient rumble loops for the Corruption Storm.
"""

import os
import wave
import struct
import numpy as np

SAMPLE_RATE = 44100

def write_wav(path, samples, sample_rate=SAMPLE_RATE, channels=1):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    # Normalize to -1.0 .. 1.0 then scale to 16-bit int
    max_val = np.max(np.abs(samples))
    if max_val > 0.0:
        samples = samples / max_val * 0.90
    int_samples = (samples * 32767.0).astype(np.int16)
    
    with wave.open(path, 'wb') as wf:
        wf.setnchannels(channels)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        if channels == 1:
            wf.writeframes(int_samples.tobytes())
        else:
            # Interleave channels
            wf.writeframes(int_samples.tobytes())
    print(f"Generated {path} ({len(int_samples)/sample_rate:.2f}s, {channels}ch)")

def generate_storm_warning(path):
    """Ominous low-to-mid rising brassy warning siren with resonant tremolo."""
    duration = 2.5
    t = np.linspace(0, duration, int(SAMPLE_RATE * duration), endpoint=False)
    
    # Dual rising saw/sine
    freq1 = np.linspace(130.0, 260.0, len(t))
    freq2 = np.linspace(132.0, 264.0, len(t))
    
    sig = 0.5 * np.sin(2 * np.pi * freq1 * t) + 0.3 * np.sin(2 * np.pi * freq2 * t)
    # Add pulse modulation (6 Hz)
    tremolo = 0.8 + 0.2 * np.sin(2 * np.pi * 6.0 * t)
    sig = sig * tremolo
    
    # Envelope: 0.1s attack, 2.0s sustain, 0.4s fadeout
    env = np.ones_like(t)
    attack_len = int(SAMPLE_RATE * 0.15)
    fade_len = int(SAMPLE_RATE * 0.5)
    env[:attack_len] = np.linspace(0.0, 1.0, attack_len)
    env[-fade_len:] = np.linspace(1.0, 0.0, fade_len)
    
    write_wav(path, sig * env)

def generate_storm_shrink(path):
    """Energy sweep / crackling perimeter contraction."""
    duration = 2.0
    t = np.linspace(0, duration, int(SAMPLE_RATE * duration), endpoint=False)
    
    # Downward resonant frequency sweep
    freq = np.linspace(420.0, 80.0, len(t))
    sig = 0.6 * np.sin(2 * np.pi * freq * t)
    
    # Add electric crackle
    noise = np.random.uniform(-0.25, 0.25, len(t))
    crackle = np.where(np.random.rand(len(t)) > 0.96, noise * 2.0, 0.0)
    sig = sig + crackle
    
    env = np.ones_like(t)
    fade_len = int(SAMPLE_RATE * 0.4)
    env[-fade_len:] = np.linspace(1.0, 0.0, fade_len)
    
    write_wav(path, sig * env)

def generate_storm_damage(path):
    """Short corruption sizzle / caustic flesh tick."""
    duration = 0.35
    t = np.linspace(0, duration, int(SAMPLE_RATE * duration), endpoint=False)
    
    # Harsh noise filtered with high sizzle
    noise = np.random.uniform(-0.5, 0.5, len(t))
    sub = 0.4 * np.sin(2 * np.pi * 75.0 * t)
    sizzle = 0.5 * np.sin(2 * np.pi * 1800.0 * t) * noise
    
    sig = sub + sizzle
    env = np.exp(-t * 9.0)
    write_wav(path, sig * env)

def generate_storm_rumble(path):
    """Low-frequency deep atmospheric rumble ambient loop."""
    duration = 4.0
    t = np.linspace(0, duration, int(SAMPLE_RATE * duration), endpoint=False)
    
    # Heavy low sub-bass (45Hz, 58Hz, 72Hz)
    sub1 = 0.5 * np.sin(2 * np.pi * 48.0 * t)
    sub2 = 0.3 * np.sin(2 * np.pi * 55.0 * t + 0.5)
    noise = np.random.uniform(-0.15, 0.15, len(t))
    
    # Smooth moving wind filter
    sig = sub1 + sub2 + noise
    # Loop crossfade
    fade = int(SAMPLE_RATE * 0.2)
    sig[:fade] = sig[:fade] * np.linspace(0.5, 1.0, fade) + sig[-fade:] * np.linspace(0.5, 0.0, fade)
    
    write_wav(path, sig)

def main():
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../assets/audio"))
    
    p_warn = os.path.join(root, "sfx/events/sfx_storm_warning.wav")
    p_shrink = os.path.join(root, "sfx/events/sfx_storm_shrink.wav")
    p_dmg = os.path.join(root, "sfx/combat/sfx_storm_damage.wav")
    p_rumble = os.path.join(root, "ambience/amb_storm_rumble.wav")
    
    generate_storm_warning(p_warn)
    generate_storm_shrink(p_shrink)
    generate_storm_damage(p_dmg)
    generate_storm_rumble(p_rumble)
    print("All storm audio files successfully generated.")

if __name__ == "__main__":
    main()

