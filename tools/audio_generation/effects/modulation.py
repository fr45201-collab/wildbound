import math
from typing import List

def amplitude_modulation(samples: List[float], mod_freq: float, sample_rate: int = 44100, depth: float = 0.5) -> List[float]:
    out = []
    for i, s in enumerate(samples):
        t = i / float(sample_rate)
        mod = 1.0 - depth + depth * math.sin(2.0 * math.pi * mod_freq * t)
        out.append(s * mod)
    return out

def frequency_modulation(carrier_freq: float, mod_freq: float, mod_index: float, t: float) -> float:
    mod = mod_index * math.sin(2.0 * math.pi * mod_freq * t)
    return math.sin(2.0 * math.pi * carrier_freq * t + mod)

