import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, sawtooth
from effects.noise import white_noise
from effects.filters import lowpass_filter

def generate_interactions(out_dir: str = "assets/audio/sfx/interactions") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)

    interactions = [
        ("sfx_item_pickup", "Item Pickup", 0.15, 600.0, 1100.0, 5001),
        ("sfx_item_drop", "Item Drop", 0.12, 400.0, 200.0, 5002),
        ("sfx_coin_pickup", "Gold Coin Clink", 0.22, 1760.0, 2637.0, 5003),
        ("sfx_gem_pickup", "Faceted Gem Chime", 0.35, 1318.5, 2093.0, 5004),
        ("sfx_potion_drink", "Potion Gulp", 0.30, 250.0, 450.0, 5005),
        ("sfx_door_open", "Wooden Door Open Creak", 0.45, 120.0, 180.0, 5006),
        ("sfx_door_close", "Wooden Door Heavy Shut", 0.25, 180.0, 80.0, 5007),
        ("sfx_chest_open", "Treasure Chest Lid Open", 0.50, 140.0, 320.0, 5008)
    ]

    results = []

    for sfx_id, title, duration, f1, f2, seed in interactions:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)
            ratio = t / duration
            freq = f1 + ratio * (f2 - f1)

            if "coin" in sfx_id or "gem" in sfx_id:
                tone = sine(f1, t) * math.exp(-t * 10.0) + sine(f2, t) * math.exp(-t * 8.0)
                val = tone * 0.7
            elif "door" in sfx_id or "chest" in sfx_id:
                creak = sawtooth(freq, t) * math.sin(t * 12.0)
                val = creak * 0.6 + white_noise() * 0.2
            else:
                val = sine(freq, t) * math.exp(-t * 15.0)

            samples.append(val * 0.7)

        samples = lowpass_filter(samples, cutoff=0.7)
        wav_path = os.path.join(out_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        results.append({
            "id": sfx_id,
            "title": title,
            "category": "interactions",
            "file": f"audio/sfx/interactions/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": "sfx",
            "default_volume": 0.65,
            "description": f"Interaction sound: {title}",
            "seed": seed
        })

    return results

