# Wildbound — Phase 45 Initial Repository Audit
## Cosmetic Ownership Integration: Store, Backend, Wardrobe & Server-Side Validation

**Project:** Wildbound  
**Phase:** 45  
**Auditor:** Antigravity  
**Date:** November 2026  
**Status:** Audit Completed — Implementation Plan Formulated

---

## 1. Executive Audit Summary

A comprehensive repository audit was conducted across all cosmetic, character customization, Go backend, database schema, and dedicated server subsystems:
- **Phase 7 & 8 (Modular Character Art & Cosmetics)**: Established `entities::ModularCharacter`, sprite layer ordering, occlusion flags, and sprite definitions.
- **Phase 17 & 19 (Costumes, Armor & Gameplay Integration)**: Cataloged 50 production cosmetics in `data/cosmetics/costume_catalog.json` with spritesheets, directional animations, and textures.
- **Phase 25 (Character Customization)**: Built `customization::CharacterAppearance`, `CosmeticRegistry`, `AppearancePreview`, `CharacterCustomizationManager`, and slot mapping (`CustomizationSlot`).
- **Phase 26 (Cosmetics Store & Wardrobe UI)**: Implemented `cosmetics::CosmeticsStoreManager`, `CosmeticsOwnershipManager`, search filtering, and preview controls. Ownership and loadouts were previously saved to local JSON files (`user_data/cosmetics/cosmetic_ownership.json` and `user_data/character/appearance_loadouts.json`).
- **Phase 42, 43, 44 (Go Backend & Database Infrastructure)**: Initial tables `cosmetic_definitions` and `player_cosmetics` were created in migration `000001` and seeded with 12 items in `000002`. Endpoints for listing catalogue, player cosmetics, and single-item equip were drafted.

### Key Finding & Core Challenge
While rich visual assets, client customization, and store UI exist on the client, and foundational tables exist in the backend, **they are currently decoupled**:
1. The C++ client reads and writes ownership to local file `user_data/cosmetics/cosmetic_ownership.json`, permitting client-side modifications.
2. The database lacks a formal `player_loadouts` / `player_loadout_items` schema with concurrency versioning (`loadout_version`).
3. The dedicated server accepts client connections without inspecting or validating equipped cosmetics against the Go backend.
4. No network message currently synchronizes server-approved character appearance to other match participants.

---

## 2. Existing Cosmetic Architecture (Client)

### 2.1 Identifiers & Data Models
- **Catalog**: Defined in `data/cosmetics/costume_catalog.json` (50 items across Clothing, Armor, Pants, Footwear, Headwear, Face Accessories, Backpacks, Character Variations).
- **ID Convention**: Standard prefixed snake_case strings, e.g.:
  - `clo_basic_traveler_001` (Clothing)
  - `arm_iron_armor_001` (Armor)
  - `clo_p_traveler_pants_001` (Pants)
  - `sho_leather_boots_001` (Footwear)
  - `hat_iron_helmet_001` (Headwear)
  - `fac_adventurer_goggles_001` (Face)
  - `bag_adventurer_backpack_001` (Backpack)
  - `var_skin_dark_001` (Skin variation)
  - `var_hair_braided_red_001` (Hair variation)
- **Asset Resolution**: All spritesheets are resolved through the Asset Management System (AMS) using keys like `sheet_clo_basic_traveler_001` and textures `tex_clo_basic_traveler_001`.
- **Character Equipment**: Managed by `entities::CharacterEquipment` inside `entities::ModularCharacter`. Layer order:
  `Skin` -> `Hair` -> `Face` -> `Pants` -> `Shoes` -> `Shirt` -> `Armor` -> `Headwear` -> `Back`.

### 2.2 Store & Wardrobe UI
- `CosmeticsStoreManager` switches between `StoreTab::Store` (catalog browsing, prototype unlock/purchase) and `StoreTab::Wardrobe` (owned cosmetics, slot equipping).
- Currently interacts with `CosmeticsOwnershipManager` which persists state to disk via `user_data/cosmetics/cosmetic_ownership.json`.

---

## 3. Verified Backend & Database Infrastructure

### 3.1 Database Tables (PostgreSQL `wildbound_db`)
- `cosmetic_definitions`: `(id, cosmetic_key, display_name, category, rarity, description, is_available, created_at)`.
- `player_cosmetics`: `(id, user_id, cosmetic_id, acquired_at, acquisition_source, is_equipped)`.
- **Limitation**: Currently has 12 items seeded from Phase 42 migration `000002_seed_cosmetics.up.sql`. It does not yet include all 50 items from `costume_catalog.json`.
- **Missing Table**: No dedicated `player_loadouts` table with `loadout_version` tracking multiple equipped slots transactionally.

### 3.2 Existing API Endpoints
- `GET /api/v1/cosmetics`: Lists available cosmetic definitions.
- `GET /api/v1/players/me/cosmetics`: Lists cosmetics owned by calling user.
- `POST /api/v1/players/me/cosmetics/equip`: Equips a single item (flips `is_equipped` flag in `player_cosmetics`).
- `POST /api/v1/internal/cosmetics/grant`: Server-authenticated endpoint to grant cosmetics.

---

## 4. Dedicated Server & Multiplayer Status

### 4.1 Identity Binding
- In Phase 44, `WildboundCoordinator` extracts JWT tokens and populates `persistentUserId` into `ParticipantReservation`.
- `DedicatedServer::SetReservedMatch` stores the reservations mapping tickets to persistent user IDs.
- `DedicatedServer` has an authenticated HTTP client capable of reaching `WildboundBackend` with `X-Server-Token`.

### 4.2 Missing Server-Side Validation
- The dedicated server currently does NOT query the backend for contender equipped cosmetics.
- The dedicated server does NOT validate whether a contender owns what they are wearing.
- `NetworkMessage.hpp` lacks a dedicated `MsgPlayerAppearanceSync` packet to broadcast appearance to all connected peers in the match.

---

## 5. Missing Functionality & Gap Analysis

1. **Database Schema Enhancements (Migration `000005`)**:
   - Seed all 50 production cosmetics from `costume_catalog.json` into `cosmetic_definitions`.
   - Add equipment slot column to `cosmetic_definitions` (`equipment_slot`).
   - Create `player_loadouts` table: `(id, user_id, loadout_version, created_at, updated_at)` with unique constraint on `user_id`.
   - Create `player_loadout_items` table: `(id, loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at)` with unique constraint on `(user_id, equipment_slot)`.
2. **Backend API Extensions**:
   - `GET /api/v1/players/me/loadout`: Returns the full equipped loadout with `loadout_version`.
   - `PUT /api/v1/players/me/loadout`: Atomic multi-slot equip/unequip with optimistic concurrency check (`expected_version`).
   - `POST /api/v1/players/me/loadout/equip`: Transactionally equips an owned cosmetic to its designated slot.
   - `POST /api/v1/players/me/loadout/unequip`: Transactionally unequips a slot.
   - `GET /api/v1/internal/players/{user_id}/loadout`: Internal endpoint for dedicated game servers to fetch verified loadouts.
3. **C++ Dedicated Server Validation**:
   - On match assignment and player connect, server calls `GET /api/v1/internal/players/{user_id}/loadout`.
   - Validates all returned cosmetic IDs against `CosmeticRegistry`.
   - Rejects unowned or invalid cosmetics; enforces default appearance fallback.
   - Broadcasts authoritative appearance to all clients via `MsgPlayerAppearanceSync`.
4. **C++ Client Integration**:
   - Update `BackendClient` to handle loadout endpoints.
   - Connect `CosmeticsStoreManager` and `CharacterCustomizationManager` to synchronize with `BackendClient` instead of solely relying on local JSON.
   - Handle server appearance synchronization in `GameClient` / `MultiplayerMenu`.

---

## 6. Integration Risks & Mitigations

| Risk | Impact | Mitigation |
| :--- | :--- | :--- |
| **Backend Unavailability during Match Startup** | Dedicated server hangs waiting for loadouts | Asynchronous non-blocking HTTP request with 2-second timeout; falls back cleanly to default appearance. Match simulation proceeds uninterrupted. |
| **Stale Loadout Overwrite Race Condition** | Client A overwrites changes made from Client B | Incremental `loadout_version` with optimistic concurrency check; reject conflicting writes with HTTP 409 Conflict (`LOADOUT_VERSION_CONFLICT`). |
| **Cosmetic ID Mismatch between C++ and Go** | Items exist in C++ catalog but missing in PostgreSQL | Synchronize definitions: migration `000005` will seed all 50 items matching `costume_catalog.json` exactly. |
| **Cosmetic Advantage / Armor Exploit** | Players equip heavy armor cosmetics thinking it increases HP | Strict visual decoupling: cosmetics alter presentation only; gameplay stats remain authoritative on the server. |

---

## 7. Proposed Implementation Plan

- **Step 1**: Repository Audit (`PHASE_45_INITIAL_AUDIT.md`) — [Completed].
- **Step 2**: Cosmetic Identifiers & Schema Migration `000005` — Add `player_loadouts`, `player_loadout_items`, seed 50 cosmetics, and document in `PHASE_45_COSMETIC_IDENTIFIERS.md`.
- **Step 3**: Backend Catalogue & Ownership APIs — Implement filters, pagination, ownership endpoints (`PHASE_45_CATALOGUE.md` and `PHASE_45_OWNERSHIP.md`).
- **Step 4**: Backend Loadout System — Transactional equip/unequip, versioning, and internal server endpoint (`PHASE_45_LOADOUT_SYSTEM.md`).
- **Step 5**: Dedicated Server Validation & Network Protocol — Add `MsgPlayerAppearanceSync`, fetch authoritative loadout from backend, and broadcast appearance (`PHASE_45_SERVER_VALIDATION.md` and `PHASE_45_NETWORK_SYNCHRONIZATION.md`).
- **Step 6**: Client Store & Wardrobe Integration — Connect `BackendClient`, `CosmeticsStoreManager`, and `CharacterCustomizationManager` (`PHASE_45_STORE_INTEGRATION.md`, `PHASE_45_WARDROBE_INTEGRATION.md`, `PHASE_45_RENDERING_INTEGRATION.md`).
- **Step 7**: Automated Testing Suite — Go tests, PostgreSQL integration tests, dedicated server tests, security tests, and end-to-end tests (`PHASE_45_SECURITY.md`, `PHASE_45_TESTING.md`).
- **Step 8**: Documentation & Final Report — OpenAPI specification update, complete documentation suite, and `PHASE_45_IMPLEMENTATION_REPORT.md`.

