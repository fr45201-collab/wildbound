# Wildbound — Cosmetic Catalogue API Specification

## 1. Overview

The cosmetic catalogue is authoritatively hosted on `WildboundBackend`, exposing all available public cosmetics, equipment slots, categories, and asset bindings for store and wardrobe consumption.

---

## 2. Endpoint Specification

### `GET /api/v1/cosmetics`
- **Authentication**: None (Public)
- **Query Parameters**:
  - `category` (optional string): Filter by cosmetic category (e.g. `clothing`, `armor`, `pants`, `footwear`, `headwear`, `face_accessories`, `backpacks`, `character_variations`).
  - `slot` (optional string): Filter by equipment slot (`Skin`, `Hair`, `Headwear`, `Helmet`, `Face`, `Chest`, `Legs`, `Feet`, `Back`).
  - `rarity` (optional string): Filter by rarity tier (`common`, `uncommon`, `rare`, `epic`, `legendary`).
  - `search` (optional string): Case-insensitive substring match against `display_name` or `description`.
  - `limit` (optional integer, default 50, max 100): Pagination page size.
  - `offset` (optional integer, default 0): Pagination offset.

### 2.1 Response Payload
```json
{
  "success": true,
  "data": {
    "items": [
      {
        "id": "clo_basic_traveler_001",
        "cosmetic_key": "clo_basic_traveler_001",
        "display_name": "Basic Traveler Shirt",
        "category": "clothing",
        "equipment_slot": "Chest",
        "rarity": "common",
        "description": "Linen traveler shirt with laced neck and rolled brown cuffs.",
        "asset_id": "sheet_clo_basic_traveler_001",
        "is_available": true,
        "created_at": "2026-11-08T00:55:00Z"
      }
    ],
    "total": 62,
    "limit": 50,
    "offset": 0
  }
}
```

---

## 3. Filtering & Asset Resolution

- Public queries only return entries where `is_available = true`.
- Each catalogue item returns `asset_id`, allowing the C++ client to resolve textures and spritesheets via the Asset Management System (AMS) without hardcoded disk paths.

