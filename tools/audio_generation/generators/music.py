import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle
from effects.envelopes import adsr
from effects.filters import lowpass_filter
from effects.reverb import apply_delay

def generate_music(out_dir: str = "assets/audio/music") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)

    tracks = [
        ("mus_main_menu", "Main Menu Theme", "Majestic fantasy motif with warm pads and plucked melody.", 10.0, True, 1001),
        ("mus_lobby", "Lobby Theme", "Relaxed campfire acoustic lute and gentle bass drone.", 8.0, True, 1002),
        ("mus_forest", "Forest Exploration", "Woodland flutes, whispering strings, and gentle pizzicato.", 12.0, True, 1003),
        ("mus_desert", "Desert Exploration", "Phrygian modal drone with plucked desert lute melody.", 10.0, True, 1004),
        ("mus_snow", "Snow Exploration", "Crystalline bell chimes and cold atmospheric pads.", 10.0, True, 1005),
        ("mus_swamp", "Swamp Exploration", "Murky chromatic bass pulse and eerie ambient resonance.", 10.0, True, 1006),
        ("mus_mountain", "Mountain Exploration", "Resonant brass horn echoes across high peaks.", 10.0, True, 1007),
        ("mus_village", "Village Warmth", "Rustic acoustic lute, pastoral flute, and tavern drone.", 10.0, True, 1008),
        ("mus_combat", "Combat Intensity", "High-intensity rhythmic war percussion and brass stabs.", 8.0, True, 1009),
        ("mus_victory", "Victory Fanfare", "Triumphant ascending fanfare chord with celebratory chimes.", 4.5, False, 1010),
        ("mus_defeat", "Defeat Elegy", "Somber descending minor chord sequence with fading cello.", 4.5, False, 1011)
    ]

    results = []

    for track_id, title, desc, duration, loop, seed in tracks:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = [0.0] * num_samples

        if "desert" in track_id:
            scale = [-5, -4, -1, 0, 2, 3, 6, 7] # Phrygian dominant
            base_f = 220.0
        elif "combat" in track_id:
            scale = [-12, -9, -5, -2, 0, 3, 7] # Minor pentatonic
            base_f = 110.0
        elif "defeat" in track_id:
            scale = [-12, -8, -5, -1, 0, 3] # Minor sombre
            base_f = 164.81
        elif "victory" in track_id:
            scale = [0, 4, 7, 11, 12, 16, 19] # Major triumphant
            base_f = 261.63
        elif "village" in track_id:
            scale = [0, 2, 4, 5, 7, 9, 11, 12] # Major pastoral
            base_f = 261.63
        else:
            scale = [0, 2, 4, 7, 9, 12, 14] # Major pentatonic / natural
            base_f = 220.0

        # Layer 1: Sustained Atmospheric Drone / Pad
        drone_f = base_f * 0.5
        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)
            drone = 0.25 * (sine(drone_f, t) + 0.5 * sine(drone_f * 1.5, t))
            samples[i] += drone

        # Layer 2: Rhythmic Arpeggiated Plucks
        note_duration = 0.5 if "combat" not in track_id else 0.25
        notes_count = int(duration / note_duration)
        for n in range(notes_count):
            semi = scale[n % len(scale)]
            freq = base_f * (2.0 ** (semi / 12.0))
            start_i = int(n * note_duration * SAMPLE_RATE)
            note_len = int(note_duration * SAMPLE_RATE)

            for j in range(note_len):
                idx = start_i + j
                if idx < num_samples:
                    t_note = j / float(SAMPLE_RATE)
                    env = adsr(t_note, note_duration, a=0.01, d=0.1, s=0.3, r=0.15)
                    osc = 0.35 * triangle(freq, t_note) + 0.15 * sine(freq * 2.0, t_note)
                    samples[idx] += osc * env

        # Layer 3: Percussion pulses for combat
        if "combat" in track_id:
            beat_step = int(0.25 * SAMPLE_RATE)
            for b in range(0, num_samples, beat_step):
                for k in range(min(4000, num_samples - b)):
                    t_beat = k / float(SAMPLE_RATE)
                    kick = math.sin(2.0 * math.pi * (80.0 * (1.0 - t_beat * 8.0)) * t_beat) * math.exp(-t_beat * 15.0)
                    samples[b + k] += kick * 0.45

        # Smooth loop boundaries (crossfade first and last 0.15s if loop)
        if loop:
            fade_samples = int(0.15 * SAMPLE_RATE)
            for i in range(fade_samples):
                fade_in = i / float(fade_samples)
                fade_out = 1.0 - fade_in
                samples[i] = samples[i] * fade_in + samples[num_samples - fade_samples + i] * fade_out

        samples = apply_delay(samples, delay_samples=int(SAMPLE_RATE * 0.2), decay=0.25)
        samples = lowpass_filter(samples, cutoff=0.35)

        wav_path = os.path.join(out_dir, f"{track_id}.wav")
        export_wav(wav_path, samples, channels=2)

        results.append({
            "id": track_id,
            "title": title,
            "category": "music",
            "file": f"audio/music/{track_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 2,
            "bit_depth": 16,
            "looping": loop,
            "bus": "music",
            "default_volume": 0.70,
            "description": desc,
            "seed": seed
        })

    return results

