import random

def white_noise() -> float:
    return random.uniform(-1.0, 1.0)

class PinkNoiseGenerator:
    def __init__(self):
        self.b0 = 0.0
        self.b1 = 0.0
        self.b2 = 0.0
        self.b3 = 0.0
        self.b4 = 0.0
        self.b5 = 0.0
        self.b6 = 0.0

    def sample(self) -> float:
        white = white_noise()
        self.b0 = 0.99886 * self.b0 + white * 0.0555179
        self.b1 = 0.99332 * self.b1 + white * 0.0750759
        self.b2 = 0.96900 * self.b2 + white * 0.1538520
        self.b3 = 0.86650 * self.b3 + white * 0.3104856
        self.b4 = 0.55000 * self.b4 + white * 0.5329522
        self.b5 = -0.7616 * self.b5 - white * 0.0168980
        val = self.b0 + self.b1 + self.b2 + self.b3 + self.b4 + self.b5 + self.b6 + white * 0.5362
        self.b6 = white * 0.115926
        return max(-1.0, min(1.0, val * 0.11))

class BrownNoiseGenerator:
    def __init__(self):
        self.last = 0.0

    def sample(self) -> float:
        white = white_noise()
        self.last = (self.last + (0.02 * white)) / 1.02
        return max(-1.0, min(1.0, self.last * 3.5))

