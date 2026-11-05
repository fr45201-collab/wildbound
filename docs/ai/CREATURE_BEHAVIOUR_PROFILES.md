# Wildbound — Creature Behaviour Profiles

## 1. Overview & Data-Driven Taxonomy

Wildbound organizes all non-player entities into discrete behavioral archetypes configured via external JSON profiles located in `data/ai/creatures/`. Each archetype defines perception radii, field-of-view angles, movement speeds, combat engagement ranges, retreat courage thresholds, and behavioral affinities.

---

## 2. Archetype Specifications

| Archetype | JSON Profile | Speed | Sight $R$ | FOV | Attack $R$ | Retreat HP% | Primary Tactical Behavior |
|---|---|---|---|---|---|---|---|
| **Goblin** | `goblin.json` | $80.0\text{px/s}$ | $150.0\text{px}$ | $90^\circ$ | $28.0\text{px}$ | $20\%$ | Opportunistic melee; retreats at low HP; alerts pack. |
| **Skeleton** | `skeleton.json` | $60.0\text{px/s}$ | $180.0\text{px}$ | $110^\circ$ | $32.0\text{px}$ | $0\%$ | Relentless pursuit; immune to fear; never retreats. |
| **Wolf** | `wolf.json` | $110.0\text{px/s}$ | $220.0\text{px}$ | $120^\circ$ | $26.0\text{px}$ | $15\%$ | High-speed pursuit; defensive pack circling before striking. |
| **Wild Boar** | `wild_boar.json` | $75.0\text{px/s}$ | $130.0\text{px}$ | $80^\circ$ | $30.0\text{px}$ | $10\%$ | Territorial posturing; straight-line charging attack. |
| **Bat** | `bat.json` | $95.0\text{px/s}$ | $160.0\text{px}$ | $180^\circ$ | $22.0\text{px}$ | $25\%$ | Erratic flight path; wide peripheral vision; hit-and-run. |
| **Wildlife** | `wildlife.json` | $100.0\text{px/s}$ | $140.0\text{px}$ | $270^\circ$ | $0.0\text{px}$ | $100\%$ | Passive non-combatant; immediate flight upon any disturbance. |

---

## 3. Deep-Dive Profile Details

### 3.1 Goblin (`data/ai/creatures/goblin.json`)
- **Faction**: `Faction::Monsters`
- **Combat Style**: Opportunistic Melee.
- **Special Mechanics**: Evaluates pack numbers. When health drops below 20%, initiates `AIState::Retreat` while shouting to alert neighboring goblins within an audio alert radius of $120.0\text{px}$.

### 3.2 Skeleton (`data/ai/creatures/skeleton.json`)
- **Faction**: `Faction::Undead`
- **Combat Style**: Relentless Melee / Guard.
- **Special Mechanics**: Completely fearless (`retreat_health_fraction = 0.0`). Once triggered, maintains unrelenting pursuit until target escapes leash boundary ($350.0\text{px}$) or is destroyed.

### 3.3 Wolf (`data/ai/creatures/wolf.json`)
- **Faction**: `Faction::Beasts`
- **Combat Style**: Predatory Stalking & Sprinting.
- **Special Mechanics**: Fast sprint velocity ($110.0\text{px/s}$). Stalks targets in `AIState::Defend` orbiting at $70.0\text{px}$ radius before lunging into melee range.

### 3.4 Wild Boar (`data/ai/creatures/wild_boar.json`)
- **Faction**: `Faction::Neutral`
- **Combat Style**: Territorial Charge.
- **Special Mechanics**: Does not engage until target penetrates territorial radius ($80.0\text{px}$). Upon triggering, locks orientation and executes a high-speed charge.

### 3.5 Bat (`data/ai/creatures/bat.json`)
- **Faction**: `Faction::Monsters`
- **Combat Style**: Erratic Evasion.
- **Special Mechanics**: Adds sinusoidal lateral oscillations to its waypoint navigation, mimicking irregular flight.

### 3.6 Wildlife (Deer / Rabbit) (`data/ai/creatures/wildlife.json`)
- **Faction**: `Faction::Wildlife`
- **Combat Style**: Passive Fleeing.
- **Special Mechanics**: Has $0.0$ attack damage and $0.0$ attack range. Any sighting of player or monster immediately triggers `AIState::Retreat` at maximum velocity.

