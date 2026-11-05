# WILDBOUND — PHASE 24 WEAPON & EQUIPMENT REPOSITORY AUDIT

**Audit Date**: November 2026  
**Auditor**: Senior C++20 Gameplay & Combat Systems Architect  
**Project**: Wildbound (2D Top-Down Fantasy Battle Royale)  
**Target Systems**: Weapons, Melee Combat, Ranged Combat, Ammunition, Equipment, Paperdoll Cosmetics, Combat UI, DevUI  

---

## 1. Executive Summary

This repository audit analyzes Wildbound's existing combat, weapon, equipment, inventory, input, and animation systems prior to implementing **Phase 24: Weapons & Equipment System**.

Wildbound possesses a robust gameplay simulation foundation built on C++20, raylib 5.5, Lua 5.4, sol2, and MSVC:
- **Phase 5 Combat System**: Provides deterministic hurtbox/hitbox spatial intersections (`HitDetection`), damage calculation with armor mitigation (`DamageSystem`), combatant registration (`CombatSystem`), and event dispatching (`CombatEventDispatcher`).
- **Phase 15 Weapon Catalogue**: Defines 11 production weapons (6 swords and 5 bows) and 1 arrow ammunition item with 580 registered AMS textures, spritesheets, and directional grip attachment points.
- **Phase 17 Armor & Costumes**: Defines paperdoll cosmetic layers for Headwear, Chest/Armor, Pants, Shoes, Face, and Back.
- **Phase 19 & 23 Integration**: Provides initial weapon equipping into `EquipmentSystem`, rudimentary bow arrow consumption from `Inventory`, and projectile simulation via `ProjectileSystem`.

However, the combat architecture currently treats weapons primarily as cosmetic attachments with hardcoded attack values (`slash` with 25 damage, generic arrow with 35 damage) rather than distinct data-driven items with dynamic statistics, rarity tiers, attack lifecycles, and equipment modifiers.

Phase 24 elevates Wildbound's combat to production quality by delivering:
1. Data-driven weapon statistics and a 6-tier weapon rarity system.
2. A formal melee attack lifecycle (Preparation $\to$ Active $\to$ Recovery $\to$ Cooldown) with strict single-hit-per-target tracking.
3. A formal ranged bow attack lifecycle (Draw charge $\to$ Hold $\to$ Release $\to$ Projectile simulation) with ammo management.
4. An expanded 8-slot equipment system supporting Primary (`MainHand`) and Secondary (`OffHand`) weapons with hot-switching and attack-state locking.
5. Aggregate equipment stat modifiers (Defense, Damage Reduction %, Speed Modifier, Health Bonus, Attack Speed).
6. Dedicated player-facing HUD weapon displays, equipment cards, DevUI live inspectors, and automated CTest test suite.

---

## 2. Combat Subsystem Audit

### 2.1 CombatSystem & Attack Execution
- **File**: [CombatSystem.hpp](file:///c:/dev/Wildbound/src/combat/CombatSystem.hpp) / [CombatSystem.cpp](file:///c:/dev/Wildbound/src/combat/CombatSystem.cpp)
- **Status**: Operational and well-structured.
- **Mechanism**: `CombatSystem::ProcessAttack(Combatant* attacker, const std::string& attackId, Direction facing)` retrieves the `AttackDefinition` from `AttackSystem`, computes a directional `Rectangle` hitbox via `HitDetection::ComputeAttackHitbox`, iterates over all registered alive combatants, checks friendly-fire rules via `CombatRules::CanTarget`, and invokes `DamageSystem::ApplyDamage`.
- **Findings & Limitations**:
  - `ProcessAttack` executes instantly in a single frame. It lacks time-extended attack lifecycles (prep wind-up, active swing window, recovery follow-through).
  - While it generates a debug hit entry, it lacks persistent target tracking per swing ID to prevent multi-hitting across ticks if a hitbox remains active for several frames.
  - Base damage comes from `AttackDefinition` rather than factoring in the equipped weapon's base stats and rarity multiplier.

### 2.2 AttackSystem & Cooldowns
- **File**: [AttackSystem.hpp](file:///c:/dev/Wildbound/src/combat/AttackSystem.hpp) / [AttackSystem.cpp](file:///c:/dev/Wildbound/src/combat/AttackSystem.cpp)
- **Status**: Operational.
- **Mechanism**: Manages cooldown timers (`m_cooldownRemaining`) and attack dictionary (`m_attacks`). Loaded from `data/combat/attacks.json`.
- **Findings**:
  - Excellent for baseline attack cooldown verification.
  - Can be paired with `WeaponController` to govern fine-grained frame phases without disrupting legacy attack definitions.

### 2.3 ProjectileSystem
- **File**: [ProjectileSystem.hpp](file:///c:/dev/Wildbound/src/combat/ProjectileSystem.hpp) / [ProjectileSystem.cpp](file:///c:/dev/Wildbound/src/combat/ProjectileSystem.cpp)
- **Status**: Operational.
- **Mechanism**: Manages active arrow projectiles with position, velocity, rotation, damage, lifetime, source ID, and faction filtering.
- **Findings**:
  - `SpawnArrow` takes origin, facing direction, speed, damage, sourceId, and faction.
  - Supports obstacle collision against world solid tiles and entity hurtbox intersections.
  - Perfectly suited for bow releases with speeds and damages computed from weapon instances.

---

## 3. Weapon Subsystem Audit

### 3.1 Weapon Catalog & Data Structures
- **File**: [WeaponData.hpp](file:///c:/dev/Wildbound/src/weapons/WeaponData.hpp) / [WeaponManager.cpp](file:///c:/dev/Wildbound/src/weapons/WeaponManager.cpp)
- **Status**: Operational for asset metadata, missing combat statistics.
- **Catalog**: [weapon_catalog.json](file:///c:/dev/Wildbound/data/weapons/weapon_catalog.json) defines 12 entries:
  - **6 Swords**:
    1. `sword_wooden_training_001` (Training sword)
    2. `sword_rusty_short_001` (Shortsword)
    3. `sword_iron_001` (Arming sword)
    4. `sword_steel_longsword_001` (Longsword)
    5. `sword_curved_001` (Curved scimitar)
    6. `sword_ancient_rune_001` (Ancient runeblade)
  - **5 Bows**:
    1. `bow_wooden_hunting_001` (Hunting shortbow)
    2. `bow_longbow_001` (Longbow)
    3. `bow_reinforced_001` (Reinforced recurve)
    4. `bow_recurve_001` (Agile recurve bow)
    5. `bow_ancient_rune_001` (Ancient runebow)
  - **1 Ammunition**:
    1. `arrow_iron_001` (Broadhead iron arrows)
- **Findings**:
  - Currently `WeaponConfig` stores `id`, `displayName`, `category`, `subcategory`, `description`, `visuals`, `attachment`, `directions`, and `animationFrames`.
  - It does NOT yet store `WeaponStats` (`baseDamage`, `attackSpeed`, `attackCooldown`, `range`, `critChance`, `critMultiplier`, `staminaCost`, `drawDuration`, `projectileSpeed`) or `WeaponRarity`.
  - `WeaponManager::Initialize` successfully validates and caches all 12 items. Extending `WeaponConfig` and providing JSON parsing with balanced fallback defaults will enable full data-driven gameplay without breaking backwards compatibility.

---

## 4. Equipment Subsystem Audit

### 4.1 EquipmentSystem
- **File**: [EquipmentSystem.hpp](file:///c:/dev/Wildbound/src/entities/EquipmentSystem.hpp) / [EquipmentSystem.cpp](file:///c:/dev/Wildbound/src/entities/EquipmentSystem.cpp)
- **Status**: Functional for single weapon + 6 armor slots.
- **Slots Currently Defined**:
  - `Head`, `Chest`, `Legs`, `Feet`, `Face`, `Back`, `Weapon`.
- **Findings**:
  - Currently only one `Weapon` slot is present.
  - To fulfill the Battle Royale weapon switching requirement (primary and secondary weapons), we must expand the slot model to include `MainHand` and `OffHand` (with `Weapon` retained as an alias for `MainHand`).
  - Stat calculation currently sums defense from `ItemDef::armor.defenseRating` and calculates damage reduction via $D / (D + 100)$.
  - We must extend this to compute comprehensive `EquipmentStatModifiers`: total defense, damage reduction %, max HP bonus, movement speed modifier, and attack speed modifier.

### 4.2 Modular Character Paperdoll Synchronization
- **File**: [ModularCharacter.hpp](file:///c:/dev/Wildbound/src/entities/ModularCharacter.hpp)
- **Status**: Full 16-layer composited character paperdoll rendering.
- **Findings**:
  - Equipping an item in `EquipmentSystem` automatically constructs a `CosmeticItem` and passes it to `ModularCharacter::GetEquipment().Equip(...)`.
  - MainHand weapon is rendered at hand attachment grip points with directional depth ordering (85 down, 18 up, 85 left/right).
  - Weapon switching seamlessly updates the rendered held texture and spritesheet.

---

## 5. User Interface & DevUI Audit

### 5.1 Player-Facing HUD & Inventory UI
- **File**: [UIManager.hpp](file:///c:/dev/Wildbound/src/ui/UIManager.hpp) / [UIManager.cpp](file:///c:/dev/Wildbound/src/ui/UIManager.cpp)
- **Status**: Operational.
- **Findings**:
  - `RenderGameplayHUD` renders HP bar, SP bar, biome indicator, armor value, hotbar slots, and interaction prompts.
  - Needs a dedicated Weapon & Ammo HUD widget showing active weapon icon, name, rarity badge tint, base damage, and remaining arrow ammo count.
  - `RenderInventory` displays category tabs and item cards. Adding an Equipment panel displaying the 8 slots and stat breakdown will provide full player agency.

### 5.2 DevUI Tooling
- **File**: [DevUI.hpp](file:///c:/dev/Wildbound/src/ui/DevUI.hpp) / [DevUI.cpp](file:///c:/dev/Wildbound/src/ui/DevUI.cpp)
- **Status**: Operational with 17 tabs.
- **Findings**:
  - Tab 11 ("Weapons") currently previews weapon sprites and attachment grip coordinates.
  - We can expand this tab into an interactive "Weapons & Equipment" workbench:
    - Inspect runtime weapon state, rarity multipliers, attack state lifecycle.
    - Inspect player equipment slots and aggregate stat modifiers.
    - Live weapon equipping and test attacks.
    - Toggle debug hitbox overlays.

---

## 6. Gap Analysis & Required Additions

| Component | Current State | Required Phase 24 State | Action |
|:---|:---|:---|:---|
| **Weapon Stats** | Not defined | `WeaponStats` struct with damage, speed, cooldown, range, crit, stamina, draw time, projectile speed | Add to `WeaponData.hpp` |
| **Weapon Rarity** | Not defined | 6 tiers (`Common` to `Mythic`) with color codes and multipliers (1.00x - 1.25x) | Add to `WeaponData.hpp` |
| **Weapon Instance** | Non-existent | `WeaponInstance` with unique ID, definition ID, rarity, effective damage calculation | Create `WeaponInstance.hpp/.cpp` |
| **Weapon Controller** | Logic in `Player.cpp` | Dedicated `WeaponController` managing attack lifecycle, bow draw, one-hit-per-target tracking | Create `WeaponController.hpp/.cpp` |
| **Melee Lifecycle** | Instant 1-frame trigger | Prep $\to$ Active $\to$ Recovery $\to$ Cooldown state machine with directional hitboxes | Implement in `WeaponController` & `CombatSystem` |
| **Ranged Lifecycle** | Instant arrow spawn | Draw charge (0% to 100%) $\to$ Release $\to$ Projectile spawn with scaled stats | Implement in `WeaponController` & `Player` |
| **Equipment Slots** | 7 slots (1 weapon) | 8 slots (`MainHand`, `OffHand`, `Head`, `Chest`, `Legs`, `Feet`, `Face`, `Back`) | Extend `EquipmentSystem` |
| **Weapon Switching**| None | Switch between MainHand and OffHand with attack-state validation | Add `SwitchWeapon()` to `EquipmentSystem` |
| **Stat Modifiers** | Defense only | Aggregate `EquipmentStatModifiers` (defense, dmg reduction, HP, speed, attack speed) | Add to `EquipmentSystem` |
| **Weapon HUD** | Basic hotbar | Active weapon name, rarity badge, damage, ammo counter, draw charge meter | Add to `UIManager::RenderGameplayHUD` |
| **DevUI Workbench** | Asset preview only | Full runtime inspection of equipped weapons, stats, equipment slots, attack state | Extend `DevUI::RenderWeaponPreview` |
| **Automated Tests** | 25 tests (Phase 23) | Test 26 `WeaponEquipmentTest` covering all Phase 24 mechanics | Create `tests/weapons/test_weapons_equipment.cpp` |

---

## 7. Recommended Implementation Sequence

1. **Weapon Data & Rarity Architecture**:
   - Author `WeaponData.hpp` extensions (`WeaponRarity`, `WeaponStats`, `WeaponAttackState`).
   - Create `WeaponInstance.hpp` / `.cpp`.
   - Update `WeaponManager` to load / default stats and rarity for all 12 catalogue entries.
2. **Weapon Controller & Attack Lifecycle**:
   - Create `WeaponController.hpp` / `.cpp` supporting melee attack lifecycle, single-hit tracking, bow draw cycle, and ammo checks.
3. **Equipment System Extension**:
   - Extend `EquipmentSlot` to 8 slots (`MainHand`, `OffHand`, etc.).
   - Implement weapon switching (`SwitchWeapon`, `GetActiveWeaponSlot`).
   - Implement `EquipmentStatModifiers` aggregation.
4. **Player & Combat Integration**:
   - Integrate `WeaponController` into `Player`.
   - Connect weapon stats and rarity multipliers to `DamageSystem` / `CombatSystem`.
   - Connect procedural audio event triggers (`SwordSwing`, `SwordImpact`, `BowDraw`, `BowRelease`, `ArrowFlight`, `ArrowImpact`, `WeaponEquip`, `ArmorEquip`).
5. **UI & DevUI Integration**:
   - Update `UIManager` to render the Weapon & Ammo HUD widget and Equipment Slot breakdown.
   - Extend `DevUI` "Weapons" tab into a live Weapon & Equipment workbench.
6. **Testing & Verification**:
   - Create Test 26 (`test_weapons_equipment.cpp`).
   - Register in `tests/CMakeLists.txt`.
   - Build Release and run all 26 CTest test suites.
   - Run `--test-run` executable smoke test.
7. **Complete Documentation Suite & README Update**:
   - Author all 13 documents in `docs/weapons/` and `docs/PHASE_24_IMPLEMENTATION_REPORT.md`.
   - Update `README.md` status and feature list.

