# Weapon Debugging and Troubleshooting Guide — Wildbound Phase 24

## 1. Developer UI Diagnostic Controls

Press **F1** or **`** (Tilde) during gameplay to open DevUI, and navigate to **Tab 10: Weapons & Equipment**.

### Inspecting Weapon States
1. Observe the **Attack State** field:
   - Should be `Idle` when no actions are taken.
   - Momentarily flashes `Prep` $\to$ `Active` $\to$ `Recovery` $\to$ `Cooldown` when swinging or shooting.
2. Check **Can Attack**:
   - If `NO`, verify whether current stamina is sufficient (`stamina >= stats.staminaCost`) or if cooldown timer is still counting down.
3. Check **Can Switch**:
   - If `NO`, verify if player is holding a charged bow draw or in the middle of active swing recovery.

---

## 2. Common Issues & Resolution

### Symptom: Weapon deals 0 or baseline (10 dmg) damage
* **Cause**: No weapon equipped in active slot, or weapon definition failed to load.
* **Fix**: Check `Player::GetEquippedWeaponConfig()`. Ensure item ID matches `weapon_sword_*` or `weapon_bow_*` registered in `WeaponManager`.

### Symptom: Bow will not fire
* **Cause**: No ammunition in player inventory.
* **Fix**: Bows require `arrow_iron_001` or another arrow stack. Verify `player->GetAmmoCount("arrow_iron_001") > 0`. Also verify the attack key is held long enough to surpass the minimum 20% draw threshold (`drawProgress >= 0.20f`).

### Symptom: Melee attack hits the same enemy multiple times in one swing
* **Cause**: Hit tracking set was not cleared or hit check was bypassed.
* **Fix**: Ensure `m_targetsHitThisSwing.contains(targetId)` is checked in `CombatSystem::ProcessAttack` before inflicting damage.

### Symptom: Player moves too slowly or too fast after equipping gear
* **Cause**: Unbalanced `speedModifierPercent` on armor pieces.
* **Fix**: Inspect DevUI Tab 10 "Speed Mod %". Each heavy armor piece typically has $-3\%$ to $-5\%$, while boots provide $+5\%$ to $+10\%$. Total modifier is clamped between $-50\%$ and $+50\%$.

