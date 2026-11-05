from typing import List

def lowpass_filter(samples: List[float], cutoff: float = 0.25) -> List[float]:
    out = []
    last = 0.0
    for s in samples:
        val = last + cutoff * (s - last)
        out.append(val)
        last = val
    return out

def highpass_filter(samples: List[float], cutoff: float = 0.1) -> List[float]:
    out = []
    last_in = 0.0
    last_out = 0.0
    for s in samples:
        val = cutoff * (last_out + s - last_in)
        out.append(val)
        last_in = s
        last_out = val
    return out

def bandpass_filter(samples: List[float], low_cutoff: float = 0.15, high_cutoff: float = 0.6) -> List[float]:
    hp = highpass_filter(samples, low_cutoff)
    return lowpass_filter(hp, high_cutoff)

