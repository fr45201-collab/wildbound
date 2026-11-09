# Phase 50D: Master Defect Register & Resolution Tracking

**Project:** Wildbound  
**Phase:** 50D — Asset Library Consolidation & Pipeline Standardization  
**Date:** 2026-11-08  
**Defect Register Version:** 1.2 (Post-Phase 50D Update)

---

## 1. Master Defect Status Register

| Defect ID | Severity | Phase Discovered | Subsystem | Historical Defect Title & Scope | Status Post-Phase 50D | Target Phase / Resolution Notes |
|---|:---:|:---:|---|---|:---:|---|
| **DEF-001** | HIGH | Phase 50A | Build / Linker | Test target asymmetry and unresolved `BackendClient` linker errors in 5 test executables | **RESOLVED & CLOSED** | Closed in Phase 50B |
| **DEF-002** | MEDIUM | Phase 50A | Dedicated Server | Host environment UDP loopback socket port binding restrictions (`17777`, `19992`) | **ENVIRONMENT BLOCKED** | Preserved (External sandbox loopback policy) |
| **DEF-003** | MEDIUM | Phase 50A | Asset Manifest | Manifest parser key divergence in Phase 31 storm assets (`"path"` vs `"file"`) | **RESOLVED & CLOSED** | **Closed in Phase 50D**: C++ parser hardened; manifest normalized. 701/701 assets registered. |
| **DEF-004** | MEDIUM | Phase 50A | Lua Scripting / AI | Missing specialized Lua script archetypes for Phase 49 creatures (`slime.lua`, `snake.lua`, `cyclops.lua`) | **OPEN** | Scheduled for Phase 50E (Creature AI Scripting) |
| **DEF-005** | LOW | Phase 50A | World / Biomes | Biome tileset visual availability vs master world map layer integration | **RESOLVED & CLOSED** | Closed in Phase 50C (159,916 tiles across 5 biomes verified) |
| **DEF-006** | MEDIUM | Phase 50A | Coordinator / Load | Multi-process ENet loopback socket communication failure in container sandbox | **ENVIRONMENT BLOCKED** | Preserved (External sandbox loopback policy) |
| **DEF-007** | LOW | Phase 50A | Asset Manifest | Asset manifest metadata & attribution schema drift across project cohorts | **RESOLVED & CLOSED** | **Closed in Phase 50D**: Schema v1.1.0 defined; 701/701 entries normalized. Zero drift. |
| **DEF-008** | LOW | Phase 50B | Match Lifecycle | Coincident elimination timestamp tie-break assertion failure in Debug (`test_match_lifecycle.cpp:345`) | **CONFIRMED & ISOLATED** | Debug race condition; Release passes. Scheduled Phase 50E. |
| **DEF-009** | LOW | Phase 50B | Network Transport | Handshake poll loop iteration exhaustion under unoptimized Debug runtime (`test_network_transport.cpp:231`) | **CONFIRMED & ISOLATED** | Handshake loop timeout in Debug; Release passes. Scheduled Phase 50E. |

---

## 2. Phase 50D Defect Resolution Details

### 2.1. DEF-003: Manifest Parser Defect (`"path"` vs `"file"`)
- **Root Cause**: `AssetMetadata::FromJson` strictly parsed `j.value("file", "")`. For the 21 Corruption Storm assets, `"file"` was absent because `"path"` was used. Consequently, `meta.file` remained empty and `AssetRegistry::RegisterAsset` rejected them with `Cannot register asset ... with empty file path`.
- **Engineering Resolution in Phase 50D**:
  1. *C++ Parser Hardening*: Updated `AssetMetadata::FromJson` in `src/resources/AssetMetadata.cpp` with backward-compatible key resolution: prefers canonical `"file"`, falls back to legacy `"path"` with a diagnostic warning, handles conflicting keys gracefully, and extracts nested dimensions.
  2. *Manifest Normalization*: Converted all 21 storm entries in `assets/manifests/asset_manifest.json` from `"path"` to canonical `"file"`.
  3. *Unit Testing*: Added Test 11 in `tests/resources/test_assets.cpp` specifically verifying `"path"` fallback, precedence, and resilience.
- **Verification Result**: `test_assets.exe` registers all **701 of 701 assets** with zero rejections in both Debug and Release.
- **Status**: **RESOLVED & CLOSED**.

### 2.2. DEF-007: Metadata Schema Drift Across Cohorts
- **Root Cause**: Over 49 development phases, manifest entries accumulated inconsistent field naming (`commercialUse` vs `commercial_use`, `attributionRequired` vs `attribution_required`), missing licensing fields in storm assets, and generic `"audio"` type strings.
- **Engineering Resolution in Phase 50D**:
  1. *Formal Schema Specification*: Published canonical Metadata Schema v1.1.0 in `docs/assets/asset_metadata_schema.md` and `docs/phase50/phase50D_metadata_schema.md`.
  2. *In-Place Migration*: Executed `tools/asset_pipeline/normalize_manifest.py` against `asset_manifest.json` (backed up to `.bak_phase50d`). Standardized 152 camelCase licensing fields, 128 `"audio"` classifications, 17 nested dimensions, and populated in-house CC0 licensing for storm assets.
  3. *Comprehensive Licensing Audit*: Programmatic audit in `docs/phase50/phase50D_licensing_audit.md` verified 100% commercial permissibility (673 CC0 1.0 Universal, 28 CC BY 3.0 with complete author attribution).
  4. *Case-Insensitive Status*: Hardened `AssetMetadata::StringToStatus` to support uppercase and lowercase `"approved"`.
- **Verification Result**: `tools/asset_pipeline/phase50D_master_audit.py` confirms 0 legacy keys, 0 missing schema fields, and 100% compliant schema across all 701 assets.
- **Status**: **RESOLVED & CLOSED**.

