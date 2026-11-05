# Storm Audio Integration Specification

## 1. Soundscapes and Event Mapping

The Corruption Storm integrates with the existing `wildbound::audio::AudioSystem` and `AudioManager`, dispatching dynamic sound cues based on game and entity state.

```
+-------------------------------------------------------------------------------+
| State Event               | Asset ID                | Loop? | Volume Modulation|
+---------------------------+-------------------------+-------+-----------------+
| Phase Warning Trigger     | sfx_storm_warning       | No    | 0.90            |
| 15s Countdown Warning     | sfx_storm_warning       | No    | 1.00 (pitch up) |
| Phase Contraction Start   | sfx_storm_shrink        | No    | 0.85            |
| Entity Outside Safe Zone  | amb_storm_rumble        | Yes   | Dist-scaled     |
| Continuous Storm Damage   | sfx_storm_damage        | Yes   | 0.70            |
+-------------------------------------------------------------------------------+
```

---

## 2. Dynamic Distance and Penetration Attenuation

### 2.1 Ambient Storm Rumble (`amb_storm_rumble`)
- Plays continuously whenever the player is outside or approaching within $200\text{px}$ of the boundary.
- Volume is modulated dynamically:
  $$V_{\text{rumble}} = \text{clamp}\left(0.2 + \frac{d}{1000.0} \cdot 0.8, 0.0, 1.0\right)$$
  where $d$ is penetration depth into the storm.

### 2.2 Storm Damage Pulse (`sfx_storm_damage`)
- Triggered synchronously with damage application pulses from `StormDamageSystem`.
- High frequency crackling and flesh-burning sound effects reinforce the physical threat of corruption exposure.

---

## 3. Acoustic Fallback Guarantee

If audio hardware is uninitialized or in headless continuous integration mode:
- All sound dispatches gracefully fallback to silent execution without memory leaks, blocking calls, or null pointer dereferencing.
- Verified by unit tests running in headless CI environments.

