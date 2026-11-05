# Appearance Persistence

## Overview
All player appearance choices and saved loadouts are persisted locally in structured JSON format. In compliance with the local-first architecture of Wildbound, persistence is strictly client-side without relying on external network backends or cloud databases.

---

## 1. File Location and Schema

### Storage Path
`user_data/character/appearance_loadouts.json`

Parent directories (`user_data/character/`) are created automatically at runtime if they do not exist.

### JSON Schema
```json
{
  "version": 1,
  "activeLoadoutId": "loadout_001",
  "loadouts": [
    {
      "id": "loadout_001",
      "name": "Classic Adventurer",
      "createdAt": "2026-11-05T00:00:00Z",
      "updatedAt": "2026-11-05T00:00:00Z",
      "appearance": {
        "skinId": "cosmetic_skin_default",
        "hairId": "cosmetic_hair_short",
        "headwearId": "",
        "helmetId": "",
        "faceId": "",
        "chestId": "cosmetic_tunic_adventurer",
        "legsId": "cosmetic_pants_leather",
        "feetId": "cosmetic_boots_leather",
        "backId": "",
        "skinColor": [245, 205, 175, 255],
        "hairColor": [80, 50, 30, 255]
      }
    }
  ]
}
```

---

## 2. Serialization and Error Handling

`AppearanceSerializer` manages disk I/O with fault tolerance:

- **Atomic Writes**: Serializes to a clean in-memory `nlohmann::json` buffer before flushing to file.
- **Corrupted File Recovery**: If `appearance_loadouts.json` contains malformed JSON or invalid schema values:
  - An error log `[APPEARANCE_SERIALIZER] Failed to parse JSON, falling back to defaults` is generated.
  - Builtin default loadouts are instantiated in memory to prevent crashes.
  - The corrupted file is safely overwritten with valid data upon the next save.
- **Missing Asset Fallback**: If a saved cosmetic ID no longer exists in `CosmeticRegistry` (e.g. from an asset patch change), the serializer falls back to an empty slot or default base item, preserving game integrity.

