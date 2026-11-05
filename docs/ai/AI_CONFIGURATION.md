# Wildbound — AI Configuration Reference

## 1. Overview & JSON Schema

All AI settings, navigation grid resolutions, threat weights, patrol profiles, and creature archetypes are externally defined in JSON documents under `data/ai/`.

```
data/ai/
├── global_ai_config.json      # Global update rates, thread budgets, debug defaults
├── navigation_config.json     # Cell size, heuristic mode, smoothing parameters
├── target_selection.json      # Threat scoring weights and decay timers
├── patrol_profiles.json       # Fixed waypoint routes and dwell settings
└── creatures/                 # Per-archetype entity definitions
    ├── goblin.json
    ├── skeleton.json
    ├── wolf.json
    ├── wild_boar.json
    ├── bat.json
    └── wildlife.json
```

---

## 2. Configuration Schemas

### 2.1 Global AI Configuration (`data/ai/global_ai_config.json`)
```json
{
  "max_active_ai_entities": 100,
  "default_path_recalc_interval_sec": 0.25,
  "near_target_recalc_interval_sec": 0.10,
  "max_leash_distance_pixels": 400.0,
  "stuck_detection_window_sec": 0.5,
  "stuck_recovery_threshold_sec": 1.5,
  "enable_multithreaded_eval": false,
  "debug_draw_paths": false,
  "debug_draw_perception": false
}
```

### 2.2 Navigation Configuration (`data/ai/navigation_config.json`)
```json
{
  "grid_cell_size_pixels": 16.0,
  "heuristic_mode": "octile",
  "allow_diagonal_movement": true,
  "prevent_corner_cutting": true,
  "enable_path_smoothing": true,
  "max_search_iterations": 2048,
  "diagonal_cost_multiplier": 1.41421356,
  "straight_cost_multiplier": 1.0
}
```

### 2.3 Target Selection Configuration (`data/ai/target_selection.json`)
```json
{
  "distance_weight": 0.40,
  "damage_received_weight": 0.35,
  "low_health_weight": 0.15,
  "line_of_sight_weight": 0.10,
  "target_memory_duration_sec": 5.0,
  "target_switch_hysteresis": 0.15
}
```

### 2.4 Creature Archetype Profile (`data/ai/creatures/goblin.json`)
```json
{
  "archetype_id": "goblin",
  "display_name": "Goblin Raider",
  "movement_speed": 80.0,
  "sight_radius": 150.0,
  "field_of_view_deg": 90.0,
  "attack_range": 28.0,
  "attack_cooldown_sec": 1.2,
  "retreat_health_fraction": 0.20,
  "avoidance_radius": 24.0,
  "avoidance_weight": 1.5,
  "can_wander": true,
  "wander_radius": 120.0
}
```

