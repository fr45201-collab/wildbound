# Weapon Definitions and Statistics — Wildbound Phase 24

## 1. Overview

Wildbound features a comprehensive weapon catalogue comprising 11 distinct armaments (6 melee swords, 5 ranged bows) and 1 dedicated ammunition type (iron arrows), integrated across the Asset Management System (AMS), item database, and runtime weapon managers.

All weapons are defined in C++ with hardcoded defaults matching Phase 15 asset metadata and can be dynamically overridden or configured via `assets/definitions/weapons.json`.

---

## 2. Canonical Weapon Catalogue

| Weapon ID | Display Name | Type | Base Dmg | Speed (atk/s) | Cooldown (s) | Range (px) | Crit % | Crit Mult | Arc / Speed | Special / Scaling |
|---|---|---|---|---|---|---|---|---|---|---|
| `weapon_sword_iron_001` | Iron Longsword | Melee | 25.0 | 1.2 | 0.833 | 48.0 | 5% | 1.5x | 90° | Balanced Starter Blade |
| `weapon_sword_steel_001`| Steel Broadsword | Melee | 35.0 | 1.0 | 1.000 | 52.0 | 7% | 1.6x | 100° | High Cleave & Reach |
| `weapon_sword_silver_001`| Silver Rapier | Melee | 22.0 | 1.6 | 0.625 | 44.0 | 15% | 2.0x | 45° | Rapid Thrust & High Crit |
| `weapon_sword_obsidian_001`| Obsidian Claymore| Melee | 50.0 | 0.7 | 1.428 | 58.0 | 10% | 1.8x | 120° | Massive Arc Heavy Damage |
| `weapon_sword_crystal_001`| Crystal Sabre | Melee | 30.0 | 1.3 | 0.769 | 46.0 | 12% | 1.7x | 80° | Fast Arc & High Mobility |
| `weapon_sword_wood_practice_001`| Wooden Training Sword | Melee | 10.0 | 1.4 | 0.714 | 42.0 | 3% | 1.2x | 85° | Low Stamina Training |
| `weapon_bow_hunting_001`| Hunter's Shortbow | Ranged | 20.0 | 1.0 | 1.000 | 350.0 | 8% | 1.5x | 0.6s draw / 500 px/s | Rapid Snap Shooting |
| `weapon_bow_longbow_001`| Iron Longbow | Ranged | 38.0 | 0.6 | 1.666 | 550.0 | 10% | 1.7x | 1.0s draw / 650 px/s | Heavy Sniper Bow |
| `weapon_bow_recurve_001`| Recurve Warbow | Melee/Rng| 30.0 | 0.8 | 1.250 | 450.0 | 12% | 1.6x | 0.8s draw / 580 px/s | Balanced Combat Bow |
| `weapon_bow_elven_001` | Elven Greenwood Bow | Ranged | 28.0 | 1.2 | 0.833 | 480.0 | 14% | 1.8x | 0.5s draw / 620 px/s | Featherweight Quickdraw |
| `weapon_bow_darkwood_001`| Shadow Darkwood Bow | Ranged | 45.0 | 0.5 | 2.000 | 600.0 | 20% | 2.2x | 1.2s draw / 700 px/s | Maximum Range & Lethal Crit |
| `arrow_iron_001` | Iron Broadhead Arrow | Ammo | +5.0 | — | — | — | — | — | Stack: 100 | Standard Combat Ammunition |

---

## 3. Data Schema & Representation

### `WeaponStats` C++ Structure (`src/weapons/WeaponData.hpp`)

```cpp
struct WeaponStats {
    float baseDamage{20.0f};          // Base damage per hit
    float attackSpeed{1.0f};          // Attacks per second (cadence)
    float attackCooldown{1.0f};       // Cooldown duration between attacks (seconds)
    float reach{48.0f};               // Melee sweep radius or ranged max effective range (pixels)
    float critChance{0.05f};          // Base critical strike probability [0.0, 1.0]
    float critMultiplier{1.5f};      // Damage multiplier on critical hit
    float staminaCost{10.0f};         // Stamina consumed per swing/shot
    float swingArcAngle{90.0f};       // Melee swing arc width in degrees
    float drawDuration{0.0f};         // Ranged bow draw time to reach 100% tension (seconds)
    float projectileSpeed{0.0f};      // Launch velocity for ranged projectiles (pixels/second)
};
```

---

## 4. JSON Serialization Format (`assets/definitions/weapons.json`)

```json
{
  "weapons": {
    "weapon_sword_iron_001": {
      "name": "Iron Longsword",
      "type": "Melee",
      "default_rarity": "Common",
      "damage": 25.0,
      "attack_speed": 1.2,
      "cooldown": 0.833,
      "reach": 48.0,
      "crit_chance": 0.05,
      "crit_multiplier": 1.5,
      "stamina_cost": 10.0,
      "swing_arc": 90.0
    },
    "weapon_bow_longbow_001": {
      "name": "Iron Longbow",
      "type": "Ranged",
      "default_rarity": "Rare",
      "damage": 38.0,
      "attack_speed": 0.6,
      "cooldown": 1.666,
      "reach": 550.0,
      "crit_chance": 0.10,
      "crit_multiplier": 1.7,
      "stamina_cost": 18.0,
      "draw_duration": 1.0,
      "projectile_speed": 650.0
    }
  }
}
```
All fields are parsed with robust defaults and validation in `WeaponManager::LoadFromJson()`.

