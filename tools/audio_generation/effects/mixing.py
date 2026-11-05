import wave
import struct
from typing import List

SAMPLE_RATE = 44100

def normalize(samples: List[float], target_peak: float = 0.95) -> List[float]:
    if not samples:
        return samples
    peak = max(abs(s) for s in samples)
    if peak <= 0.00001:
        return samples
    scale = target_peak / peak if peak > target_peak or peak < 0.3 else 1.0
    return [max(-1.0, min(1.0, s * scale)) for s in samples]

def export_wav(filename: str, samples: List[float], sample_rate: int = SAMPLE_RATE, channels: int = 1):
    norm = normalize(samples)
    with wave.open(filename, 'w') as wav:
        wav.setnchannels(channels)
        wav.setsampwidth(2) # 16-bit
        wav.setframerate(sample_rate)

        raw_bytes = bytearray()
        for s in norm:
            val = max(-1.0, min(1.0, s))
            int_sample = int(val * 32767.0)
            raw_bytes.extend(struct.pack('<h', int_sample))
            if channels == 2:
                # Stereo duplicate
                raw_bytes.extend(struct.pack('<h', int_sample))

        wav.writeframes(raw_bytes)

