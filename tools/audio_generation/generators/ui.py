import os
import random
from typing import List, Dict, Any
from effects.mixing import export_wav, SAMPLE_RATE
from effects.oscillators import chirp, sine, triangle, square
from effects.envelopes import adsr

def generate_ui_sfx(out_dir: str = "assets/audio/sfx/ui") -> List[Dict[str, Any]]:
    os.makedirs(out_dir, exist_ok=True)

    ui_sounds = [
        ("sfx_ui_hover", "Button Hover", 0.05, 1200.0, 1800.0, "sine", 0.30, 2001),
        ("sfx_ui_click", "Button Click", 0.08, 800.0, 400.0, "triangle", 0.60, 2002),
        ("sfx_ui_confirm", "Button Confirm", 0.18, 523.25, 1046.5, "sine", 0.70, 2003),
        ("sfx_ui_cancel", "Button Cancel", 0.14, 440.0, 220.0, "triangle", 0.50, 2004),
        ("sfx_ui_tab", "Tab Switch", 0.07, 900.0, 1100.0, "sine", 0.45, 2005),
        ("sfx_ui_panel_open", "Panel Open", 0.16, 350.0, 700.0, "sine", 0.50, 2006),
        ("sfx_ui_panel_close", "Panel Close", 0.14, 650.0, 300.0, "sine", 0.50, 2007),
        ("sfx_ui_item_select", "Item Select", 0.09, 750.0, 950.0, "triangle", 0.55, 2008),
        ("sfx_ui_cosmetic_apply", "Cosmetic Apply", 0.22, 600.0, 1200.0, "sine", 0.75, 2009),
        ("sfx_ui_cosmetic_remove", "Cosmetic Remove", 0.15, 800.0, 400.0, "sine", 0.55, 2010),
        ("sfx_ui_settings_adjust", "Settings Adjust", 0.06, 600.0, 750.0, "sine", 0.40, 2011),
        ("sfx_ui_error", "Error Notification", 0.25, 220.0, 160.0, "square", 0.65, 2012),
        ("sfx_ui_success", "Success Notification", 0.35, 659.25, 1318.5, "sine", 0.75, 2013),
        ("sfx_ui_back", "Back Navigation", 0.10, 550.0, 350.0, "sine", 0.45, 2014),
        ("sfx_ui_modal_confirm", "Modal Confirmation", 0.25, 440.0, 880.0, "sine", 0.70, 2015)
    ]

    results = []

    for sfx_id, title, duration, f_start, f_end, wave_type, vol, seed in ui_sounds:
        random.seed(seed)
        num_samples = int(SAMPLE_RATE * duration)
        samples = []

        for i in range(num_samples):
            t = i / float(SAMPLE_RATE)
            ratio = t / duration
            freq = f_start + ratio * (f_end - f_start)

            if wave_type == "sine":
                val = sine(freq, t)
            elif wave_type == "triangle":
                val = triangle(freq, t)
            elif wave_type == "square":
                val = square(freq, t)
            else:
                val = sine(freq, t)

            env = adsr(t, duration, a=0.005, d=0.03, s=0.4, r=0.03)
            samples.append(val * env * vol)

        wav_path = os.path.join(out_dir, f"{sfx_id}.wav")
        export_wav(wav_path, samples, channels=1)

        results.append({
            "id": sfx_id,
            "title": title,
            "category": "ui",
            "file": f"audio/sfx/ui/{sfx_id}.wav",
            "duration": duration,
            "sample_rate": SAMPLE_RATE,
            "channels": 1,
            "bit_depth": 16,
            "looping": False,
            "bus": "ui",
            "default_volume": vol,
            "description": f"UI sound: {title}",
            "seed": seed
        })

    return results

