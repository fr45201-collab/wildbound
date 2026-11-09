# Phase 50B: Initial Build Audit & DEF-001 Investigation

**Project:** Wildbound  
**Phase:** 50B — Build System Harmonization & Test Infrastructure Validation  
**Date:** 2026-11-08  
**Operating System:** Windows 11 (Build 26200)  
**Compiler:** MSVC 19.51.36248.0 (Visual Studio 2026/v18.7)  
**Build System:** CMake 4.3.0  
**Repository HEAD:** `5ec8ec01665e4fe478e26a7c0ec0fbca21e04428`  
**Branch:** `main`

---

## 1. Executive Summary

Phase 50A identified a critical discrepancy between Debug and Release test target coverage, classified as **DEF-001 (Build Target Asymmetry & Linker Defect)**:

| Configuration | Total Expected | Built / Passed | Failed | Missing / Unbuilt |
|---|---:|---:|---:|---:|
| Release (Phase 50A Baseline) | 48 | 43 | 2 | 3 |
| Debug (Phase 50A Baseline) | 48 | 37 | 6 | 5 |

An exhaustive initial audit of `CMakeLists.txt`, `tests/CMakeLists.txt`, and compiler/linker diagnostics was conducted to establish the exact root causes of this asymmetry, reconcile build artifacts, and establish a verifiable path toward 100% target synchronization across both configurations.

---

## 2. Root Cause Analysis: Linker Defect DEF-001

### 2.1 Failing Targets
When attempting clean builds of the test suite under the Debug configuration, 5 test executables failed to link:
1. `test_ui` (`tests/ui/test_ui.cpp`)
2. `test_gameplay_integration` (`tests/integration/test_gameplay_integration.cpp`)
3. `test_customization` (`tests/customization/test_customization.cpp`)
4. `test_cosmetics_store` (`tests/cosmetics/test_cosmetics_store.cpp`)
5. `test_game_ui` (`tests/ui/test_game_ui.cpp`)

### 2.2 Exact Linker Diagnostics
All five targets generated identical MSVC linker errors:
```
CosmeticsOwnershipManager.obj : error LNK2019: unresolved external symbol "public: void __cdecl wildbound::network::http::BackendClient::ListMyCosmetics(class std::function<void __cdecl(bool,class std::vector<struct wildbound::network::http::CosmeticItemDto,class std::allocator<struct wildbound::network::http::CosmeticItemDto> > const &)>)" (?ListMyCosmetics@BackendClient@http@network@wildbound@@QEAAXV?$function@$$A6AX_NAEBV?$vector@UCosmeticItemDto@http@network@wildbound@@V?$allocator@UCosmeticItemDto@http@network@wildbound@@@std@@@std@@@std@@@Z) referenced in function "public: void __cdecl wildbound::cosmetics::CosmeticsOwnershipManager::SyncWithBackend(class std::function<void __cdecl(bool)>)" (?SyncWithBackend@CosmeticsOwnershipManager@cosmetics@wildbound@@QEAAXV?$function@$$A6AX_N@Z@std@@@Z)

CosmeticsOwnershipManager.obj : error LNK2019: unresolved external symbol "public: void __cdecl wildbound::network::http::BackendClient::EquipLoadoutItem(class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &,class std::function<void __cdecl(bool,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &)>)"

CosmeticsOwnershipManager.obj : error LNK2019: unresolved external symbol "public: void __cdecl wildbound::network::http::BackendClient::UnequipLoadoutSlot(class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &,class std::function<void __cdecl(bool,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &)>)"
```

### 2.3 Mechanism of Failure
During Phase 45 (Live Service & Cosmetics Backend Integration), `src/cosmetics/CosmeticsOwnershipManager.cpp` and `src/cosmetics/CosmeticsStoreManager.cpp` were updated to integrate directly with `wildbound::network::http::BackendClient` (`src/network/http/BackendClient.cpp`).

However:
1. `tests/CMakeLists.txt` included `CosmeticsOwnershipManager.cpp` and `CosmeticsStoreManager.cpp` in the source lists for `test_ui`, `test_gameplay_integration`, `test_customization`, `test_cosmetics_store`, and `test_game_ui`.
2. `tests/CMakeLists.txt` **omitted** `${CMAKE_SOURCE_DIR}/src/network/http/BackendClient.cpp` and `${CMAKE_SOURCE_DIR}/src/network/http/HttpClient.cpp` from those 5 target definitions.
3. Furthermore, `winhttp` was not linked into those 5 targets.
4. In the Release build directory, pre-Phase 45 compiled binaries existed from previous test runs (`2026-11-07`), which allowed CTest to invoke stale binaries, masking the linker failure during target-by-target builds, whereas the Debug build attempted a clean compilation and encountered the unresolved symbols immediately.

---

## 3. Secondary Asymmetry: Missing Phase 47/48 Targets in Release

Inspection of the Release build directory revealed that two executables:
- `test_security_validation.exe` (Phase 47 — Security & Anti-Cheat Foundations)
- `test_multiplayer_load.exe` (Phase 48 — Multiplayer Load & Stress Testing)

existed in `build/tests/Debug/` but had not been compiled in `build/tests/Release/`. This occurred because CMake project builds had been invoked with explicit `--target` flags during Phase 47 and Phase 48 targeting Debug only.

---

## 4. Reconciled Baseline Audit Matrix

Prior to applying corrective engineering in Phase 50B, the verified physical state of the repository targets was:

| Target Name | Defined in CMake? | Debug Binary Present? | Release Binary Present? | Linker Failure Identified |
|---|:---:|:---:|:---:|---|
| `test_config` | Yes | Yes | Yes | None |
| `test_lua` | Yes | Yes | Yes | None |
| `test_rendering` | Yes | Yes | Yes | None |
| `test_assets` | Yes | Yes | Yes | None |
| `test_world` | Yes | Yes | Yes | None |
| `test_player` | Yes | Yes | Yes | None |
| `test_combat` | Yes | Yes | Yes | None |
| `test_modular_character` | Yes | Yes | Yes | None |
| `test_production_cosmetics` | Yes | Yes | Yes | None |
| `test_creatures` | Yes | Yes | Yes | None |
| `test_environment_assets` | Yes | Yes | Yes | None |
| `test_environmental_animation` | Yes | Yes | Yes | None |
| `test_village_buildings` | Yes | Yes | Yes | None |
| `test_biomes` | Yes | Yes | Yes | None |
| `test_weapons` | Yes | Yes | Yes | None |
| `test_items` | Yes | Yes | Yes | None |
| `test_costumes_armor` | Yes | Yes | Yes | None |
| `test_audio` | Yes | Yes | Yes | None |
| `test_ui` | Yes | **No (Link Error)** | Stale | Missing `BackendClient.cpp`, `HttpClient.cpp`, `winhttp` |
| `test_gameplay_integration` | Yes | **No (Link Error)** | Stale | Missing `BackendClient.cpp`, `HttpClient.cpp`, `winhttp` |
| `test_collision` | Yes | Yes | Yes | None |
| `test_input` | Yes | Yes | Yes | None |
| `test_interaction` | Yes | Yes | Yes | None |
| `test_inventory` | Yes | Yes | Yes | None |
| `test_weapons_equipment` | Yes | Yes | Yes | None |
| `test_customization` | Yes | **No (Link Error)** | Stale | Missing `BackendClient.cpp`, `HttpClient.cpp`, `winhttp` |
| `test_cosmetics_store` | Yes | **No (Link Error)** | Stale | Missing `BackendClient.cpp`, `HttpClient.cpp`, `winhttp` |
| `test_game_ui` | Yes | **No (Link Error)** | Stale | Missing `BackendClient.cpp`, `HttpClient.cpp`, `winhttp` |
| `test_ai` | Yes | Yes | Yes | None |
| `test_battle_royale_world` | Yes | Yes | Yes | None |
| `test_corruption_storm` | Yes | Yes | Yes | None |
| `test_match_lifecycle` | Yes | Yes | Yes | None |
| `test_match_results` | Yes | Yes | Yes | None |
| `test_game_modes` | Yes | Yes | Yes | None |
| `test_network_architecture` | Yes | Yes | Yes | None |
| `test_network_transport` | Yes | Yes | Yes | None |
| `test_entity_replication` | Yes | Yes | Yes | None |
| `test_dedicated_server` | Yes | Yes | Yes | None |
| `test_matchmaking_sessions` | Yes | Yes | Yes | None |
| `test_matchmaking_lobbies_parties` | Yes | Yes | Yes | None |
| `test_matchmaking_invitations` | Yes | Yes | Yes | None |
| `test_matchmaking_queues` | Yes | Yes | Yes | None |
| `test_matchmaking_server_registry` | Yes | Yes | Yes | None |
| `test_matchmaking_assignment` | Yes | Yes | Yes | None |
| `test_matchmaking_coordinator_e2e` | Yes | Yes | Yes | None |
| `test_security_validation` | Yes | Yes | **No (Unbuilt)** | Target unbuilt in Release |
| `test_multiplayer_load` | Yes | Yes | **No (Unbuilt)** | Target unbuilt in Release |
| `wildbound` | Yes | Yes | Yes | None |
| `WildboundServer` | Yes | Yes | Yes | None |
| `WildboundCoordinator` | Yes | Yes | Yes | None |

---

## 5. Audit Conclusions & Prescribed Remediation

1. **Remediation Plan**:
   - Introduce `BACKEND_HTTP_SOURCES` in `tests/CMakeLists.txt` containing `${CMAKE_SOURCE_DIR}/src/network/http/HttpClient.cpp` and `${CMAKE_SOURCE_DIR}/src/network/http/BackendClient.cpp`.
   - Append `${BACKEND_HTTP_SOURCES}` to `test_ui`, `test_gameplay_integration`, `test_customization`, `test_cosmetics_store`, and `test_game_ui`.
   - Link `winhttp` on `WIN32` for all 5 targets.
   - Execute a coordinated rebuild across both Debug and Release configurations.
   - Develop automated validation scripts in `tools/build_validation/` to ensure regression protection.

