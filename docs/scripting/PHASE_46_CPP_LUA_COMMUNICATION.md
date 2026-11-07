# Wildbound — Phase 46: C++ <-> Lua Communication Protocol & Decision Execution

## 1. Architectural Philosophy

Wildbound maintains strict separation of concerns between game simulation and scripting:
- **C++ Engine**: Authoritative over physics, entity health, inventory, network state, attack cooldowns, hit detection, and spatial collisions.
- **Lua Scripts**: Autonomous decision engines. Scripts receive read-only situational awareness contexts and propose intentional action packages.

No Lua script has permission to directly damage an entity, spawn an item, or teleport an actor. Every proposed decision passes through a rigorous C++ validation layer before physical application.

```
+-----------------------------------------------------------+
|                      C++ Enemy Tick                       |
+-----------------------------------------------------------+
                             |
             [1. Build Context Table (Read-Only)]
                             |
                             v
+-----------------------------------------------------------+
|                      Lua AI Script                        |
|       (goblin_update / skeleton_update / enemy_update)    |
+-----------------------------------------------------------+
                             |
             [2. Return Decision Table (Intentions)]
                             |
                             v
+-----------------------------------------------------------+
|             C++ Enemy::ExecuteAIDecision()                |
|  - Finite check (move_x, move_y != NaN / Inf)             |
|  - Speed clamp                                            |
|  - Attack range verification                              |
|  - Attack cooldown & weapon readiness check               |
|  - State transition validation                            |
+-----------------------------------------------------------+
                             |
               [3. Authoritative World Update]
```

---

## 2. Context Table Specification (C++ -> Lua)

Passed as the sole argument `ctx` to the creature's update function (`goblin_update(ctx)`, etc.):

| Key | Type | Description |
| :--- | :--- | :--- |
| `enemy_id` | number | Unique unsigned integer ID of the enemy entity |
| `enemy_type` | string | Creature archetype identifier (e.g., `"goblin"`, `"skeleton"`) |
| `dt` | number | Elapsed frame delta time in seconds |
| `x`, `y` | number | World space coordinates of the enemy |
| `spawn_x`, `spawn_y` | number | Spawn coordinates for home tethering / returning |
| `state` | string | Current AI state string (e.g., `"IDLE"`, `"CHASE"`, `"ATTACK"`) |
| `state_timer` | number | Seconds the entity has been in the current state |
| `current_hp` | number | Absolute current health points |
| `max_hp` | number | Maximum health capacity |
| `hp_percent` | number | Normalized health ratio ($0.0 \dots 1.0$) |
| `has_target` | boolean | True if an opposing target is actively acquired |
| `target_x`, `target_y` | number | Position of the target (if `has_target == true`) |
| `target_dist` | number | Precomputed Euclidean distance to the target |
| `target_hp_percent` | number | Normalized health ratio of the target |
| `detection_radius` | number | Sensory detection radius in world units |
| `attack_radius` | number | Effective melee / attack trigger radius |
| `patrol_radius` | number | Permitted wander distance around spawn |
| `retreat_hp` | number | Health threshold below which retreat is considered |
| `can_retreat` | boolean | Entity fear immunity flag (`false` for undead skeletons) |
| `default_attack` | string | Primary attack identifier string (`"goblin_club"`, `"skeleton_blade"`) |

---

## 3. Decision Table Specification (Lua -> C++)

Returned by the Lua update function:

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `next_state` | string | Current State | Proposed transition state (e.g., `"CHASE"`, `"ATTACK"`, `"RETREAT"`) |
| `move_x` | number | 0.0 | Proposed movement vector X component (direction or delta) |
| `move_y` | number | 0.0 | Proposed movement vector Y component (direction or delta) |
| `request_attack` | boolean | false | True if the entity wishes to trigger an offensive strike |
| `attack_id` | string | `default_attack` | Specific attack definition ID requested |

---

## 4. C++ Validation Layer & Sanitization

Implemented in `wildbound::entities::Enemy::ExecuteAIDecision()`:

### 4.1 Finite Movement Validation
Lua movement vectors are tested for valid finite numbers:
```cpp
if (std::isfinite(moveX) && std::isfinite(moveY)) {
    float moveLenSq = moveX * moveX + moveY * moveY;
    if (moveLenSq > 0.0001f) {
        float moveLen = std::sqrt(moveLenSq);
        float clampedSpeed = std::min(m_speed, m_speed * (moveLen > 1.0f ? 1.0f : moveLen));
        m_velocity = { (moveX / moveLen) * clampedSpeed, (moveY / moveLen) * clampedSpeed };
    } else {
        m_velocity = { 0.0f, 0.0f };
    }
} else {
    Logger::Warn("AI", "Invalid NaN/Inf move vector from Lua for enemy " + std::to_string(m_id));
    m_velocity = { 0.0f, 0.0f };
}
```

### 4.2 Attack Range & Cooldown Verification
If Lua requests an attack, C++ validates the distance against the weapon definition and verifies whether the weapon is ready:
```cpp
if (requestAttack && m_target) {
    float targetDist = Vector2Distance(m_position, m_target->GetPosition());
    float maxAllowedDist = m_config.attackRadius * 1.5f;

    if (targetDist <= maxAllowedDist) {
        std::string attackToUse = attackId.empty() ? m_config.defaultAttack : attackId;
        if (m_attackSystem.CanAttack(attackToUse)) {
            m_attackSystem.ExecuteAttack(attackToUse, m_id, m_position, m_facingDirection);
        }
    } else {
        Logger::Debug("AI", "Attack rejected: Target beyond allowed physical range.");
    }
}
```

### 4.3 Safe Fallback Execution
If Lua encounters an unhandled runtime error or missing script function, C++ increments `s_fallbackAICount` and falls back to deterministic C++ behavior:
- Target within attack radius: Strike target.
- Target within detection radius: Approach target.
- Otherwise: Return to spawn.
This ensures enemies never freeze or hang during active gameplay.

