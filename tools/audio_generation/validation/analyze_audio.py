import os
import wave
import struct
import math
from typing import Dict, Any

def analyze_wav(path: str) -> Dict[str, Any]:
    with wave.open(path, 'r') as w:
        n_channels = w.getnchannels()
        framerate = w.getframerate()
        n_frames = w.getnframes()
        duration = n_frames / float(framerate)

        raw = w.readframes(n_frames)
        total_samples = n_frames * n_channels
        samples = struct.unpack(f"<{total_samples}h", raw)

        peak = max(abs(s) for s in samples)
        rms = math.sqrt(sum(s * s for s in samples) / float(total_samples))
        peak_db = 20.0 * math.log10(peak / 32767.0) if peak > 0 else -99.0
        rms_db = 20.0 * math.log10(rms / 32767.0) if rms > 0 else -99.0

        return {
            "path": path,
            "duration": round(duration, 3),
            "channels": n_channels,
            "framerate": framerate,
            "peak": peak,
            "peak_db": round(peak_db, 2),
            "rms": round(rms, 1),
            "rms_db": round(rms_db, 2)
        }

if __name__ == "__main__":
    import sys
    target = sys.argv[1] if len(sys.argv) > 1 else "assets/audio"
    if os.path.isfile(target):
        print(analyze_wav(target))
    else:
        for root, _, files in os.walk(target):
            for f in files:
                if f.endswith(".wav"):
                    p = os.path.join(root, f)
                    res = analyze_wav(p)
                    print(f"{res['path']:<45} {res['duration']:>6}s | Peak: {res['peak_db']:>6.1f} dB | RMS: {res['rms_db']:>6.1f} dB")

