# Spatial Audio Specification

## 1. Overview & Coordinate Systems

Wildbound's 2D spatial audio engine calculates distance attenuation and stereo positioning for all world-space sound events relative to the player's world position.

```
                  -Y (North)
                      |
                      |   Source (xs, ys)
                      |    *
                      |   /
   -X (West) ---------L--------- +X (East)
                   Listener
                 (Player Pos)
                      |
                      |
                  +Y (South)
```

---

## 2. Mathematical Model

### 2.1 Distance Attenuation
Let $\mathbf{p}_{listener} = (x_l, y_l)$ and $\mathbf{p}_{source} = (x_s, y_s)$ in 2D world coordinates.

$$\Delta x = x_s - x_l, \quad \Delta y = y_s - y_l$$
$$d = \sqrt{\Delta x^2 + \Delta y^2}$$

If $d \ge d_{max}$, the voice is culled ($V = 0.0$). Otherwise, quadratic falloff models natural inverse-square acoustic attenuation:

$$A_{dist} = \left( 1.0 - \frac{d}{d_{max}} \right)^2$$

### 2.2 Stereo Panning
Horizontal offset $\Delta x$ determines the sound balance between left and right speakers. Panning horizon $D_{pan} = 400.0\text{ units}$:

$$p = \text{clamp}\left( \frac{\Delta x}{400.0}, -1.0, 1.0 \right)$$
$$P_{stereo} = 0.5 + 0.5 \times p$$

- When source is directly in line or at listener ($p = 0.0$), $P_{stereo} = 0.5$ (dead center).
- When source is $400\text{ px}$ to the left ($p = -1.0$), $P_{stereo} = 0.0$ (100% Left channel).
- When source is $400\text{ px}$ to the right ($p = +1.0$), $P_{stereo} = 1.0$ (100% Right channel).

---

## 3. raylib Hardware Integration

`AudioSystem::PlaySoundSpatial` maps calculated values directly into raylib's sound API:

```cpp
float busVol = GetEffectiveVolume(info->bus);
float finalVol = std::clamp(busVol * info->defaultVolume * volumeScale * distanceAttenuation, 0.0f, 1.0f);

::SetSoundVolume(it->second, finalVol);
::SetSoundPan(it->second, raylibPan);
if (pitchScale != 1.0f) {
    ::SetSoundPitch(it->second, pitchScale);
}
::PlaySound(it->second);
```

### Benefits:
- Zero external commercial middleware required (operates directly over miniaudio/WASAPI).
- Minimal per-frame CPU overhead ($O(1)$ scalar square root and clamp).
- Culls sounds beyond $d_{max}$ before loading or playing them, preserving audio mixer voice channels.

