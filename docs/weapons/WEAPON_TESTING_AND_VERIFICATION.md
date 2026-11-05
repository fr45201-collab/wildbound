# Weapon Testing and Verification Suite — Wildbound Phase 24

## 1. Automated Test Architecture (Test 26: `WeaponEquipmentTest`)

Automated validation for the Weapons & Equipment System is located in `tests/weapons/test_weapons_equipment.cpp` and registered in CTest as Test 26.

```
ctest --test-dir build -C Release -R WeaponEquipmentTest --output-on-failure
```

---

## 2. Test Specifications

### Test 1: `WeaponCatalogDefaultStats`
* **Objective**: Verifies canonical default stats for all 11 weapons and 1 ammunition item.
* **Assertions**:
  - `weapon_sword_iron_001`: damage = 25, speed = 1.2, cooldown = 0.833, reach = 48, arc = 90.
  - `weapon_sword_obsidian_001`: damage = 50, speed = 0.7, cooldown = 1.428, reach = 58, arc = 120.
  - `weapon_bow_longbow_001`: damage = 38, speed = 0.6, reach = 550, drawDuration = 1.0, projectileSpeed = 650.
  - `arrow_iron_001`: category = Ammunition, damage bonus = +5.

### Test 2: `WeaponInstanceRarityScaling`
* **Objective**: Validates multi-tier damage scaling and crit bonuses.
* **Assertions**:
  - Base damage = 30.0.
  - Common (1.00x) $\to$ 30 dmg.
  - Uncommon (1.05x) $\to$ 32 dmg (+1% crit).
  - Rare (1.10x) $\to$ 33 dmg (+2% crit).
  - Epic (1.15x) $\to$ 35 dmg (+3% crit).
  - Legendary (1.20x) $\to$ 36 dmg (+4% crit).
  - Mythic (1.25x) $\to$ 38 dmg (+5% crit).

### Test 3: `WeaponControllerMeleeLifecycle`
* **Objective**: Validates deterministic 4-phase state progression.
* **Assertions**:
  - Initial state: `Idle`, `CanAttack() == true`.
  - On attack start: transitions to `Windup` / `Prep`.
  - Ticking past windup: transitions to `Active`.
  - Ticking past active: transitions to `Recovery`.
  - Ticking past recovery: transitions to `Cooldown`.
  - Ticking past cooldown: returns to `Idle`.

### Test 4: `WeaponControllerHitDeduplication`
* **Objective**: Ensures single-hit-per-target guarantee across multi-frame swings.
* **Assertions**:
  - Target 101 recorded as hit on frame 1.
  - Target 101 query on frame 2 returns `true` (already hit).
  - Target 102 query on frame 2 returns `false` (not yet hit).
  - On new attack start, target 101 is cleared and can be hit again.

### Test 5: `WeaponControllerBowDrawCycle`
* **Objective**: Validates bow charge tension, release, and cancelation.
* **Assertions**:
  - `StartDraw()` transitions state and sets `IsDrawing() == true`.
  - Update with 0.5s of 1.0s drawDuration yields `GetDrawProgress() == 0.5`.
  - Release returns progress and transitions to `Recovery`.
  - Premature cancelation restores `Idle`.

### Test 6: `EquipmentSystemSlotsAndSwitching`
* **Objective**: Validates 8-slot array, dual weapon equipping, and hot-switching.
* **Assertions**:
  - Equip `weapon_sword_iron_001` into `MainHand`.
  - Equip `weapon_bow_hunting_001` into `OffHand`.
  - `ToggleWeaponSlot()` switches active slot from `MainHand` to `OffHand`.
  - Active weapon configuration matches the current active slot.

### Test 7: `EquipmentStatModifiersAndMitigation`
* **Objective**: Validates armor defense sum and diminishing returns formula.
* **Assertions**:
  - Total defense = 100.
  - Damage mitigation % = $\frac{100}{100 + 100} = 50.0\%$.
  - Speed modifier % is correctly aggregated from boots and chest plate.

### Test 8: `PlayerWeaponCombatAndArrowConsumption`
* **Objective**: Validates end-to-end player combat damage scaling and ammo deduction.
* **Assertions**:
  - Player equipped with Legendary iron sword inflicts 30 damage instead of 25.
  - Player equipped with bow without arrows cannot attack.
  - Adding 10 arrows permits firing, and exactly 1 arrow is deducted per shot.

