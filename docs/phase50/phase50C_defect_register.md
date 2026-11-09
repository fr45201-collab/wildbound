# Phase 50C: Consolidated Defect Register & Resolution Planning (Reconciled)

**Project:** Wildbound  
**Phase:** 50C — Core Engine & Systems Integration Validation  
**Date:** 2026-11-08  
**Reconciliation Version:** 1.1 (Officially Reconciled)  

---

## 1. Master Defect Register

| Defect ID | Severity | Phase Discovered | Subsystem | Historical Defect Title & Scope | Current Status in Phase 50C | Recommended Target Phase |
|---|:---:|:---:|---|---|:---:|:---:|
| **DEF-001** | HIGH | Phase 50A | Build / Linker | Test target asymmetry and unresolved `BackendClient` linker errors in 5 test executables | **RESOLVED & CLOSED** | N/A (Closed in Phase 50B) |
| **DEF-002** | MEDIUM | Phase 50A | Dedicated Server | Host environment UDP loopback socket port binding restrictions (`17777`, `19992`) | **CONFIRMED (ENVIRONMENT)** | Phase 50E (Multiplayer) |
| **DEF-003** | MEDIUM | Phase 50A | Asset Manifest | Manifest schema key divergence in Phase 31 storm assets (`"path"` vs `"file"`) | **CONFIRMED & ISOLATED** | Phase 50D (Assets) |
| **DEF-004** | MEDIUM | Phase 50A | Lua Scripting / AI | Missing specialized Lua script archetypes for Phase 49 creatures (`slime.lua`, `snake.lua`, `cyclops.lua`) | **OPEN (HISTORICAL PRESERVED)** | Phase 50E (AI Scripting) |
| **DEF-005** | LOW | Phase 50A | World / Biomes | Biome tileset visual availability vs master world map layer integration | **RESOLVED & CLOSED** | N/A (Closed in Phase 50C) |
| **DEF-006** | MEDIUM | Phase 50A | Coordinator / Load | Multi-process ENet loopback socket communication failure in container sandbox | **CONFIRMED (ENVIRONMENT)** | Phase 50E (Multiplayer) |
| **DEF-007** | LOW | Phase 50A | Asset Manifest | Asset manifest metadata & attribution schema inconsistency across project phases | **CONFIRMED & ISOLATED** | Phase 50D (Assets) |
| **DEF-008** | LOW | Phase 50B | Match Lifecycle | Coincident elimination timestamp tie-break assertion failure in Debug (`test_match_lifecycle.cpp:345`) | **CONFIRMED & ISOLATED** | Phase 50D / 50E |
| **DEF-009** | LOW | Phase 50B | Network Transport | Handshake poll loop iteration exhaustion under unoptimized Debug runtime (`test_network_transport.cpp:231`) | **CONFIRMED & ISOLATED** | Phase 50D / 50E |

---

## 2. Detailed Reconciled Defect Profiles

### 2.1 DEF-003: Asset Manifest Key Divergence (`"path"` vs `"file"`)
- **Severity**: MEDIUM
- **Subsystem**: Asset Management System (`assets/manifests/asset_manifest.json`, `src/resources/AssetMetadata.cpp`)
- **Origin**: Phase 31 (Corruption Storm & Safe Zone Mechanics).
- **Exact Mechanism**: When storm assets were added to `asset_manifest.json`, 21 entries were written using schema key `"path"`:
  ```json
  { "id": "tex_storm_overlay", "path": "environment/storm/storm_overlay.png", "type": "texture" }
  ```
  The remaining 680 assets and `AssetMetadata::FromJson` (`src/resources/AssetMetadata.cpp:32`) expect key `"file"`:
  ```cpp
  meta.file = j.value("file", "");
  ```
  Because `"file"` is absent in these 21 entries, `meta.file` is initialized to empty string `""`. In `AssetRegistry::RegisterAsset`, line 25 rejects the asset:
  ```cpp
  if (metadata.file.empty()) {
      Logger::Error("ASSET_REGISTRY", "Cannot register asset '" + metadata.id + "' with empty file path.");
      return false;
  }
  ```
- **Physical Verification**: All 21 files physically exist on disk in `assets/environment/storm/`, `assets/ui/icons/storm/`, and `assets/audio/`.
- **Impact**: AMS registers 680 of 701 manifest assets.
- **Recommended Remediation (Phase 50D)**: Update `AssetMetadata::FromJson` with a backward-compatible fallback:
  ```cpp
  meta.file = j.value("file", j.value("path", ""));
  ```
  and normalize all 21 JSON entries in `asset_manifest.json` to `"file"`.

---

### 2.2 DEF-004: Missing Specialized Creature Lua Script Archetypes
- **Severity**: MEDIUM
- **Subsystem**: Scripting / AI Subsystem (`scripts/gameplay/ai/`)
- **Historical Origin**: Identified during Phase 50A repository audit.
- **Exact Scope**: Phase 49 introduced artwork and animations for 3 new creatures:
  1. Marsh Slime (`tex_creature_slime`)
  2. Marsh Viper / Snake (`tex_creature_snake`)
  3. Mountain Cyclops (`tex_creature_cyclops`)
  While their spritesheets, animations, and C++ fallback AI behaviors exist, their specialized Lua behavior scripts (`scripts/gameplay/ai/slime.lua`, `scripts/gameplay/ai/snake.lua`, `scripts/gameplay/ai/cyclops.lua`) have not yet been authored. Existing goblins and skeletons operate under dedicated Lua scripts.
- **Reconciliation Note**: The provisional Phase 50C report inadvertently conflated DEF-004 with biome tileset testing. Historical DEF-004 is hereby preserved in its correct AI scripting scope.
- **Recommended Remediation (Phase 50E)**: Author the 3 missing creature behavior scripts and integrate them with `LuaManager`.

---

### 2.3 DEF-005: Five-Biome Regional Integration into Master Island
- **Severity**: LOW
- **Subsystem**: World & Map Loader (`maps/world/battle_royale_island.json`)
- **Historical Scope**: Phase 50A hypothesized that the master Battle Royale island only contained forest terrain, while Desert, Snow, Swamp, and Mountain biomes existed only as isolated test maps (`desert_01.json`, etc.).
- **Audit Findings in Phase 50C**: A complete GID census of `maps/world/battle_royale_island.json` proved this hypothesis incorrect. All 5 biomes are actively painted into the master island map:
  - **Forest**: 121,210 tiles (75.80%)
  - **Desert**: 16,290 tiles (10.19%)
  - **Snow**: 8,206 tiles (5.13%)
  - **Swamp**: 8,015 tiles (5.01%)
  - **Mountain**: 6,195 tiles (3.87%)
  - **Total**: Exactly **159,916 tiles** across all 6 tile layers.
- **Status**: **RESOLVED & CLOSED**.

---

### 2.4 DEF-007: Asset Manifest Attribution & Metadata Schema Hygiene
- **Severity**: LOW
- **Subsystem**: Asset Manifest (`assets/manifests/asset_manifest.json`)
- **Historical Origin**: Identified during Phase 49 Final Acceptance Audit and Phase 50A.
- **Exact Scope & Relationship to DEF-003**:
  - **DEF-003** is a *functional parser defect* (`"path"` vs `"file"`) that directly prevents 21 storm assets from being registered by AMS.
  - **DEF-007** is a *metadata schema hygiene issue*. Early manifest entries (Phases 1–30) contain basic metadata fields (`id`, `file`, `type`, `category`) with placeholder attribution records (`"Wildbound Art Team"`). In Phase 49, a strict metadata schema was introduced requiring explicit fields: `source`, `author`, `license`, `commercial_use`, `attribution_required`, `checksum`, `width`, `height`, and `status`. Some pre-existing entries have incomplete license URLs or divergent metadata nesting (e.g. nested `"metadata"` sub-object in storm assets).
  - DEF-007 does *not* prevent assets from loading at runtime, but represents metadata schema drift.
- **Recommended Remediation (Phase 50D)**: Perform a uniform schema pass across `asset_manifest.json` ensuring all 701 entries comply with the standardized Phase 49 metadata schema.

---

### 2.5 DEF-008: Coincident Elimination Placement Tie-Breaker in Debug
- **Severity**: LOW
- **Subsystem**: Match Results / Tournament Ranking (`tests/match/test_match_lifecycle.cpp:345`)
- **Root Cause**: Test 6 eliminates P3 and P2 with zero elapsed time between them ($t = 0.0\text{s}$). `ResultCalculator::CalculatePlacements` evaluates timestamp differences $< 0.001\text{s}$ as tied rank #2. Active assertion in Debug mode expects rank #3.
- **Recommended Remediation (Phase 50D/50E)**: Insert `mgr.Update(10.0f);` between eliminations in `test_match_lifecycle.cpp`.

---

### 2.6 DEF-009: Debug Network Transport Poll Iteration Timeout
- **Severity**: LOW
- **Subsystem**: Network Transport Unit Test (`tests/network/test_network_transport.cpp:231`)
- **Root Cause**: Polling loop counter expires before ENet packet round-trip processing completes under unoptimized Debug runtime instrumentation (`/RTC1`).
- **Recommended Remediation (Phase 50D/50E)**: Increase maximum poll iteration counter in `test_network_transport.cpp` when compiled under `_DEBUG`.
