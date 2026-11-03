# Player Entity Architecture & Character Subsystem

## 1. Overview

The **Player Character System** in Wildbound provides a modular, extensible, and data-driven entity framework built specifically for 2D top-down pixel-art adventure games. It adheres strictly to the single responsibility principle and encapsulates state, movement, animation, and collision into distinct, cohesive classes.

---

## 2. Entity Class Hierarchy

```
Entity (Base)
  │
  ├── m_id (uint32_t)
  ├── m_name (std::string)
  ├── m_tag (std::string)
  ├── m_position (Vector2: Logical feet anchor)
  └── m_active (bool)
       ▲
       │
Character (Specialized Animated Actor)
  │
  ├── m_stats (CharacterStats: walkSpeed, runSpeed, normalizeDiagonal)
  ├── m_collisionConfig (CharacterCollision: feet box geometry & offsets)
  ├── m_animController (AnimationController: frame tracking & timing)
  ├── m_animStateMachine (AnimationStateMachine: velocity-driven clip selection)
  └── CharacterController (Input polling & directional vector normalization)
       ▲
       │
Player (Controllable Player Entity)
  │
  ├── ResolveMovement(delta, world): Axis-decoupled swept AABB collision
  ├── Spawn(position): Level spawn initialization
  └── Camera integration: Target feed for smooth tracking
```

---

## 3. Data-Driven Configuration Schema

Player and NPC character properties are stored externally in JSON configuration files (`data/characters/<id>.json`), avoiding hardcoded values in C++ source files.

### Configuration Specification (`data/characters/player.json`)

```json
{
  "id": "player",
  "name": "Explorer",
  "spritesheet": "player_spritesheet",
  "stats": {
    "walk_speed": 100.0,
    "run_speed": 165.0,
    "normalize_diagonal": true
  },
  "collision": {
    "enabled": true,
    "width": 16.0,
    "height": 12.0,
    "offset_x": 0.0,
    "offset_y": 0.0
  },
  "animations": {
    "idle_down": "player_idle_down",
    "idle_up": "player_idle_up",
    "idle_left": "player_idle_left",
    "idle_right": "player_idle_right",
    "walk_down": "player_walk_down",
    "walk_up": "player_walk_up",
    "walk_left": "player_walk_left",
    "walk_right": "player_walk_right",
    "run_down": "player_run_down",
    "run_up": "player_run_up",
    "run_left": "player_run_left",
    "run_right": "player_run_right"
  }
}
```

---

## 4. Logical Position & Feet Anchor

In top-down 2D rendering, visual sprites and collision footprints must share a consistent logical anchor:
* **Visual Anchor**: The character's logical `m_position` represents the **bottom-center point of the character's feet** on the ground.
* **Sprite Alignment**: The 32×48 sprite texture has its origin at `(16, 48)`. Dest rect is calculated as `(pos.x - 16, pos.y - 48, 32, 48)`.
* **Collision Alignment**: The 16×12 collision box is centered horizontally at `pos.x` and extends upwards by 12 pixels (`pos.y - 12` to `pos.y`).
* **Depth Interleaving**: The Y-sorting pass in `WorldRenderer` uses `m_position.y` as the entity's baseline Z-order, ensuring that the player's head and torso naturally overlap behind environmental obstacles (trees, rocks, walls) while the feet remain blocked in front.
