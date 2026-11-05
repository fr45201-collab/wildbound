# Local Persistence Specification (Phase 26)

## 1. Storage Layout & File Locations

All visual customization and cosmetic ownership data are persisted locally on disk under `user_data/`:

| Subsystem | File Path | Format | Schema Version |
|---|---|---|---|
| **Cosmetic Ownership** | `user_data/cosmetics/cosmetic_ownership.json` | JSON | `1` |
| **Appearance & Loadouts** | `user_data/character/appearance_loadouts.json` | JSON | `1` |

Both directories are created automatically by `std::filesystem::create_directories` upon first run if they do not exist.

---

## 2. Cosmetic Ownership Schema (`cosmetic_ownership.json`)

```json
{
  "version": 1,
  "records": [
    {
      "cosmeticId": "cosmetic_tunic_adventurer",
      "state": "owned",
      "acquisitionSource": "starter",
      "acquisitionDate": "2026-11-05 12:00:00",
      "selectedVariantId": "cloth_forest"
    },
    {
      "cosmeticId": "clo_royal_outfit_001",
      "state": "owned",
      "acquisitionSource": "prototype_unlock",
      "acquisitionDate": "2026-11-05 13:12:45",
      "selectedVariantId": "cloth_royal"
    },
    {
      "cosmeticId": "arm_ancient_rune_001",
      "state": "locked",
      "acquisitionSource": "none",
      "acquisitionDate": "",
      "selectedVariantId": "variant_default"
    }
  ]
}
```

---

## 3. Atomic Serialization & Error Resilience

1. **Graceful Loading Fallback**:
   If `user_data/cosmetics/cosmetic_ownership.json` is missing, corrupted, or invalid:
   - An error or warning is logged.
   - The ownership manager immediately initializes with canonical starter equipment.
   - A fresh valid file is serialized to disk.
2. **Re-seeding Invariant**:
   Even if an older ownership file omits newly added starter gear, `SeedDefaults()` runs post-load to guarantee all canonical starter cosmetics remain accessible.
3. **Reset Operation**:
   `ResetOwnership()` clears all records, re-seeds starter defaults, and immediately saves to disk.

