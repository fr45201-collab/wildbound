import os
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle, sawtooth
from effects.envelopes import adsr
from effects.filters import lowpass_filter

def generate_events(out_dir: str = "assets/audio/sfx/events") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)

    events = [
        ("sfx_match_countdown", "Match Countdown Beep", 0.15, 880.0, 880.0, 7001),
        ("sfx_match_start", "Match Start Horn", 0.85, 220.0, 440.0, 7002),
        ("sfx_gameplay_alert", "Urgent Gameplay Alert", 0.35, 650.0, 950.0, 7003),
        ("sfx_stinger_victory", "Victory Fanfare Stinger", 1.20, 523.25, 1046.5, 7004),
        ("sfx_stinger_defeat", "Defeat Gloom Stinger", 1.20, 392.0, 196.0, 7005)
    ]

    results = []

    for sfx_id, title, duration, f1, f2, seed in events:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)
            ratio = t / duration
            freq = f1 + ratio * (f2 - f1)

            if "horn" in sfx_id:
                val = sawtooth(freq, t) * 0.7 + sine(freq * 2.0, t) * 0.3
            elif "stinger" in sfx_id:
                val = triangle(freq, t) * 0.6 + sine(freq * 1.5, t) * 0.4
            else:
                val = sine(freq, t)

            env = adsr(t, duration, a=0.01, d=0.1, s=0.6, r=0.2)
            samples.append(val * env * 0.75)

        samples = lowpass_filter(samples, cutoff=0.7)
        wav_path = os.path.join(out_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        results.append({
            "id": sfx_id,
            "title": title,
            "category": "events",
            "file": f"audio/sfx/events/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": "sfx",
            "default_volume": 0.75,
            "description": f"Match event sound: {title}",
            "seed": seed
        })

    return results

