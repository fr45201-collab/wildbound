import math
import struct
import wave
import random

SAMPLE_RATE = 44100

class Synth:
    @staticmethod
    def sine(freq, t):
        return math.sin(2.0 * math.pi * freq * t)

    @staticmethod
    def square(freq, t):
        return 1.0 if math.sin(2.0 * math.pi * freq * t) >= 0.0 else -1.0

    @staticmethod
    def triangle(freq, t):
        return 2.0 * abs(2.0 * (freq * t - math.floor(freq * t + 0.5))) - 1.0

    @staticmethod
    def sawtooth(freq, t):
        return 2.0 * (freq * t - math.floor(freq * t + 0.5))

    @staticmethod
    def noise():
        return random.uniform(-1.0, 1.0)

    @staticmethod
    def adsr(t, duration, a=0.02, d=0.08, s=0.7, r=0.1):
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

    @staticmethod
    def lowpass_filter(samples, cutoff=0.2):
        out = []
        last = 0.0
        for s in samples:
            val = last + cutoff * (s - last)
            out.append(val)
            last = val
        return out

    @staticmethod
    def apply_delay(samples, delay_samples=8820, decay=0.35):
        out = list(samples)
        for i in range(delay_samples, len(out)):
            out[i] += out[i - delay_samples] * decay
        return out

    @staticmethod
    def export_wav(filename, samples, sample_rate=SAMPLE_RATE, channels=1):
        # Normalize and clamp to prevent clipping
        max_val = max(abs(s) for s in samples) if samples else 1.0
        scale = 0.95 / max_val if max_val > 0.95 else 1.0

        with wave.open(filename, 'w') as wav:
            wav.setnchannels(channels)
            wav.setsampwidth(2) # 16-bit
            wav.setframerate(sample_rate)

            raw_bytes = bytearray()
            for s in samples:
                val = max(-1.0, min(1.0, s * scale))
                int_sample = int(val * 32767.0)
                raw_bytes.extend(struct.pack('<h', int_sample))
                if channels == 2:
                    # Stereo duplicate
                    raw_bytes.extend(struct.pack('<h', int_sample))

            wav.writeframes(raw_bytes)

