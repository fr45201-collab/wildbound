import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle, sawtooth
from effects.noise import white_noise
from effects.filters import lowpass_filter

def generate_weapons(out_dir: str = "assets/audio/sfx/weapons") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)

    weapons_data = [
        ("sfx_sword_swing_001", "Sword Swing Light", 0.18, 4001),
        ("sfx_sword_swing_002", "Sword Swing Heavy", 0.22, 4002),
        ("sfx_sword_impact_metal", "Sword Metallic Clash", 0.25, 4003),
        ("sfx_sword_impact_wood", "Sword Wood Impact", 0.20, 4004),
        ("sfx_sword_impact_stone", "Sword Stone Deflect", 0.22, 4005),
        ("sfx_sword_sheathe", "Sword Handling Sheathe", 0.30, 4006),
        ("sfx_bow_equip", "Bow Handling Equip", 0.25, 4007),
        ("sfx_bow_draw", "Bowstring Tension Draw", 0.45, 4008),
        ("sfx_bow_release", "Bowstring Twang Release", 0.16, 4009),
        ("sfx_arrow_flight", "Arrow Whistle Flight", 0.35, 4010),
        ("sfx_arrow_impact", "Arrow Thud Impact General", 0.18, 4011),
        ("sfx_arrow_impact_flesh", "Arrow Flesh Penetration", 0.20, 4012),
        ("sfx_arrow_impact_wood", "Arrow Wood Embed Thump", 0.22, 4013),
        ("sfx_arrow_impact_stone", "Arrow Stone Ricochet Crack", 0.18, 4014),
        ("sfx_shield_block", "Shield Solid Block", 0.24, 4015),
        ("sfx_parry_clink", "Weapon Defensive Parry", 0.26, 4016)
    ]

    results = []

    for sfx_id, title, duration, seed in weapons_data:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)
            if "swing" in sfx_id:
                # Whoosh sweep
                noise = white_noise() * (0.8 - t * 2.0)
                freq = 200.0 + 400.0 * math.sin(t * math.pi / duration)
                tone = sine(freq, t) * 0.4
                val = (noise + tone) * math.sin(t * math.pi / duration)
            elif "metal" in sfx_id or "parry" in sfx_id:
                # High pitched metallic clash ring
                f1 = 1480.0 if "parry" not in sfx_id else 1850.0
                f2 = 2120.0 if "parry" not in sfx_id else 2640.0
                ring = sine(f1, t) * math.exp(-t * 12.0) + sine(f2, t) * math.exp(-t * 18.0)
                clack = white_noise() * math.exp(-t * 60.0) * 0.8
                val = ring * 0.6 + clack
            elif "bow_draw" in sfx_id:
                # Wood creak + string tension
                creak = sawtooth(70.0 + 30.0 * math.sin(t * 15.0), t) * math.exp(-t * 2.0)
                val = creak * 0.5 + white_noise() * 0.15
            elif "bow_release" in sfx_id:
                # Fast twang
                twang = triangle(190.0 * math.exp(-t * 8.0), t) * math.exp(-t * 15.0)
                val = twang * 0.85
            elif "bow_equip" in sfx_id:
                # Leather rustle + light wood flex
                val = white_noise() * math.exp(-t * 12.0) * 0.4 + sine(120.0, t) * math.exp(-t * 18.0) * 0.4
            elif "arrow_impact_flesh" in sfx_id:
                # Damp squish thump
                thud = sine(95.0, t) * math.exp(-t * 22.0) + white_noise() * math.exp(-t * 30.0) * 0.6
                val = thud * 0.8
            elif "arrow_impact_wood" in sfx_id or "wood" in sfx_id:
                # Crisp wood crack and thud
                crack = triangle(280.0, t) * math.exp(-t * 35.0) + white_noise() * math.exp(-t * 40.0) * 0.4
                val = crack * 0.8
            elif "arrow_impact_stone" in sfx_id or "stone" in sfx_id:
                # Sharp flint snap
                snap = sine(1200.0, t) * math.exp(-t * 45.0) + white_noise() * math.exp(-t * 50.0) * 0.7
                val = snap * 0.75
            elif "shield_block" in sfx_id:
                # Deep resonant deflection thud
                heavy_thud = sine(80.0, t) * math.exp(-t * 14.0) * 0.7 + triangle(220.0, t) * math.exp(-t * 25.0) * 0.4
                val = heavy_thud + white_noise() * math.exp(-t * 30.0) * 0.3
            else: # arrow flight / general impact
                val = (sine(120.0, t) + white_noise() * 0.5) * math.exp(-t * 20.0)

            samples.append(val * 0.75)

        samples = lowpass_filter(samples, cutoff=0.7)
        wav_path = os.path.join(out_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        results.append({
            "id": sfx_id,
            "title": title,
            "category": "weapons",
            "file": f"audio/sfx/weapons/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": "sfx",
            "default_volume": 0.72,
            "description": f"Weapon sound effect: {title}",
            "seed": seed
        })

    return results

