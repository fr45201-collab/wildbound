# Wildbound — Defensive & Evasive Behaviour

## 1. Overview & Defensive Archetypes

Defensive behaviour allows creatures to protect their territory, coordinate in packs, maintain standoff distances, or preserve their lives through tactical retreat.

This encompasses three primary tactical states:
1. **Defend / Circle (`AIState::Defend`)**: Circling or maintaining distance while awaiting pack members or combat cooldowns.
2. **Retreat (`AIState::Retreat`)**: Fleeing along vectors away from overwhelming hostile threats when health drops below archetype courage limits.
3. **Investigate & Search (`AIState::Investigate` & `AIState::Search`)**: Cautious exploration of disturbance points.

---

## 2. Defensive Stalking & Circling (`AIState::Defend`)

Predators such as Wolves or tactical defenders like Boars maintain an orbital standoff radius ($R_{\text{orbit}} \approx 64.0\text{px} - 96.0\text{px}$) from the player:

$$\mathbf{v}_{\text{radial}} = \frac{\mathbf{p}_{\text{agent}} - \mathbf{p}_{\text{target}}}{\|\mathbf{p}_{\text{agent}} - \mathbf{p}_{\text{target}}\|}$$

$$\mathbf{v}_{\text{tangent}} = (-v_{\text{radial}, y}, v_{\text{radial}, x}) \cdot \text{direction}$$

$$\mathbf{v}_{\text{orbit}} = \mathbf{v}_{\text{tangent}} \cdot w_{\text{tangent}} + (\text{dist} - R_{\text{orbit}}) \cdot \mathbf{v}_{\text{radial}} \cdot w_{\text{radial}}$$

This creates a dynamic circular pacing movement around the target before closing in for coordinated pounces.

---

## 3. Tactical Retreat & Evasion (`AIState::Retreat`)

### 3.1 Retreat Triggers
Retreat triggers dynamically when:
- $\text{HP} / \text{MaxHP} \le \text{RetreatThreshold}$ (Goblin: $0.20$, Wildlife: $1.00$).
- Target is invulnerable or out-damaging the agent severely.

### 3.2 Flee Direction Calculation
The evasion vector is directed directly away from the highest-threat target:
$$\hat{\mathbf{v}}_{\text{flee}} = \frac{\mathbf{p}_{\text{agent}} - \mathbf{p}_{\text{threat}}}{\|\mathbf{p}_{\text{agent}} - \mathbf{p}_{\text{threat}}\|}$$

The destination cell is projected $D_{\text{flee}} \approx 120.0\text{px}$ along $\hat{\mathbf{v}}_{\text{flee}}$. If the direct escape cell is blocked by a wall or boundary, `NavigationGrid::FindNearestWalkableCell` searches lateral escape angles ($\pm 45^\circ, \pm 90^\circ$).

### 3.3 Re-engagement & Safe Distance
When the agent reaches a safe standoff distance ($> 300.0\text{px}$) or escapes the threat's sight range, the retreat ends, transitioning the entity back to `AIState::Return` or `AIState::Idle`.

