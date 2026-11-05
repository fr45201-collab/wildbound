from typing import List

def apply_delay(samples: List[float], delay_samples: int = 8820, decay: float = 0.35) -> List[float]:
    out = list(samples)
    for i in range(delay_samples, len(out)):
        out[i] += out[i - delay_samples] * decay
    return out

def simple_reverb(samples: List[float], delays: List[int] = [1323, 2205, 3087, 4410], decay: float = 0.2) -> List[float]:
    out = list(samples)
    for d in delays:
        for i in range(d, len(out)):
            out[i] += out[i - d] * decay
    return out

