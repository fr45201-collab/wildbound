import os
import wave
import struct
import math
from typing import Tuple

def validate_wav_file(path: str) -> Tuple[bool, str]:
    if not os.path.exists(path):
        return False, "File does not exist"

    try:
        with wave.open(path, 'r') as w:
            n_channels = w.getnchannels()
            sampwidth = w.getsampwidth()
            framerate = w.getframerate()
            n_frames = w.getnframes()
            duration = n_frames / float(framerate)

            if sampwidth != 2:
                return False, f"Expected 16-bit PCM (sampwidth=2), got {sampwidth}"
            if framerate != 44100:
                return False, f"Expected 44100 Hz, got {framerate}"
            if n_frames == 0 or duration == 0.0:
                return False, "Empty audio file (0 frames)"

            raw_data = w.readframes(n_frames)
            total_samples = n_frames * n_channels
            samples = struct.unpack(f"<{total_samples}h", raw_data)

            # Check peak and RMS
            peak = max(abs(s) for s in samples)
            rms = math.sqrt(sum(s * s for s in samples) / float(total_samples))

            if peak == 0 or rms < 1.0:
                return False, f"Audio is silent (peak={peak}, rms={rms:.2f})"

            return True, f"Valid WAV: {duration:.2f}s, {n_channels}ch, peak={peak}/32767, RMS={rms:.1f}"
    except Exception as ex:
        return False, f"WAV read exception: {str(ex)}"

def validate_all_audio(audio_root: str = "assets/audio") -> bool:
    if not os.path.exists(audio_root):
        print(f"Audio directory '{audio_root}' missing!")
        return False

    valid_count = 0
    fail_count = 0

    for root, _, files in os.walk(audio_root):
        for f in files:
            if f.endswith(".wav"):
                full_path = os.path.join(root, f)
                ok, msg = validate_wav_file(full_path)
                if ok:
                    valid_count += 1
                else:
                    print(f"[FAIL] {full_path}: {msg}")
                    fail_count += 1

    print(f"\nAudio Validation Complete: {valid_count} passed, {fail_count} failed.")
    return fail_count == 0

if __name__ == "__main__":
    validate_all_audio()

