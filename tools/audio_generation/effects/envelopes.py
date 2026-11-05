import math

def adsr(t: float, duration: float, a: float = 0.02, d: float = 0.08, s: float = 0.7, r: float = 0.1) -> float:
    if t < a:
        return t / a if a > 0 else 1.0
    elif t < a + d:
        factor = (t - a) / d if d > 0 else 1.0
        return 1.0 - factor * (1.0 - s)
    elif t < duration - r:
        return s
    elif t < duration:
        factor = (duration - t) / r if r > 0 else 0.0
        return max(0.0, s * factor)
    return 0.0

def exp_decay(t: float, rate: float = 15.0) -> float:
    return math.exp(-t * rate)

def linear_fade(t: float, duration: float, fade_in: bool = True) -> float:
    if duration <= 0:
        return 1.0
    progress = max(0.0, min(1.0, t / duration))
    return progress if fade_in else (1.0 - progress)

