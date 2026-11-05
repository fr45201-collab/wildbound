import os
import math
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import sine, triangle, sawtooth, square
from effects.noise import white_noise
from effects.filters import lowpass_filter

def generate_creatures(out_dir: str = "assets/audio/sfx/creatures") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)

    creature_events = [
        # Goblin
        ("sfx_creature_goblin_idle", "Goblin Throaty Cackle", 0.40, 5101),
        ("sfx_creature_goblin_step", "Goblin Scuffling Step", 0.12, 5102),
        ("sfx_creature_goblin_attack", "Goblin Club Swing Whoosh", 0.22, 5103),
        ("sfx_creature_goblin_hurt", "Goblin Squealing Yelp", 0.25, 5104),
        ("sfx_creature_goblin_death", "Goblin Defeat Shriek", 0.55, 5105),

        # Skeleton
        ("sfx_creature_skeleton_step", "Skeleton Bone Rattle Step", 0.14, 5201),
        ("sfx_creature_skeleton_attack", "Skeleton Bone Thrust", 0.24, 5202),
        ("sfx_creature_skeleton_hurt", "Skeleton Bone Crack", 0.20, 5203),
        ("sfx_creature_skeleton_death", "Skeleton Bone Collapse Pile", 0.60, 5204),

        # Wolf
        ("sfx_creature_wolf_idle", "Wolf Low Throaty Snarl", 0.50, 5301),
        ("sfx_creature_wolf_growl", "Wolf Threatening Growl", 0.65, 5302),
        ("sfx_creature_wolf_attack", "Wolf Rapid Snap Bite", 0.25, 5303),
        ("sfx_creature_wolf_hurt", "Wolf Pain Yelp", 0.28, 5304),
        ("sfx_creature_wolf_death", "Wolf Mournful Whimper", 0.70, 5305),

        # Wild Boar
        ("sfx_creature_boar_idle", "Wild Boar Snort Grunt", 0.45, 5401),
        ("sfx_creature_boar_step", "Wild Boar Heavy Hoof Stamp", 0.15, 5402),
        ("sfx_creature_boar_attack", "Wild Boar Tusk Gore Charge", 0.35, 5403),
        ("sfx_creature_boar_hurt", "Wild Boar Piercing Squeal", 0.32, 5404),
        ("sfx_creature_boar_death", "Wild Boar Collapse Squeal", 0.65, 5405),

        # Bat
        ("sfx_creature_bat_flight", "Bat Leathery Wing Flutter", 0.25, 5501),
        ("sfx_creature_bat_attack", "Bat High Ultrasonic Dive", 0.22, 5502),
        ("sfx_creature_bat_hurt", "Bat Shrill Screech", 0.18, 5503),
        ("sfx_creature_bat_death", "Bat Soft Impact Drop", 0.35, 5504),

        # Wildlife
        ("sfx_wildlife_bird_takeoff", "Bird Wing Flutter Takeoff", 0.35, 5601),
        ("sfx_wildlife_bird_landing", "Bird Gentle Branch Landing", 0.20, 5602),
        ("sfx_wildlife_butterfly_flutter", "Butterfly Delicate Wing Murmur", 0.25, 5603)
    ]

    results = []

    for sfx_id, title, duration, seed in creature_events:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)

            if "goblin" in sfx_id:
                # Raspy vocal cords + frequency wobble
                wobble = 280.0 + 90.0 * math.sin(t * 45.0)
                tone = sawtooth(wobble, t) * 0.6 + square(wobble * 0.5, t) * 0.3
                noise = white_noise() * 0.25
                env = math.exp(-t * (8.0 if "death" in sfx_id else 14.0))
                val = (tone + noise) * env
            elif "skeleton" in sfx_id:
                # Dry hollow clicking & rattling bones
                click_f = 1200.0 if (int(t * 30) % 2 == 0) else 1800.0
                click = sine(click_f, t) * math.exp(-((t * 40) % 1.0) * 20.0) * 0.7
                rattle = white_noise() * math.exp(-t * 8.0) * 0.4
                val = click + rattle
            elif "wolf" in sfx_id:
                # Deep guttural growl or high yelp
                if "hurt" in sfx_id or "death" in sfx_id:
                    yelp = sine(650.0 - t * 300.0, t) * 0.7 + sawtooth(320.0, t) * 0.3
                    val = yelp * math.exp(-t * 6.0)
                else:
                    rumble = sawtooth(75.0 + 20.0 * math.sin(t * 25.0), t) * 0.7
                    val = (rumble + white_noise() * 0.2) * (0.8 + 0.2 * math.sin(t * 15.0))
            elif "boar" in sfx_id:
                # Heavy snort or high pig squeal
                if "hurt" in sfx_id or "death" in sfx_id:
                    squeal = sawtooth(1100.0 - t * 400.0, t) * 0.8 + square(550.0, t) * 0.2
                    val = squeal * math.exp(-t * 5.0)
                else:
                    grunt = triangle(80.0, t) * math.exp(-((t * 6) % 1.0) * 10.0) * 0.8
                    val = grunt + white_noise() * 0.2
            elif "bat" in sfx_id:
                # Ultrasonic high sweep or rapid flutter
                if "flight" in sfx_id:
                    flap = white_noise() * (0.5 + 0.5 * math.sin(t * 60.0)) * math.exp(-t * 4.0)
                    val = flap
                else:
                    shriek = sine(3200.0 - t * 1200.0, t) * math.exp(-t * 12.0)
                    val = shriek
            else:
                # Wildlife flutter
                flap = white_noise() * (0.5 + 0.5 * math.sin(t * 35.0)) * math.exp(-t * 7.0)
                val = flap * 0.5

            samples.append(val * 0.75)

        samples = lowpass_filter(samples, cutoff=0.7)
        wav_path = os.path.join(out_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        results.append({
            "id": sfx_id,
            "title": title,
            "category": "creatures",
            "file": f"audio/sfx/creatures/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": "creatures",
            "default_volume": 0.68,
            "description": f"Creature audio vocalization: {title}",
            "seed": seed
        })

    return results

