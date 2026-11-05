import math
import random

def sine(freq: float, t: float) -> float:
    return math.sin(2.0 * math.pi * freq * t)

def square(freq: float, t: float, duty: float = 0.5) -> float:
    phase = (freq * t) % 1.0
    return 1.0 if phase < duty else -1.0

def triangle(freq: float, t: float) -> float:
    return 2.0 * abs(2.0 * (freq * t - math.floor(freq * t + 0.5))) - 1.0

def sawtooth(freq: float, t: float) -> float:
    return 2.0 * (freq * t - math.floor(freq * t + 0.5))

def chirp(f_start: float, f_end: float, t: float, duration: float) -> float:
    ratio = t / duration if duration > 0 else 0.0
    f = f_start + ratio * (f_end - f_start)
    return math.sin(2.0 * math.pi * f * t)

