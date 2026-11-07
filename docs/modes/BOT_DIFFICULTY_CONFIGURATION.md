# WILDBOUND — BOT DIFFICULTY CONFIGURATION SPECIFICATION

## 1. Difficulty Profiles

Wildbound provides three distinct bot difficulty profiles loaded from `assets/data/gamemodes/bot_difficulty.json`:

| Behavioral Metric | Easy | Normal | Hard |
|---|---|---|---|
| **Perception Range** | $180\text{ px}$ | $280\text{ px}$ | $400\text{ px}$ |
| **Reaction Latency** | $0.80\text{ s}$ | $0.35\text{ s}$ | $0.12\text{ s}$ |
| **Attack Cooldown Multiplier** | $1.60\times$ (slower) | $1.00\times$ (standard) | $0.75\times$ (aggressive) |
| **Movement Speed Multiplier** | $0.85\times$ | $1.00\times$ | $1.15\times$ |
| **Storm Evacuate Urgency** | $12.0\text{ s}$ | $20.0\text{ s}$ | $30.0\text{ s}$ |
| **Retreat Health Threshold** | $15\%$ | $25\%$ | $35\%$ |
| **Aggression Weight** | $0.35$ | $0.65$ | $0.90$ |

---

## 2. Dynamic Behavioral Impact

- **Reaction Latency**: Easy bots exhibit a deliberate delay between target acquisition and attack execution, offering players reaction windows. Hard bots react almost instantaneously.
- **Aggression Weight**: Influences how aggressively bots pursue targets vs holding defensive ground.
- **Storm Avoidance**: Higher difficulty bots begin retreating toward the safe zone well before the corruption barrier encroaches.
- **Fairness Principle**: Bot difficulty modifies intelligence, timing, and perception rather than inflating enemy health or damage values artificially.

