# Wildbound — Ammunition Integration

## 1. Ammunition Specification
Ammunition enables ranged combat weapon classes (e.g. Bows) to function authoritatively:

- **Item ID**: `arrow_iron_001`
- **Category**: `ItemCategory::Ammunition`
- **Subcategory**: `arrow`
- **Max Stack**: 30
- **Weight**: 0.05 kg
- **Compatible Weapon**: `"bow"`
- **Bonus Damage**: +5.0 DMG
- **Textures**:
  - Icon: `tex_arrow_iron_001_icon`
  - Ground: `tex_arrow_iron_001_ground`
  - In-flight Projectile: `tex_ammo_arrow_broadhead_001_held`

---

## 2. Combat Enforcement in `Player::TryAttack`
When the player attempts to attack with a bow equipped:
1. `Player::TryAttack` verifies `m_equipmentSystem.GetEquippedWeaponConfig()->category == WeaponCategory::Bow`.
2. Queries inventory for 1 arrow:
   - Primary query: `m_inventory.RemoveItemById("arrow_iron_001", 1)`.
   - Secondary query: Any stack matching `"arrow"` or `"ammo"`.
3. If no ammunition is available:
   - Logs warning: `"[COMBAT] Cannot fire bow: Out of arrows!"`.
   - Returns `false` immediately without executing attack animation or spawning projectiles.
4. If ammunition is consumed:
   - Spawns projectile via `combatSys.GetProjectileSystem().SpawnArrow(...)`.
   - Returns `true`.

