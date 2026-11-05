import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle, sawtooth, square
from effects.noise import white_noise, PinkNoiseGenerator, BrownNoiseGenerator
from effects.filters import lowpass_filter, bandpass_filter

def generate_environmental(out_dir: str = "assets/audio/ambience") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)
    sfx_env_dir = "assets/audio/sfx/environment"
    os.makedirs(sfx_env_dir, exist_ok=True)

    ambience_loops = [
        ("amb_forest_wind", "Forest Canopy Breeze", 8.0, 6001),
        ("amb_forest_birds", "Forest Songbirds", 8.0, 6002),
        ("amb_forest_river", "Forest Flowing River", 8.0, 6003),
        ("amb_waterfall_heavy", "Cascading Torrential Waterfall", 8.0, 6004),
        ("amb_desert_wind", "Desert Sandstorm Gusts", 8.0, 6005),
        ("amb_snow_blizzard", "Arctic Snow Blizzard", 8.0, 6006),
        ("amb_swamp_insects", "Swamp Night Insects & Frogs", 8.0, 6007),
        ("amb_mountain_wind", "Mountain Peak Gale & Echo", 8.0, 6008),
        ("amb_indoor_fireplace", "Village Tavern Fireplace", 8.0, 6009),
        ("amb_village_market", "Village Bustling Town Atmosphere", 8.0, 6010),
        ("amb_village_blacksmith", "Village Blacksmith Forge & Anvil", 8.0, 6011)
    ]

    results = []

    for amb_id, title, duration, seed in ambience_loops:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = [0.0] * num_samples
        pink = PinkNoiseGenerator()
        brown = BrownNoiseGenerator()

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)

            if "birds" in amb_id:
                if (int(t * 2) % 3 == 0) and (t % 0.5 < 0.15):
                    chirp_f = 2400.0 + 800.0 * math.sin(t * 40.0)
                    samples[i] += sine(chirp_f, t) * 0.35
                samples[i] += pink.sample() * 0.05
            elif "insects" in amb_id:
                # Crickets & swamp frog ribbits
                cricket = square(4500.0, t) * (1.0 if math.sin(t * 30.0) > 0.0 else 0.0)
                frog = sawtooth(120.0, t) * (1.0 if (int(t * 1.5) % 4 == 0 and t % 0.4 < 0.2) else 0.0) * 0.4
                samples[i] += cricket * 0.12 + frog + pink.sample() * 0.08
            elif "river" in amb_id:
                # Steady bubbling white/pink water noise with slow modulation
                water_flow = pink.sample() * (0.6 + 0.15 * math.sin(t * 3.0))
                bubble = sine(380.0 + 80.0 * math.sin(t * 20.0), t) * 0.15
                samples[i] += water_flow * 0.6 + bubble
            elif "waterfall" in amb_id:
                # Heavy brown noise roar with powerful low frequencies
                roar = brown.sample() * 0.75 + pink.sample() * 0.3
                samples[i] += roar * 0.8
            elif "blacksmith" in amb_id:
                # Forge fire drone + rhythmic anvil clink
                fire = brown.sample() * 0.35
                anvil = 0.0
                if t % 2.0 < 0.15:
                    t_hit = t % 2.0
                    anvil = (sine(1850.0, t_hit) + sine(3200.0, t_hit) * 0.5) * math.exp(-t_hit * 30.0) * 0.7
                samples[i] += fire + anvil
            elif "market" in amb_id:
                # Warm crowd room tone + distant clatter
                rumble = brown.sample() * (0.3 + 0.1 * math.sin(t * 1.5))
                chatter = pink.sample() * 0.15
                samples[i] += rumble + chatter
            elif "fireplace" in amb_id:
                crackle = white_noise() * (1.0 if random.random() < 0.02 else 0.05)
                drone = sine(65.0, t) * 0.2
                samples[i] += crackle * 0.4 + drone
            else:
                # Wind gust
                mod = 0.5 + 0.5 * math.sin(t * 0.8)
                wind = brown.sample() * mod * 0.55 + pink.sample() * 0.15
                samples[i] += wind

        samples = lowpass_filter(samples, cutoff=0.35)
        # Crossfade loop boundaries
        fade_samples = int(0.2 * SAMPLE_RATE)
        for i in range(fade_samples):
            f_in = i / float(fade_samples)
            f_out = 1.0 - f_in
            samples[i] = samples[i] * f_in + samples[num_samples - fade_samples + i] * f_out

        wav_path = os.path.join(out_dir, f"{amb_id}.wav")
        export_wav(wav_path, samples, channels=2)

        results.append({
            "id": amb_id,
            "title": title,
            "category": "ambience",
            "file": f"audio/ambience/{amb_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 2,
            "bit_depth": 16,
            "looping": True,
            "bus": "environment",
            "default_volume": 0.50,
            "description": f"Environmental ambience loop: {title}",
            "seed": seed
        })

    # One-shot environmental sounds
    one_shots = [
        ("sfx_env_desert_sand_shift", "Desert Shifting Sand Gust", 0.65, 6101),
        ("sfx_env_ice_crack", "Glacial Ice Fracture Snap", 0.45, 6102),
        ("sfx_env_swamp_bubble", "Swamp Marsh Gas Bubble Pop", 0.25, 6103),
        ("sfx_env_mountain_rockfall", "Mountain Distant Rockfall Rumbling", 0.85, 6104),
        ("sfx_env_wood_creak", "Timber Structure Stress Creak", 0.40, 6105)
    ]

    for sfx_id, title, duration, seed in one_shots:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)
            if "ice_crack" in sfx_id:
                snap = sine(2200.0, t) * math.exp(-t * 25.0) + white_noise() * math.exp(-t * 30.0) * 0.8
                val = snap
            elif "swamp_bubble" in sfx_id:
                pop = sine(180.0 + 350.0 * (t / duration), t) * math.exp(-t * 18.0)
                val = pop * 0.85
            elif "rockfall" in sfx_id:
                rumble = sine(50.0, t) * math.exp(-t * 2.0) + white_noise() * math.exp(-t * 3.5) * 0.6
                val = rumble
            elif "wood_creak" in sfx_id:
                creak = sawtooth(90.0 + 30.0 * math.sin(t * 20.0), t) * math.exp(-t * 3.0)
                val = creak * 0.7
            else: # sand shift
                val = white_noise() * math.sin(t * math.pi / duration) * 0.5

            samples.append(val * 0.7)

        samples = lowpass_filter(samples, cutoff=0.55)
        wav_path = os.path.join(sfx_env_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        results.append({
            "id": sfx_id,
            "title": title,
            "category": "environment",
            "file": f"audio/sfx/environment/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": "environment",
            "default_volume": 0.60,
            "description": f"Environmental sound: {title}",
            "seed": seed
        })

    return results

