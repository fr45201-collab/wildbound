# Wildbound — Target Selection & Threat Evaluation

## 1. Overview & Evaluation Goals

In battle royale and fantasy multiplayer scenarios, multiple potential targets (the player, enemy bots, wildlife, or rival factions) may simultaneously occupy an agent's sensory range. Selecting simply the closest entity can lead to erratic target thrashing or ignoring high-threat damage dealers.

Wildbound utilizes a multi-factor threat scoring system in `wildbound::ai::TargetSelector` combined with stateful intel persistence in `wildbound::ai::TargetMemory`.

---

## 2. Threat Scoring Formula

For each perceived target candidate $k$, the selector computes a composite threat score $S_k$:

$$S_k = w_{\text{dist}} \cdot \left(1.0 - \frac{d_k}{R_{\max}}\right) + w_{\text{damage}} \cdot \min\left(1.0, \frac{\text{DamageRecent}_k}{\text{MaxHP}}\right) + w_{\text{lowhp}} \cdot \left(1.0 - \frac{\text{HP}_k}{\text{MaxHP}_k}\right) + w_{\text{los}} \cdot \text{HasLOS}_k$$

### Default Factor Weights
| Weight Parameter | Value | Tactical Rationale |
|---|---|---|
| $w_{\text{dist}}$ (Distance Weight) | $0.40$ | Favors immediate proximity to minimize travel time. |
| $w_{\text{damage}}$ (Damage Received Weight) | $0.35$ | Strong retaliatory bias against entities that recently attacked the AI. |
| $w_{\text{lowhp}}$ (Vulnerability Weight) | $0.15$ | Opportunistic finishing bias against heavily wounded opponents. |
| $w_{\text{los}}$ (Direct LOS Weight) | $0.10$ | Prioritizes targets that can be engaged immediately without navigating around walls. |

The candidate yielding the highest score $S^* = \max_k(S_k)$ is selected as the primary target.

---

## 3. Threat Persistence & Target Memory

When a target breaks line of sight (e.g. ducking behind a stone wall or into foliage), `wildbound::ai::TargetMemory` preserves target intel:

```cpp
struct TargetRecord {
    uint32_t targetId = 0;
    Vector2 lastKnownPosition = { 0.0f, 0.0f };
    float threatScore = 0.0f;
    float timeSinceLastSeen = 0.0f;
    float expirationDuration = 5.0f;
    bool hasDirectLOS = false;
};
```

### Memory Rules
1. **Position Updating**: As long as direct line of sight exists, `lastKnownPosition` continuously updates to the target's exact current coordinate.
2. **Confidence Decay**: Once LOS is broken, `hasDirectLOS` becomes `false`. The threat score linearly degrades over `expirationDuration` (default $3.0\text{s} - 5.0\text{s}$).
3. **Investigation Trigger**: The agent pursues `lastKnownPosition`. Upon arriving at the coordinate without reacquiring visual contact, the target record expires and transitions the agent to `Investigate` or `Search`.

