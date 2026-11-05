import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle, sawtooth
from effects.noise import white_noise
from effects.filters import lowpass_filter

def generate_combat(out_dir: str = "assets/audio/sfx/player") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)
    events_dir = "assets/audio/sfx/events"
    os.makedirs(events_dir, exist_ok=True)

    combat_sfx = [
        ("sfx_player_hurt", "Player Hurt Vocalization", 0.28, "player", 0.70, 3101),
        ("sfx_player_death", "Player Defeat Cry", 0.65, "player", 0.75, 3102),
        ("sfx_enemy_hurt", "Enemy Generic Hurt Grunt", 0.22, "combat", 0.65, 3103),
        ("sfx_enemy_death", "Enemy Generic Death Fall", 0.50, "combat", 0.70, 3104),
        ("sfx_impact_flesh_001", "Melee Hit Flesh Light", 0.16, "combat", 0.75, 3105),
        ("sfx_impact_flesh_002", "Melee Hit Flesh Heavy", 0.22, "combat", 0.80, 3106),
        ("sfx_combat_critical_hit", "Critical Strike Resonance", 0.38, "combat", 0.85, 3107)
    ]

    results = []

    for sfx_id, title, duration, category, vol, seed in combat_sfx:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        target_dir = out_dir if category == "player" else "assets/audio/sfx/combat"
        os.makedirs(target_dir, exist_ok=True)

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)

            if sfx_id == "sfx_player_hurt":
                f = 190.0 - t * 80.0
                val = sawtooth(max(40.0, f), t) * 0.6 + white_noise() * 0.25
                env = math.exp(-t * 11.0)
            elif sfx_id == "sfx_player_death":
                f = 160.0 - t * 130.0
                val = sawtooth(max(30.0, f), t) * 0.65 + white_noise() * 0.3
                env = math.exp(-t * 3.8)
            elif sfx_id == "sfx_enemy_hurt":
                f = 120.0 - t * 50.0
                val = triangle(max(40.0, f), t) * 0.7 + white_noise() * 0.3
                env = math.exp(-t * 14.0)
            elif sfx_id == "sfx_enemy_death":
                f = 110.0 - t * 70.0
                thud = sine(55.0, t) * math.exp(-t * 6.0) * 0.6
                val = triangle(max(30.0, f), t) * 0.5 * math.exp(-t * 7.0) + thud
                env = 1.0
            elif "impact_flesh" in sfx_id:
                # Heavy wet punch thump
                base = 90.0 if "001" in sfx_id else 70.0
                thump = sine(base, t) * math.exp(-t * 22.0) * 0.7
                crack = white_noise() * math.exp(-t * 35.0) * 0.5
                val = thump + crack
                env = 1.0
            elif sfx_id == "sfx_combat_critical_hit":
                # Stinger shimmer + deep bass strike
                chime = sine(880.0, t) * math.exp(-t * 8.0) + sine(1320.0, t) * math.exp(-t * 10.0)
                sub = sine(65.0, t) * math.exp(-t * 12.0)
                val = chime * 0.5 + sub * 0.7 + white_noise() * math.exp(-t * 40.0) * 0.3
                env = 1.0
            else:
                val = white_noise() * math.exp(-t * 20.0)
                env = 1.0

            samples.append(val * env * vol)

        samples = lowpass_filter(samples, cutoff=0.65)
        wav_path = os.path.join(target_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        rel_dir = "audio/sfx/player" if category == "player" else "audio/sfx/combat"
        bus_name = "sfx" if category == "player" else "combat"

        results.append({
            "id": sfx_id,
            "title": title,
            "category": category,
            "file": f"{rel_dir}/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": bus_name,
            "default_volume": vol,
            "description": f"Combat audio: {title}",
            "seed": seed
        })

    return results

