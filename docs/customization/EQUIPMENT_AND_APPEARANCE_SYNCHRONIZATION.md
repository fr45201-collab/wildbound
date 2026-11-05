# Equipment and Appearance Synchronization

## Overview
A critical architectural requirement in Wildbound is that character cosmetics must never interfere with, mutate, or override gameplay balance numbers, armor values, or combat parameters.

```
┌─────────────────────────────────┐       ┌─────────────────────────────────┐
│        EquipmentSystem          │       │  CharacterCustomizationManager  │
├─────────────────────────────────┤       ├─────────────────────────────────┤
│ • Authority over combat stats   │       │ • Authority over visual display │
│ • Armor defense calculation     │       │ • Cosmetic layer assignments    │
│ • Speed penalties / weight      │       │ • Hair & skin color tinting     │
│ • Durability & item instances   │       │ • Vanity overrides / transmog   │
└────────────────┬────────────────┘       └────────────────┬────────────────┘
                 │                                         │
                 │              Visual Sync                │
                 └────────────────► ◄──────────────────────┘
                                    │
                                    ▼
                         ModularCharacterRenderer
```

---

## 1. Separation of Responsibilities

| Responsibility | Owning System | Impact on Stats | Impact on Appearance |
| :--- | :--- | :--- | :--- |
| Physical / Elemental Defense | `EquipmentSystem` | YES | None |
| Movement Speed Modifiers | `EquipmentSystem` | YES | None |
| Combat Damage Mitigation | `EquipmentSystem` | YES | None |
| Cosmetic Sprite Sheet Selection | `CharacterCustomizationManager` | NO | YES |
| Hair & Skin Tone Customization | `CharacterCustomizationManager` | NO | YES |
| Preview & Loadout Presets | `CharacterCustomizationManager` | NO | YES |

---

## 2. Synchronization Mechanisms

When a player equips a combat item in `EquipmentSystem`:
1. If the player has **NOT** selected a cosmetic override for that slot (i.e., slot is set to default or empty), the cosmetic system displays the visual asset corresponding to the equipped combat item.
2. If the player **HAS** selected a cosmetic vanity override for that slot, the visual appearance displays the chosen vanity item, while `EquipmentSystem` applies the statistical buffs of the actual equipped item.
3. Tests in `tests/customization/test_customization.cpp` (`CosmeticStatDecoupling`) explicitly assert that toggling cosmetic armor changes `0` points of `EquipmentStatModifiers.totalDefense`, preserving gameplay fairness.

---

## 3. Directional Anchor & Socket Alignment

Visual equipment layers are anchored to modular character sockets:
- Headwear and helmets track head position and tilt during walk animations.
- Chest garments follow torso bobbing.
- Back accessories track spine anchor points and swap depth relative to the torso when facing North vs. South.

