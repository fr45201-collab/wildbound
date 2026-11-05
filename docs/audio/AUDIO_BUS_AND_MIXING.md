# Audio Bus & Mixing Matrix Specification

## 1. Overview & Objectives

Wildbound implements an 8-channel hierarchical mixing architecture designed to guarantee clear audio intelligibility, balanced dynamic range, and player-customizable attenuation across different sound categories.

The architecture ensures that gameplay-critical sounds (such as incoming enemy attacks and damage feedback) remain audible over heavy environmental ambience or high-intensity combat music.

---

## 2. Bus Hierarchy & Routing Matrix

```
                      +-------------------+
                      |    Master Bus     |  [Master Volume, Mute All]
                      +-------------------+
                                |
        +-----------+-----------+-----------+-----------+
        |           |           |           |           |
        v           v           v           v           v
   +---------+ +---------+ +---------+ +---------+ +---------+
   |  Music  | |   UI    | |   SFX   | |  Combat | |Creatures| ... [Environment, Voice]
   +---------+ +---------+ +---------+ +---------+ +---------+
        |           |           |           |           |
     Tracks      Buttons     Footsteps   Impacts     Goblin
     Stingers    Sliders     Pickups     Crits       Wolf
                 Panels      Doors       Hurt/Death  Skeleton
```

---

## 3. Bus Specifications & Default Balancing

| Bus | Default Gain | Dynamic Priority | Typical RMS Target | Headroom Allowance |
|---|---|---|---|---|
| **Master** | `0.80` ($-1.9\text{ dB}$) | Supreme | N/A | $-0.5\text{ dBFS}$ Peak |
| **Music** | `0.70` ($-3.1\text{ dB}$) | Low | $-18\text{ dBFS}$ | $-3.0\text{ dBFS}$ |
| **UI** | `0.80` ($-1.9\text{ dB}$) | High | $-14\text{ dBFS}$ | $-1.5\text{ dBFS}$ |
| **SFX** | `0.80` ($-1.9\text{ dB}$) | Medium | $-16\text{ dBFS}$ | $-1.5\text{ dBFS}$ |
| **Environment** | `0.70` ($-3.1\text{ dB}$) | Background | $-22\text{ dBFS}$ | $-4.0\text{ dBFS}$ |
| **Combat** | `0.80` ($-1.9\text{ dB}$) | Critical | $-12\text{ dBFS}$ | $-1.0\text{ dBFS}$ |
| **Creatures** | `0.75` ($-2.5\text{ dB}$) | High | $-15\text{ dBFS}$ | $-2.0\text{ dBFS}$ |
| **Voice** | `0.80` ($-1.9\text{ dB}$) | Reserved | $-12\text{ dBFS}$ | $-1.0\text{ dBFS}$ |

---

## 4. Volume Attenuation Algorithm

The effective gain $G_{out}$ applied to any audio buffer before feeding into raylib's device backend is computed via:

$$G_{out} = \text{muteAll} \,?\, 0.0 : \text{clamp}\Big( G_{master} \times G_{bus} \times G_{assetDefault} \times G_{scale} \times A_{spatial},\; 0.0,\; 1.0 \Big)$$

### Mathematical Guarantees:
1. **Safety Clamping:** All slider inputs and calculated products are strictly clamped between $0.0$ and $1.0$ to eliminate digital overflow distortion.
2. **Deterministic Muting:** Toggling `muteAll = true` forces $G_{out} = 0.0$ across all channels without clearing the player's custom slider levels.
3. **Linearity & Decibels:** Volume sliders operate linearly in amplitude $[0.0..1.0]$ for predictable, user-friendly UI adjustment.

