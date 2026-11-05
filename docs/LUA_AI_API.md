# Wildbound Lua AI Scripting API

Wildbound provides a sandboxed Lua 5.4 scripting environment powered by `sol2` for authoring enemy artificial intelligence and behaviors.

---

## 1. Safety Guidelines & Isolation

Lua scripts operate under strict gameplay boundaries:
1. **Intention Only**: Lua AI scripts cannot directly modify entity health, damage values, or coordinates. Scripts only output *intentions* (`move_x`, `move_y`, `next_state`, `request_attack`).
2. **Read-Only Context**: The engine passes a copy of the entity's current state as a Lua table.
3. **No Direct C++ Pointers**: Pointers to `Combatant`, `Entity`, or `World` are never exposed to Lua scripts.

---

## 2. API Contract: `wildbound.ai.update_enemy`

Every AI decision interval, the engine invokes:
```lua
local result = wildbound.ai.update_enemy(context)
```

### Input Table (`context`)

| Field | Type | Description |
|---|---|---|
| `enemy_id` | `integer` | Unique ID of the enemy executing AI |
| `x` | `number` | Current world position X |
| `y` | `number` | Current world position Y |
| `spawn_x` | `number` | Home spawn position X |
| `spawn_y` | `number` | Home spawn position Y |
| `hp_percent` | `number` | Health percentage in range $[0.0, 1.0]$ |
| `detection_radius` | `number` | Target vision radius in pixels |
| `attack_radius` | `number` | Attack reach threshold in pixels |
| `retreat_hp` | `number` | Retreat health threshold percentage |
| `state` | `string` | Current AI state (`"IDLE"`, `"CHASE"`, `"ATTACK"`, etc.) |
| `has_target` | `boolean` | Whether a target is within detection radius |
| `target_id` | `integer` | CombatantId of closest target |
| `target_x` | `number` | Target world position X |
| `target_y` | `number` | Target world position Y |
| `target_dist` | `number` | Distance to target in world pixels |

### Output Table (`result`)

| Field | Type | Description |
|---|---|---|
| `next_state` | `string` | Target AI state (`"IDLE"`, `"CHASE"`, `"ATTACK"`, `"RETREAT"`, `"PATROL"`) |
| `target_id` | `integer` | ID of target entity |
| `move_x` | `number` | Desired movement direction X $[-1.0, 1.0]$ |
| `move_y` | `number` | Desired movement direction Y $[-1.0, 1.0]$ |
| `request_attack` | `boolean` | Whether the entity requests an attack execution |
| `attack_id` | `string` | Attack definition to execute (`"goblin_club"`, `"skeleton_blade"`) |

---

## 3. Modular Behavior Scripts

Behavior scripts are located under `scripts/gameplay/ai/`:
- `enemy_base.lua`: Main dispatch router and state machine logic.
- `behaviors/idle.lua`: Standing watch and waiting.
- `behaviors/patrol.lua`: Waypoint seeking around spawn origin.
- `behaviors/chase.lua`: Pursuing player target.
- `behaviors/attack.lua`: Attack request dispatch within melee range.
- `behaviors/retreat.lua`: Directional fleeing away from threats when wounded.

