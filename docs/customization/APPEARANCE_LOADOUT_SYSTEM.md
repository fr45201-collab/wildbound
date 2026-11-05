# Appearance Loadout System

## Overview
The Appearance Loadout System allows players to save, organize, and quickly swap multiple complete cosmetic presets. Each preset encapsulates an entire 9-slot appearance configuration along with custom metadata and timestamps.

---

## 1. AppearanceLoadout Data Structure

```cpp
struct AppearanceLoadout {
    std::string id;            // Unique identifier, e.g. "loadout_001"
    std::string name;          // User-facing display name, e.g. "Forest Ranger"
    CharacterAppearance appearance; // 9-slot cosmetic map + skin/hair tints
    std::string createdAt;     // ISO 8601 timestamp
    std::string updatedAt;     // ISO 8601 timestamp
};
```

---

## 2. Loadout Operations

The `CharacterCustomizationManager` manages the active loadout registry:

- **Save Current As Loadout (`SaveCurrentAsLoadout(const std::string& name)`)**:
  - Captures the active or confirmed appearance.
  - Assigns a UUID/ID and writes timestamp metadata.
  - Persists directly to the local disk file `user_data/character/appearance_loadouts.json`.
- **Apply Loadout (`ApplyLoadout(const std::string& loadoutId)`)**:
  - Validates that the loadout exists and all referenced cosmetic items are registered.
  - Updates the active player appearance.
  - Dispatches an audio confirmation event (`AudioEventId::UI_CLICK`).
- **Delete Loadout (`DeleteLoadout(const std::string& loadoutId)`)**:
  - Removes the loadout from the collection.
  - Prevents deletion of the system default preset if marked immutable.
- **Rename Loadout (`RenameLoadout(const std::string& loadoutId, const std::string& newName)`)**:
  - Updates display name and refreshes persistence file.

---

## 3. Presets and Defaults

On first launch, if no saved file exists at `user_data/character/appearance_loadouts.json`, the serializer seeds default presets:
1. **Default Adventurer**: Classic linen tunic, brown boots, short hair, natural skin tone.
2. **Iron Knight**: Heavy plate armor, iron bascinet, chainmail leggings.
3. **Shadow Rogue**: Dark cowl, leather jerkin, hooded cloak, midnight hair.

Players can switch between presets with a single click in the Wardrobe UI "Loadouts" tab.

