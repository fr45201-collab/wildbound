# Wildbound — Persistent Equipped Loadouts & Concurrency Control

## 1. Overview

Players customize their appearance across 9 discrete visual equipment slots. Phase 45 establishes persistent, transaction-protected equipped loadouts with optimistic concurrency control in PostgreSQL.

---

## 2. Equipment Slots & Compatibility Rules

| Slot | Description | Compatible Categories | Mutex Constraints |
| :--- | :--- | :--- | :--- |
| **Skin** | Base body complexion | `character_variations`, `skin` | Exactly 1 active skin tone |
| **Hair** | Hairstyle and color | `character_variations`, `hair` | Replaced if helmet has `hair_hide_all` |
| **Headwear**| Hats, caps, hoods | `headwear` | Conflicts with full-face helmets |
| **Helmet** | Heavy armored helms | `headwear`, `helmet` | Occludes or replaces top hair layers |
| **Face** | Masks, goggles, warpaint | `face_accessories` | May hide eyewear or lower face |
| **Chest** | Tunics, shirts, robes, armor| `clothing`, `armor` | Top layer rendered over lower body |
| **Legs** | Trousers, pants, leggings | `pants` | Rendered under shirts and above footwear |
| **Feet** | Boots, shoes, sandals | `footwear` | Rendered at base of legs |
| **Back** | Backpacks, quivers, packs | `backpacks` | Rendered behind character torso |

---

## 3. Concurrency Versioning (`loadout_version`)

To prevent race conditions where older requests overwrite newer loadouts from another device:
- The `player_loadouts` table stores `loadout_version INT NOT NULL DEFAULT 1`.
- Equip and unequip requests accept an optional `expected_version` integer.
- If `expected_version > 0` and does not match the active `loadout_version` in the database, the transaction aborts with HTTP 409 Conflict:
  ```json
  {
    "success": false,
    "data": null,
    "error": {
      "code": "LOADOUT_VERSION_CONFLICT",
      "message": "Stale loadout version detected"
    }
  }
  ```
- Every successful loadout modification increments `loadout_version` by 1.

---

## 4. Loadout Endpoints

### 4.1 `GET /api/v1/players/me/loadout`
Returns the active loadout version, timestamp, and map of equipped slots to items.

### 4.2 `POST /api/v1/players/me/loadout/equip`
Equips a single owned item to its compatible slot, replacing any existing item in that slot transactionally.

### 4.3 `POST /api/v1/players/me/loadout/unequip`
Removes the cosmetic currently occupying the specified slot.

### 4.4 `PUT /api/v1/players/me/loadout`
Atomically replaces all equipped slots with the submitted array of owned cosmetic IDs.

