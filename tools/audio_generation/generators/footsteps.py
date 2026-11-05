import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle
from effects.noise import white_noise
from effects.filters import lowpass_filter

def generate_footsteps(out_dir: str = "assets/audio/sfx/player") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)
    results = []
    terrains = ["grass", "dirt", "stone", "snow", "sand", "mud", "wood", "water"]

    for t_name in terrains:
        for v in range(1, 4):
            seed = 3000 + terrains.index(t_name) * 10 + v
            random.seed(seed)
            sfx_id = f"sfx_footstep_{t_name}_00{v}"
            duration = 0.13
            num_samples = int(SAMPLE_RATE * duration)
            samples = []

            for i in range(num_samples):
                t = i / float(SAMPLE_RATE)
                env = math.exp(-t * (35.0 if t_name != "water" else 25.0))

                if t_name == "grass":
                    noise = white_noise() * 0.45
                    thump = sine(85.0 + v * 5.0, t) * 0.55
                elif t_name == "dirt":
                    noise = white_noise() * 0.5
                    thump = sine(105.0 + v * 8.0, t) * 0.5
                elif t_name == "stone":
                    noise = white_noise() * 0.3
                    thump = sine(270.0 + v * 15.0, t) * 0.7
                elif t_name == "snow":
                    noise = white_noise() * 0.75
                    thump = sine(65.0 + v * 4.0, t) * 0.25
                elif t_name == "sand":
                    noise = white_noise() * 0.6
                    thump = sine(75.0 + v * 6.0, t) * 0.4
                elif t_name == "mud":
                    squish = sine(60.0 + 40.0 * math.sin(t * 120.0), t) * 0.65
                    noise = white_noise() * 0.35
                    thump = squish
                elif t_name == "wood":
                    # Hollow resonant wooden tap
                    wood_thud = sine(180.0 + v * 10.0, t) * math.exp(-t * 40.0) * 0.6
                    clack = triangle(340.0, t) * math.exp(-t * 60.0) * 0.4
                    noise = white_noise() * 0.15
                    thump = wood_thud + clack
                else: # water
                    # Liquid splash & drip
                    splash = sine(220.0 + 80.0 * math.sin(t * 80.0), t) * math.exp(-t * 20.0) * 0.6
                    noise = white_noise() * 0.4
                    thump = splash

                samples.append((thump + noise) * env * 0.65)

            samples = lowpass_filter(samples, cutoff=0.55 if t_name != "stone" else 0.8)
            wav_path = os.path.join(out_dir, f"{sfx_id}.wav")
            export_wav(wav_path, samples, channels=1)

            results.append({
                "id": sfx_id,
                "title": f"Footstep {t_name.capitalize()} {v}",
                "category": "player",
                "file": f"audio/sfx/player/{sfx_id}.wav",
                "duration": duration,
                "sample_rate": SAMPLE_RATE,
                "channels": 1,
                "bit_depth": 16,
                "looping": False,
                "bus": "sfx",
                "default_volume": 0.55,
                "description": f"Player footstep cadence on {t_name} variation {v}",
                "seed": seed
            })

    return results

