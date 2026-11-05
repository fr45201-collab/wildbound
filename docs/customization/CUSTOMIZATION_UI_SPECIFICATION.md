# Customization UI Specification

## Overview
The Wardrobe screen provides players with an intuitive interface to browse cosmetic categories, preview appearance changes live, manage presets, and commit or discard alterations.

---

## 1. UI Layout & Architecture

The Wardrobe UI is rendered natively via raylib in `UIManager::RenderWardrobe()`:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                             WARDROBE & CUSTOMIZATION                        │
├───────────────────┬───────────────────────────────┬─────────────────────────┤
│  CATEGORY TABS    │     LIVE CHARACTER PREVIEW    │      ITEM SELECTOR      │
│                   │                               │                         │
│  [ Hair ]         │       ┌───────────────┐       │   ┌─────────────────┐   │
│  [ Headwear ]     │       │               │       │   │ Short Hair      │   │
│  [ Helmet ]       │       │    Preview    │       │   │ Long Hair       │   │
│  [ Face ]         │       │   Character   │       │   │ Buzz Cut        │   │
│  [ Chest ]        │       │               │       │   │ Braids          │   │
│  [ Legs ]         │       └───────────────┘       │   │ Ponytail        │   │
│  [ Feet ]         │                               │   └─────────────────┘   │
│  [ Back ]         │   [ < ] [ Rotate ] [ > ]      │                         │
│  [ Skin ]         │   [ Walk Cycle: ON/OFF ]      │   [ Scroll Up/Down ]    │
│  [ Loadouts ]     │                               │                         │
├───────────────────┴───────────────────────────────┴─────────────────────────┤
│   [ ESC: Cancel / Back ]       [ R: Reset ]       [ ENTER: Apply Changes ]  │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Interaction Specifications

- **Tab Navigation**:
  - 10 distinct tabs: Hair, Head, Helm, Face, Chest, Legs, Feet, Back, Skin, Loadouts.
  - Clicking a tab updates the active item list with items filtered by slot.
- **Item Selection**:
  - Displays cosmetic name, rarity badge, and selection outline.
  - Clicking an item instantly calls `AppearancePreview::ApplySlotChange()`.
- **Preview Viewport**:
  - Centered box with neutral dark slate background (`Color{ 30, 36, 48, 255 }`).
  - Rendered at 3.0x pixel scale.
  - Direction controls rotate preview character 90 degrees per click.
  - Walk toggle animates the preview character through walk frames using delta time.
- **Commit Controls**:
  - **Confirm (Green Button / Enter)**: Commits draft changes to player, saves active loadout, dispatches `AudioEventId::UI_CLICK`, and closes wardrobe.
  - **Reset (Yellow Button / R)**: Discards uncommitted modifications, restoring the player's initial appearance.
  - **Cancel (Red Button / ESC)**: Discards changes and closes wardrobe.

---

## 3. Audio & Input Integration

- Screen opens on `ActionType::OpenWardrobe` or UI menu click.
- Activates `InputContextType::Wardrobe`, blocking locomotion and combat actions while allowing UI navigation.
- All button clicks dispatch appropriate audio events through `AudioSystem`.

