# Character Preview System

## Overview
The Character Preview System (`AppearancePreview`) provides an isolated, interactive staging ground for testing appearance changes before committing them to the active player character.

```
       Active Player Appearance
                 │
                 ▼ (BeginPreview)
         Isolated Preview
                 │
       [Rotate Direction (S/W/N/E)]
       [Toggle Animation (Idle/Walk)]
       [Apply Cosmetic Slot Change]
                 │
       ┌─────────┴─────────┐
       ▼                   ▼
 (ConfirmChanges)    (CancelChanges)
       │                   │
  Commit to Player   Discard Draft
```

---

## 1. Lifecycle and State Guarantee

The preview session enforces a strict lifecycle:
- **`BeginPreview(const CharacterAppearance& baseAppearance)`**:
  - Clones the base appearance into an isolated `CharacterAppearance draftAppearance`.
  - Initializes a dedicated preview `ModularCharacter` entity positioned at the preview center point.
  - Guarantees zero mutation of the gameplay `Player` entity while in progress.
- **`ApplySlotChange(CustomizationSlot slot, const std::string& cosmeticId)`**:
  - Updates only the draft preview appearance.
  - Immediately refreshes the staging `ModularCharacter` components and layers.
- **`ConfirmChanges()`**:
  - Returns `true` and yields the finalized `draftAppearance` to be committed to the player and persisted to disk.
- **`CancelChanges()`**:
  - Reverts `draftAppearance` back to the starting `baseAppearance` and discards all pending modifications.

---

## 2. Interactive Preview Controls

The preview viewport offers comprehensive real-time inspection controls:
- **Direction Rotation**:
  - Step left / step right or explicit direction selection: `Direction::South` (Front), `Direction::North` (Back), `Direction::East` (Right profile), `Direction::West` (Left profile).
  - Essential for verifying hairstyles, capes, quivers, and helmets from all angles.
- **Animation Mode Toggle**:
  - `AnimationMode::Idle` vs. `AnimationMode::Walk`.
  - Driven by delta time in `AppearancePreview::Update(float deltaTime)`.
  - Validates sprite synchronization across moving limbs, armor, and flowing garments.
- **Render Scale**:
  - Supports scaled viewport presentation (e.g. 2.0x, 3.0x, or 4.0x pixel-art scaling) centered inside the Wardrobe UI frame.

---

## 3. UI and DevUI Integration

Both player-facing UI (`UIManager::RenderWardrobe`) and developer tools (`DevUI::RenderWardrobe`) interface with `AppearancePreview`:
- **Player Wardrobe**: Centered 280x280 preview viewport with flanking rotation arrows and an "Animate" toggle button.
- **DevUI Debugger**: Direction dropdown, animation mode combo box, and raw layer inspect table showing active sprite sheet asset keys and color tints.

