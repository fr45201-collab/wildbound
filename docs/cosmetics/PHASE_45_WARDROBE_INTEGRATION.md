# WILDBOUND — PHASE 45: WARDROBE INTEGRATION SPECIFICATION

## 1. Overview & Objectives

The Wardrobe interface provides players with an interactive 3D/2D visual character preview where they can inspect owned cosmetics, organize equipment slots, and persist customized visual presets.

In Phase 45, the wardrobe seamlessly integrates with:
1. `wildbound::entities::ModularCharacter` for real-time visual layer composite rendering.
2. `wildbound::cosmetics::CosmeticsOwnershipManager` for owned inventory filtering.
3. `wildbound::network::http::BackendClient` for persistent backend loadout synchronization.
4. `wildbound::network::ENetServer` / `DedicatedServer` for multiplayer appearance synchronization.

---

## 2. Wardrobe Data Flow

```
+-------------------------------------------------------------------------+
|                        WARDROBE USER INTERFACE                          |
|                                                                         |
|  [Slot Selector] -> [Item Gallery (Owned Only)] -> [Character Preview]   |
|         |                        |                          |           |
|         v                        v                          v           |
|  Select "Chest"           Select "Forest Tunic"     Render Composite    |
+----------------------------------|--------------------------------------+
                                   |
                                   v
+-------------------------------------------------------------------------+
|                    COSMETICS STORE & LOADOUT MANAGER                     |
|                                                                         |
| 1. Verify Ownership: CosmeticsOwnershipManager::IsOwned("clo_forest_...")|
| 2. Check Concurrency: LoadoutVersion == CurrentExpectedVersion          |
| 3. Send HTTP Request: BackendClient::EquipLoadoutItem(...)              |
+----------------------------------|--------------------------------------+
                                   |
                                   v
+-------------------------------------------------------------------------+
|                            GO REST BACKEND                              |
|                                                                         |
| Endpoint: POST /api/v1/players/me/loadout/equip                          |
| 1. Validate JWT session -> extract UserID                               |
| 2. Validate cosmetic existence and slot definition in PostgreSQL        |
| 3. Verify user ownership in `player_cosmetics`                          |
| 4. In PostgreSQL Transaction:                                           |
|    - Verify `player_loadouts.loadout_version == expected_version`       |
|    - Increment `loadout_version = loadout_version + 1`                  |
|    - Upsert `player_loadout_items` for target slot                      |
|    - Set `player_cosmetics.is_equipped = true`                          |
| 5. Return updated loadout JSON with new version and all slots           |
+-------------------------------------------------------------------------+
```

---

## 3. Slot Mapping & Compatibility

The wardrobe enforces strict slot-to-slot mapping between catalogue definitions and character equipment:

| Wardrobe Slot ID | Database `equipment_slot` | Character Equipment Layer | Asset Sheet Prefix |
|---|---|---|---|
| `head` | `Head` | `CharacterEquipment::Slot::Head` | `sheet_head_` |
| `chest` | `Chest` | `CharacterEquipment::Slot::Chest` | `sheet_clo_` |
| `legs` | `Legs` | `CharacterEquipment::Slot::Legs` | `sheet_clo_p_` |
| `boots` | `Boots` | `CharacterEquipment::Slot::Boots` | `sheet_bot_` |
| `back` | `Back` | `CharacterEquipment::Slot::Back` | `sheet_bac_` |
| `weapon` | `Weapon` | `CharacterEquipment::Slot::Weapon` | `sheet_wep_` |
| `accessory` | `Accessory` | `CharacterEquipment::Slot::Accessory` | `sheet_acc_` |

Slot constraints:
- Equipping an item into an occupied slot atomically replaces the prior item.
- The previous item's `player_cosmetics.is_equipped` flag is set to `false`.
- Emptying a slot via `UnequipSlot` removes the item row and clears the modular character render layer.

---

## 4. Visual Preview & Instant Feedback

1. **Immediate Visual Preview**: When hovering or clicking an item in the wardrobe, the preview model temporarily mounts the asset sprite sheet.
2. **Commit on Confirm**: Clicking "Apply Loadout" or "Equip" commits the change via HTTP.
3. **Rollback on Error**: If the backend returns an error (e.g. HTTP 403 Unowned, HTTP 409 Conflict, or network timeout), the preview model immediately reverts to the server-verified active loadout.

