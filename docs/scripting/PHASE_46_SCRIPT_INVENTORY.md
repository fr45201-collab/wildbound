# Wildbound — Phase 46: Script Inventory & Directory Catalog

## 1. Directory Tree Structure

All Lua scripts reside in the canonical `scripts/` directory:

```
scripts/
├── config/
│   └── engine.lua                   # Engine display, canvas, audio, and physics settings
└── gameplay/
    └── ai/
        ├── enemy_base.lua           # Generic polymorphic fallback AI decision engine
        ├── goblin.lua               # Archetype: Cowardly, opportunistic, retreat-capable
        ├── skeleton.lua             # Archetype: Relentless undead, fearless, never retreats
        └── behaviors/
            ├── idle.lua             # Idle behavior module (drift & scan)
            ├── patrol.lua           # Patrol behavior module (spawn perimeter wandering)
            ├── chase.lua            # Target pursuit behavior module
            ├── attack.lua           # Strike timing & attack triggering module
            ├── retreat.lua          # Flee vector calculation module
            ├── investigate.lua      # Disturbance investigation module
            ├── defend.lua           # Position holding module
            └── return.lua           # Spawn return navigation module
```

---

## 2. Script Profiles & Responsibilities

### 2.1 Configuration Scripts

#### `scripts/config/engine.lua`
- **Purpose**: Defines declarative window resolution, canvas sizing, target frame rates, audio volume scaling, and physics parameters.
- **Loaded by**: `wildbound::core::Game::Initialize()` on engine boot.
- **Exported Global**: `engine_config` table.

---

### 2.2 Creature Archetype Scripts

#### `scripts/gameplay/ai/goblin.lua`
- **Archetype**: Cowardly Rogue.
- **Key Characteristics**:
  - Strikes aggressively with `goblin_club` when target is in melee range ($d \le 32.0$).
  - Retreats whenever `hp_percent` drops below 20%, fleeing directly along the inverse target vector.
  - Returns to spawn perimeter when lost.
- **Entrypoint**: `wildbound.ai.goblin_update(ctx)`.

#### `scripts/gameplay/ai/skeleton.lua`
- **Archetype**: Fearless Undead Warrior.
- **Key Characteristics**:
  - Relentless engagement with `skeleton_blade` ($d \le 36.0$).
  - **Immune to retreat**: Never flees, even at 1% health.
  - Relentlessly chases targets across larger sensory radii ($d \le 160.0$).
- **Entrypoint**: `wildbound.ai.skeleton_update(ctx)`.

#### `scripts/gameplay/ai/enemy_base.lua`
- **Purpose**: Modular base controller that composes the 8 behavior modules.
- **Key Characteristics**:
  - Evaluates standard state machine transitions (`IDLE` -> `PATROL` -> `CHASE` -> `ATTACK` -> `RETREAT` -> `RETURN`).
  - Supports modular overrides.
- **Entrypoint**: `wildbound.ai.enemy_update(ctx)`.

---

### 2.3 Modular Behavior Modules (`scripts/gameplay/ai/behaviors/`)

Each module returns a standard table with `update(ctx)` and optional helper methods:

1. **`idle.lua`**: Evaluates whether target has entered detection radius. Initiates wandering or state preservation.
2. **`patrol.lua`**: Computes waypoint vectors within `patrol_radius` of `(spawn_x, spawn_y)`.
3. **`chase.lua`**: Computes normalized vector towards `(target_x, target_y)` when target is acquired.
4. **`attack.lua`**: Evaluates melee proximity ($d \le attack\_radius$). Emits `request_attack = true` and `attack_id`.
5. **`retreat.lua`**: Calculates inverted escape vector fleeing from the hostile target.
6. **`investigate.lua`**: Directs entity toward sound or disturbance coordinates.
7. **`defend.lua`**: Maintains guarded stance near defensive anchor point.
8. **`return.lua`**: Directs entity along straight-line or path back to home spawn location.

