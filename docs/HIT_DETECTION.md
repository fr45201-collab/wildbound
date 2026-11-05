# Wildbound Hit Detection & Hitbox Architecture

Wildbound uses directional Axis-Aligned Bounding Box (AABB) hit detection for melee and close-range combat. Hit detection calculations are separated from rendering sprites, guaranteeing deterministic collision regardless of display resolution or camera scaling.

---

## 1. Hitbox vs Hurtbox Definitions

- **Hurtbox**: The vulnerability volume representing an entity's physical presence. For `Player` and `Enemy`, this corresponds to their world bounds derived from their feet anchor position:
  $$\text{Hurtbox} = [x - \text{width}/2,\; y - \text{height},\; \text{width},\; \text{height}]$$
- **Hitbox**: The transient strike volume generated in front of an attacking entity during a swing or thrust.

---

## 2. Directional Hitbox Generation

Implemented in [`src/combat/HitDetection.cpp`](file:///c:/dev/Wildbound/src/combat/HitDetection.cpp), the hitbox origin and dimensions dynamically align with the attacker's facing direction:

```cpp
Rectangle HitDetection::ComputeAttackHitbox(Vector2 attackerPos,
                                           graphics::animation::Direction facing,
                                           const AttackDefinition& attack)
```

### Direction Alignment Math:

| Direction | Hitbox Width | Hitbox Height | Hitbox X Offset | Hitbox Y Offset |
|---|---|---|---|---|
| **`Down`** | `hitboxWidth` | `hitboxHeight` | $-\text{width}/2$ | $+\text{forwardOffset}$ |
| **`Up`** | `hitboxWidth` | `hitboxHeight` | $-\text{width}/2$ | $-\text{forwardOffset} - \text{height}$ |
| **`Left`** | `hitboxHeight` | `hitboxWidth` | $-\text{forwardOffset} - \text{height}$ | $-\text{width}/2 - 12$ |
| **`Right`** | `hitboxHeight` | `hitboxWidth` | $+\text{forwardOffset}$ | $-\text{width}/2 - 12$ |

*Note: For Left and Right attacks, width and height are transposed so the swing reaches outward along the horizontal axis.*

---

## 3. Intersection Testing

Raylib's native `CheckCollisionRecs` performs the AABB overlap test:

```cpp
bool HitDetection::Intersects(const Rectangle& hitbox, const Rectangle& hurtbox) {
    return CheckCollisionRecs(hitbox, hurtbox);
}
```

### Hit Point Determination
When a hit is registered, the contact point is calculated as the center of the intersection rectangle:
$$\text{HitPoint} = \left(\max(h_1.x, h_2.x) + \frac{w_{\text{overlap}}}{2},\; \max(h_1.y, h_2.y) + \frac{h_{\text{overlap}}}{2}\right)$$
This position is passed into `DamageEvent::hitPosition` for spawning visual blood splatters or spark VFX.

---

## 4. Visual Debugging

Pressing **`F6`** in game enables Combat Hitbox Visualization:
- **Red Rectangle**: Active attack hitboxes (flashes for 0.15s upon attack trigger).
- **Green Rectangle**: Combatant hurtboxes (green when idle/chasing, turns yellow when damaged).
- **Blue Line**: Facing direction vector.

