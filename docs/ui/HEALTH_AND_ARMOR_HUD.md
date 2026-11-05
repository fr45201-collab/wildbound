# Health and Armor HUD Specification

## 1. Overview & Mechanics

The Vitals HUD (`HUDManager`) renders player vitality, damage reception feedback, and protective armor status in the Top-Left anchor. It incorporates standard competitive fighting game / battle royale damage visualization mechanics:
- **Instant Health Depletion**: When damaged, current HP drops instantly.
- **Delayed Catch-down Lag Bar**: The previous health level remains visible in amber/crimson for $0.5\text{s}$, then drains at a constant rate of $65\text{ HP/s}$.
- **Critical Low-Health Pulsing Alert**: When player HP drops below $25\%$ of maximum health, a pulsing vignette frames the screen.
- **Armor Mitigation Readout**: Displays the player's total armor defense rating alongside the calculated damage mitigation percentage.

---

## 2. Delayed Lag Bar Mathematics & Algorithm

```
Damage Event occurs: HP drops from 100 to 40
Frame 0:   Current HP = 40, Lag HP = 100, Timer = 0.50s
Frame 30:  Current HP = 40, Lag HP = 100, Timer = 0.00s (Expires)
Frame 60:  Current HP = 40, Lag HP = 67.5, Timer <= 0 (Draining at 65 HP/s)
Frame 85:  Current HP = 40, Lag HP = 40.0, Catch-down complete!
```

### Mathematical Formulation
Let:
- $H_{current}$ be the true health from `HealthComponent`.
- $H_{displayed}$ be the smoothed bar health.
- $H_{lag}$ be the delayed trailing damage bar.
- $T_{lag}$ be the catch-down delay timer ($0.5\text{s}$).
- $R_{drain} = 65.0\text{ HP/sec}$.

Upon receiving damage ($H_{current} < H_{displayed}$):
$$H_{displayed} \leftarrow H_{current}$$
$$T_{lag} \leftarrow 0.5\text{s}$$

During frame update with step $\Delta t$:
```cpp
if (m_lagTimer > 0.0f) {
    m_lagTimer -= dt;
    if (m_lagTimer < 0.0f) {
        float excessDt = -m_lagTimer;
        m_lagTimer = 0.0f;
        m_lagHealth = std::max(m_displayedHealth, m_lagHealth - (65.0f * excessDt));
    }
} else if (m_lagHealth > m_displayedHealth) {
    m_lagHealth = std::max(m_displayedHealth, m_lagHealth - (65.0f * dt));
}
```
If the player receives healing ($H_{current} > H_{displayed}$):
$$H_{displayed} \leftarrow H_{current}$$
$$H_{lag} \leftarrow H_{current}$$
$$T_{lag} \leftarrow 0.0\text{s}$$
Healing immediately updates both bars without lag, providing instant positive reinforcement.

---

## 3. Critical Low-Health Pulse Alert

When $H_{current} \le 0.25 \times H_{max}$:
1. A heartbeat oscillation is computed via sinusoidal frequency $\omega = 4.0\text{ rad/s}$:
   $$\alpha(t) = 0.35 + 0.35 \sin(4.0 \cdot t)$$
2. A pulsing red perimeter vignette (`Color{ 180, 20, 20, (unsigned char)(alpha * 255) }`) is rendered across the viewport perimeter.
3. The health bar fill color shifts from Healthy Green (`#2ECC71`) to Warning Orange (`#E67E22`) to Critical Crimson (`#E74C3C`).

---

## 4. Armor Defense Rating & Mitigation Calculation

Armor is displayed below the health bar with an iron shield icon and blue progress fill (`#3498DB`).

The damage mitigation formula implemented in Wildbound's combat system follows:
$$\text{Mitigation } \% = \left( 1.0 - \frac{100.0}{100.0 + \text{Armor}} \right) \times 100\%$$

### Mitigation Lookup Table
| Armor Points | Damage Reduction | Example (100 Damage Hit) |
|---|---|---|
| **0** | $0.0\%$ | Takes 100 dmg |
| **25** | $20.0\%$ | Takes 80 dmg |
| **50** | $33.3\%$ | Takes 66.7 dmg |
| **100** | $50.0\%$ | Takes 50 dmg |
| **150** | $60.0\%$ | Takes 40 dmg |

The UI displays both values:
```
[SHIELD ICON] Armor: 75 (Mitigation: 42.8%)
```
This gives players immediate feedback on their damage survivability.

