# Weapon and Ammunition HUD Specification

## 1. Overview & Mechanics

The Weapon & Ammunition HUD occupies the Bottom-Left region of the viewport $(X = 12, Y = 300)$. It provides visual telemetry for:
1. **Dual Weapon Slots**: Slot 1 (Primary / MainHand) and Slot 2 (Secondary / OffHand).
2. **Active Slot Selection**: Gold beveled border framing the active hand.
3. **Attack Cooldown Sweeps**: Radial or vertical dark sweep overlay representing weapon swing recovery.
4. **Bow Draw Charge Bar**: Dynamic horizontal progress bar $[0.0..1.0]$ when drawing ranged weapons.
5. **Ammunition Counter**: Current arrows loaded in quiver vs. reserve in backpack inventory.

---

## 2. Visual Layout & Geometry

```
+-------------------------------------------------------------+
|  SLOT 1 (Active)           SLOT 2 (Holstered)                |
|  +-----------------+       +-----------------+              |
|  | [SWORD ICON]    |       | [BOW ICON]      |              |
|  | Iron Broadsword |       | Hunter's Longbow|              |
|  | Level 2         |       | Level 1         |              |
|  +-----------------+       +-----------------+              |
|  Cooldown: [READY]         Arrows: 24 / 96                  |
|                            Draw Charge: [==========    ] 70%|
+-------------------------------------------------------------+
```

### Slot Dimensions
- **Slot Box**: $36 \times 36\text{ px}$, $6\text{ px}$ gap between slots.
- **Active Slot Border**: $2\text{ px}$ solid Gold `#D4AF37`.
- **Inactive Slot Border**: $1\text{ px}$ subtle Gray `#4A4A5A`.
- **Slot Background**: Translucent dark charcoal `Color{ 20, 20, 26, 210 }`.

---

## 3. Cooldown Sweep Mechanics

When a melee weapon attacks or a ranged weapon fires, the weapon's cooldown timer $T_{cd}$ is reported by `WeaponManager`:
$$\text{Progress} = \frac{T_{remaining}}{T_{total}}$$
- The HUD renders a dark translucent overlay (`Color{ 0, 0, 0, 180 }`) over the weapon slot corresponding to the remaining fraction.
- As the weapon cools down, the overlay wipes vertically downwards from top to bottom.
- When $T_{remaining} \le 0.0\text{s}$, a $0.15\text{s}$ white flash is triggered to signal that the weapon is ready for attack input.

---

## 4. Bow Draw Charge Visualization

When using bows, holding the attack button charges the shot from $0.0$ to $1.0$ (determining projectile velocity and critical damage modifier):
- Position: Immediately below the active slot $(X = 12, Y = 342, W = 78, H = 5)$.
- Background: Dark Red/Gray $(X = 12, Y = 342)$.
- Fill Color:
  - $[0.0..0.4]$: Yellow `#F1C40F` (Undercharged shot, partial velocity).
  - $[0.4..0.9]$: Orange `#E67E22` (Effective shot).
  - $[0.9..1.0]$: Bright Cyan/White `#00FFFF` (Full draw, maximum range & critical bonus).
- If the attack is released or cancelled, the charge bar instantly recedes.

---

## 5. Ammunition Display

When a ranged weapon is active (e.g., Bow), the ammo counter appears next to the slot:
```
[ARROW GLYPH] 24 / 96
```
- **Equipped Count**: Arrows in the active quiver slot (e.g., 24).
- **Reserve Count**: Sum of matching arrow stacks across all backpack inventory slots (e.g., 96).
- **Out of Ammo Warning**: If equipped count reaches 0, the text turns Bright Red (`#E74C3C`) and pulses to alert the player to reload or switch weapons.

