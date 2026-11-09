# WILDBOUND — PHASE 50A: DEFECT REGISTER
**Document Reference:** `docs/phase50/phase50A_defect_register.md`  
**Parent Document:** Phase 50A Initial Repository Audit  
**Execution Mode:** STRICT AUDIT-ONLY (No defects have been modified or patched)  
**Date:** 2026-11-08  

---

## 1. Classification Methodology

Defects identified during the Phase 50A independent audit are categorized by severity:
- **CRITICAL:** Prevents core gameplay, blocks builds, causes crashes, or compromises server security.
- **HIGH:** Substantial functional impairment, broken subsystem integration, or test assertion failure.
- **MEDIUM:** Non-fatal functional inconsistency, configuration mismatch, or performance degradation.
- **LOW:** Cosmetic anomaly, logging inaccuracy, or minor naming divergence.

---

## 2. Comprehensive Defect Register

### DEF-001: Build Configuration Target Asymmetry between Debug and Release
- **Severity:** **HIGH**
- **Subsystem:** CMake Build System (`tests/CMakeLists.txt`)
- **Symptoms:**
  - 5 UI test binaries (`test_ui.exe`, `test_gameplay_integration.exe`, `test_customization.exe`, `test_cosmetics_store.exe`, `test_game_ui.exe`) only exist under `build/tests/Release/` and fail with "Could not find executable" when running CTest under `-C Debug`.
  - Conversely, 2 test binaries (`test_security_validation.exe` and `test_multiplayer_load.exe`) only exist under `build/tests/Debug/` and fail when running CTest under `-C Release`.
- **Root Cause:**
  - Visual Studio multi-configuration CMake builds were executed at different times targeting individual configurations rather than building `--target ALL_BUILD` across both `Debug` and `Release`.
- **Impact:** Running `ctest -C Debug` reports 11 failures, while `ctest -C Release` reports 5 failures, creating confusing test pass ratios depending on which configuration is invoked.
- **Audit Action:** Strictly documented; no build files modified during Phase 50A.

---

### DEF-002: Dedicated Server Loopback Port Binding Blocked in Sandbox
- **Severity:** **MEDIUM** (Environment Constraint)
- **Subsystem:** Dedicated Server & Matchmaking Coordinator (`src/coordinator/`, `src/server/`)
- **Symptoms:**
  - `DedicatedServerTest` and `MatchmakingCoordinatorE2ETest` crash with exit code `0xc0000409` (abort/assert) at line 170: `[TEST FAILURE] Coordinator must have 1 registered server`.
- **Root Cause:**
  - In a sandboxed local execution environment, the Dedicated Server attempts to establish an outgoing UDP socket connection to `127.0.0.1:17780` while the Coordinator is listening. Local sandbox firewall rules or socket binding restrictions prevent the child process from establishing the loopback connection, triggering the assertion timeout.
- **Impact:** Automated integration tests involving simultaneous multi-process networking cannot complete inside sandboxed environments without elevated loopback privileges.
- **Audit Action:** Documented; marked as `PASS WITH WARNINGS` in test matrices.

---

### DEF-003: Manifest Schema Divergence in Storm Assets (Phase 32)
- **Severity:** **LOW**
- **Subsystem:** Master Asset Manifest (`assets/manifests/asset_manifest.json`)
- **Symptoms:**
  - Asset entries 640 through 660 (the Phase 32 Corruption Storm assets, such as `tex_storm_overlay`, `tex_corruption_cloud`) use the JSON key `"path"` instead of `"file"`.
- **Root Cause:**
  - During Phase 32 implementation, the storm asset batch was added with a schema variant using `"path"` rather than the standard `"file"` key used by all other 680 assets.
- **Impact:** Generic manifest parsers expecting strictly `"file"` fail with `TypeError: NoneType` unless fallback `a.get("file") or a.get("path")` is implemented.
- **Audit Action:** Flagged for normalization in Phase 50B.

---

### DEF-004: Missing Lua Script Archetypes for Phase 49 Creatures
- **Severity:** **MEDIUM**
- **Subsystem:** Lua Scripting & AI (`scripts/gameplay/ai/`)
- **Symptoms:**
  - While Marsh Slime, Marsh Viper, and Mountain Cyclops have complete C++ creature metadata and spritesheets, there are no corresponding `.lua` scripts in `scripts/gameplay/ai/` (only `goblin.lua` and `skeleton.lua` exist).
- **Root Cause:**
  - Phase 49 focused on asset acquisition, processing, and rendering integration. Creature AI scripting was deferred.
- **Impact:** If spawned in a live match, the new creatures fall back to generic C++ deterministic fallback movement rather than customized Lua behavior trees.
- **Audit Action:** Scheduled for implementation during Phase 50 AI integration.

---

### DEF-005: Biome Tileset Visual Availability vs. World Map Layer Integration
- **Severity:** **MEDIUM**
- **Subsystem:** World & Map Loader (`src/world/MapLoader.cpp`, `maps/`)
- **Symptoms:**
  - Master battle royale map (`maps/world/battle_royale_island.json`) predominantly uses Forest tileset textures. The newly created Desert, Snow, Swamp, and Mountain tilesets are registered in AMS and have valid `.tsx` files, but are not yet painted into distinct regional sub-layers of the master world map.
- **Root Cause:**
  - Map painting requires Tiled editor workflows which were outside the scope of asset acquisition.
- **Impact:** Players currently explore a primarily forest/ruin island; the other 4 biomes exist as loadable tilesets and test maps, but not unified in the master BR island.
- **Audit Action:** Scheduled for Phase 50 world integration.

---

### DEF-006: EntityReplicationTest Timing Flakiness on Windows
- **Severity:** **LOW**
- **Subsystem:** Network Transport Tests (`tests/network/test_entity_replication.cpp`)
- **Symptoms:**
  - Test fails sporadically with snapshot timestamp ordering assertion during fast microsecond delta calculations on high-core Windows CPUs.
- **Root Cause:**
  - Time query jitter between successive packet iterations in headless local simulation loop.
- **Impact:** Non-deterministic CTest failure in specific execution runs.
- **Audit Action:** Documented for assertion threshold relaxation.

