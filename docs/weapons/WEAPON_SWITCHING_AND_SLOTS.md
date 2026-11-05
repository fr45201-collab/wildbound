# Weapon Switching and Equipment Slots — Wildbound Phase 24

## 1. Equipment Slot Architecture

Wildbound adopts an 8-slot modular equipment model defined in `src/entities/EquipmentSystem.hpp`:

```cpp
enum class EquipmentSlot {
    MainHand,  // Primary equipped weapon (Sword, Bow, Staff)
    OffHand,   // Secondary weapon or shield (Shield, Secondary Bow/Sword)
    Head,      // Helmets, Hoods, Caps, Headbands
    Chest,     // Tunics, Hauberks, Robes, Cuirasses
    Legs,      // Trousers, Greaves, Skirts, Chausses
    Feet,      // Boots, Shoes, Sandals, Sabatons
    Face,      // Masks, Glasses, Visors
    Back       // Cloaks, Capes, Backpacks, Quivers
};
```

---

## 2. Weapon Slot Switching Mechanics

A player can equip weapons into both `MainHand` and `OffHand`. At any moment during traversal or combat, the player has one active weapon designated by:
```cpp
EquipmentSlot m_activeWeaponSlot{EquipmentSlot::MainHand};
```

### Switching API
* `SetActiveWeaponSlot(EquipmentSlot slot)`: Validates that `slot` is either `MainHand` or `OffHand`. If the target slot is empty or invalid, logs a warning and returns `false`.
* `ToggleWeaponSlot()`: Toggles between `MainHand` and `OffHand`. If the other slot is empty, remains on the current slot.
* `Player::SwitchWeapon(EquipmentSlot slot)` / `Player::ToggleWeapon()`: Dispatches to `EquipmentSystem` and plays the `"Item.Equip"` audio cue.

### Weapon Switching State Guard
Weapon hot-switching is constrained by the `WeaponController` state machine:
```cpp
bool WeaponController::CanSwitchWeapon() const {
    return (m_attackState == WeaponAttackState::Idle || 
            m_attackState == WeaponAttackState::Cooldown) && 
           !m_isDrawing;
}
```
If the player attempts to switch weapons while:
- In windup (`Prep`),
- Actively cleaving (`Active`),
- Follow-through recovery (`Recovery`),
- Or holding a charged bow draw (`m_isDrawing == true`),

the switch is rejected, preventing animation canceling exploits and maintaining combat commitment.

---

## 3. Paperdoll & Modular Appearance Synchronization

When an item is equipped or unequipped in `EquipmentSystem`:
1. The corresponding `CosmeticCategory` is resolved (`EquipmentSlotToCosmeticCategory`).
2. If the equipped item provides cosmetic spritesheet metadata, `Player::GetModularAppearance().GetEquipment().Equip(...)` is invoked.
3. This guarantees that visual character representation, HUD paperdoll previews, and physical gameplay collision bounds remain 100% in sync without manual glue code.

