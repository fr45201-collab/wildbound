import os
import wave
import struct
import math

def check_loop_continuity(path: str, tolerance_frames: int = 441) -> bool:
    """Checks whether the beginning and end of a looping audio file have matching amplitudes."""
    with wave.open(path, 'r') as w:
        n_channels = w.getnchannels()
        n_frames = w.getnframes()
        if n_frames < tolerance_frames * 2:
            return False

        w.setpos(0)
        start_raw = w.readframes(tolerance_frames)
        start_samples = struct.unpack(f"<{tolerance_frames * n_channels}h", start_raw)

        w.setpos(n_frames - tolerance_frames)
        end_raw = w.readframes(tolerance_frames)
        end_samples = struct.unpack(f"<{tolerance_frames * n_channels}h", end_raw)

        start_rms = math.sqrt(sum(s * s for s in start_samples) / float(len(start_samples)))
        end_rms = math.sqrt(sum(s * s for s in end_samples) / float(len(end_samples)))

        diff_db = abs(20.0 * math.log10(max(1.0, start_rms) / max(1.0, end_rms)))
        return diff_db < 6.0 # Within 6 dB matching energy

if __name__ == "__main__":
    loop_dirs = ["assets/audio/ambience", "assets/audio/music"]
    for d in loop_dirs:
        if os.path.exists(d):
            for f in os.listdir(d):
                if f.endswith(".wav"):
                    p = os.path.join(d, f)
                    ok = check_loop_continuity(p)
                    status = "OK" if ok else "WARN"
                    print(f"[{status}] Loop Continuity: {p}")

