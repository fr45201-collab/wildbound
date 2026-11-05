# Weapon and Equipment UI — Wildbound Phase 24

## 1. Overview of Presentation Layers

The Weapon and Equipment systems interface with two distinct UI layers:
1. **Player-Facing UI (`UIManager.cpp`)**: Built using native raylib vector rendering, theme colors, and icons.
2. **Developer Tooling (`DevUI.cpp`)**: Built using Dear ImGui and rlImGui for live runtime inspection and tweaking.

---

## 2. Player-Facing raylib HUD & Inventory

### Active Weapon & Ammo HUD Widget (Bottom-Left)
Located at screen coordinates `(12, 305)` with dimensions `175x48`:
* **Weapon Name & Rarity**: Displayed in the color assigned to the weapon's rarity tier (e.g. Gold for Legendary, Red for Mythic).
* **Damage Stat**: Formatted as `"DMG: <value>"`.
* **Ammunition / Slot Readout**:
  - For bows: `" | Arrows: <count>"` scanning inventory stacks.
  - For melee: `" | [MainHand]"` or `" | [OffHand]"`.
* **Attack Cooldown Bar**: A progress bar (`Rectangle{ 18, 342, 120, 4 }`) that visually drains during `Cooldown` state until weapon is ready.

### Inventory Screen Controls
* **EQUIP Button**: When inspecting weapons or armor, equips the item to its designated slot.
* **SWAP WPN Button**: Toggles between `MainHand` and `OffHand` weapons with instant visual and audio feedback.

---

## 3. Developer Tooling (`DevUI.cpp` Tab 10)

Tab 10 in DevUI provides comprehensive weapons & equipment diagnostics:
1. **Canonical Weapon Catalogue**: Displays all 11 weapons and 1 ammunition item with Type, Base Damage, Attack Speed, Reach, Crit %, and Arc.
2. **Rarity Damage Scaling Matrix**: Real-time table showcasing damage multipliers (1.00x - 1.25x) and effective damage across all 6 tiers.
3. **Live Player Equipment Inspector**:
   - Lists all 8 equipment slots (MainHand, OffHand, Head, Chest, Legs, Feet, Face, Back).
   - Displays equipped Item ID, Rarity, Defense, and Durability.
   - Shows active weapon slot with a `Toggle Active Weapon` button.
   - Displays real-time `WeaponController` attack state (`Idle`, `Prep`, `Active`, `Recovery`, `Cooldown`), state timer, and bow draw progress.
   - Summarizes total defense, % damage mitigation, speed modifiers, and effective player damage.
4. **Quick Equipment Spawner**: One-click debug buttons to equip Iron Longsword, Steel Broadsword, or Elven Bow directly to the player.

