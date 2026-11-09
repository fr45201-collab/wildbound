# Phase 50B: Complete Target Inventory & Coverage

**Project:** Wildbound  
**Phase:** 50B — Build System Harmonization & Test Infrastructure Validation  
**Date:** 2026-11-08  
**Total Production Targets:** 3  
**Total Test Targets:** 47  
**Total Registered CTest Tests:** 48  

---

## 1. Production Binary Inventory

| Target Identifier | Type | Output Executable | Primary Subsystems | Linked Libraries |
|---|---|---|---|---|
| `wildbound` | Executable | `build/<Config>/wildbound.exe` | Complete Game Client (Raylib, ImGui, Lua, Audio, World, Network) | raylib, sol2, lua_static, imgui, rlimgui, nlohmann_json, enet, winhttp |
| `WildboundServer` | Executable | `build/<Config>/WildboundServer.exe` | Headless Dedicated Server (Game simulation, authority, network) | raylib, sol2, lua_static, imgui, rlimgui, nlohmann_json, enet, ws2_32, winmm, winhttp |
| `WildboundCoordinator` | Executable | `build/<Config>/WildboundCoordinator.exe` | Headless Matchmaking Coordinator (Lobbies, party, queues, allocation) | enet, nlohmann_json, ws2_32, winmm |

---

## 2. Test Executable & CTest Matrix

| Index | Target Identifier | Executable Binary | Registered CTest Name | Test Category | Source Location |
|---|---|---|---|---|---|
| 1 | `test_config` | `test_config.exe` | `CoreConfigTest` | Core Subsystem | `tests/core/test_config.cpp` |
| 2 | `test_lua` | `test_lua.exe` | `LuaScriptingTest` | Scripting Subsystem | `tests/scripting/test_lua.cpp` |
| 3 | `test_rendering` | `test_rendering.exe` | `RenderingCameraTest` | Graphics Subsystem | `tests/graphics/test_rendering.cpp` |
| 4 | `test_assets` | `test_assets.exe` | `AssetManagementTest` | Resource Management | `tests/resources/test_assets.cpp` |
| 5 | `test_world` | `test_world.exe` | `WorldTilemapTest` | World Subsystem | `tests/world/test_world.cpp` |
| 6 | `test_player` | `test_player.exe` | `PlayerEntityTest` | Entity Subsystem | `tests/entities/test_player.cpp` |
| 7 | `test_combat` | `test_combat.exe` | `CombatSystemTest` | Combat Subsystem | `tests/combat/test_combat.cpp` |
| 8 | `test_modular_character` | `test_modular_character.exe` | `ModularCharacterTest` | Entity Rendering | `tests/entities/test_modular_character.cpp` |
| 9 | `test_production_cosmetics` | `test_production_cosmetics.exe` | `ProductionCosmeticsTest` | Cosmetics Subsystem | `tests/customization/test_production_cosmetics.cpp` |
| 10 | `test_creatures` | `test_creatures.exe` | `CreaturesTest` | Entity / Creature | `tests/entities/test_creatures.cpp` |
| 11 | `test_environment_assets` | `test_environment_assets.exe` | `EnvironmentAssetsTest` | World Assets | `tests/world/test_environment_assets.cpp` |
| 12 | `test_environmental_animation` | `test_environmental_animation.exe` | `EnvironmentalAnimationTest` | Animation Subsystem | `tests/graphics/test_environmental_animation.cpp` |
| 13 | `test_village_buildings` | `test_village_buildings.exe` | `VillageBuildingsTest` | World Buildings | `tests/world/test_village_buildings.cpp` |
| 14 | `test_biomes` | `test_biomes.exe` | `BiomeSystemTest` | World Biomes | `tests/world/test_biomes.cpp` |
| 15 | `test_weapons` | `test_weapons.exe` | `WeaponSystemTest` | Combat / Weapons | `tests/weapons/test_weapons.cpp` |
| 16 | `test_items` | `test_items.exe` | `ItemSystemTest` | Items / Inventory | `tests/items/test_items.cpp` |
| 17 | `test_costumes_armor` | `test_costumes_armor.exe` | `CostumesArmorTest` | Customization | `tests/customization/test_costumes_armor.cpp` |
| 18 | `test_audio` | `test_audio.exe` | `AudioSystemTest` | Audio Subsystem | `tests/audio/test_audio.cpp` |
| 19 | `test_ui` | `test_ui.exe` | `UISystemTest` | UI Management | `tests/ui/test_ui.cpp` |
| 20 | *(Meta-Test)* | *(Uses `wildbound.exe`)* | `ExecutableSmokeTest` | Client Smoke Test | `wildbound --test-run` |
| 21 | `test_gameplay_integration` | `test_gameplay_integration.exe` | `GameplayIntegrationTest` | Subsystem Integration | `tests/integration/test_gameplay_integration.cpp` |
| 22 | `test_collision` | `test_collision.exe` | `PhysicsCollisionTest` | Physics & Collision | `tests/physics/test_collision.cpp` |
| 23 | `test_input` | `test_input.exe` | `InputSystemTest` | Input Subsystem | `tests/input/test_input.cpp` |
| 24 | `test_interaction` | `test_interaction.exe` | `WorldInteractionTest` | Interaction System | `tests/interaction/test_interaction.cpp` |
| 25 | `test_inventory` | `test_inventory.exe` | `LootInventoryTest` | Loot & Container | `tests/items/test_inventory.cpp` |
| 26 | `test_weapons_equipment` | `test_weapons_equipment.exe` | `WeaponEquipmentTest` | Weapons Equipment | `tests/weapons/test_weapons_equipment.cpp` |
| 27 | `test_customization` | `test_customization.exe` | `CharacterCustomizationTest` | Customization Manager | `tests/customization/test_customization.cpp` |
| 28 | `test_cosmetics_store` | `test_cosmetics_store.exe` | `CosmeticsStoreTest` | Store & Ownership | `tests/cosmetics/test_cosmetics_store.cpp` |
| 29 | `test_game_ui` | `test_game_ui.exe` | `GameUITest` | HUD & Menus | `tests/ui/test_game_ui.cpp` |
| 30 | `test_ai` | `test_ai.exe` | `AINavigationTest` | AI & Pathfinding | `tests/ai/test_ai.cpp` |
| 31 | `test_battle_royale_world` | `test_battle_royale_world.exe` | `BattleRoyaleWorldTest` | Battle Royale World | `tests/world/test_battle_royale_world.cpp` |
| 32 | `test_corruption_storm` | `test_corruption_storm.exe` | `CorruptionStormTest` | Storm Mechanics | `tests/storm/test_corruption_storm.cpp` |
| 33 | `test_match_lifecycle` | `test_match_lifecycle.exe` | `MatchLifecycleTest` | Match State Machine | `tests/match/test_match_lifecycle.cpp` |
| 34 | `test_match_results` | `test_match_results.exe` | `MatchResultsTest` | Results & Outcomes | `tests/results/test_match_results.cpp` |
| 35 | `test_game_modes` | `test_game_modes.exe` | `GameModesTest` | Game Modes / Teams | `tests/gamemodes/test_game_modes.cpp` |
| 36 | `test_network_architecture` | `test_network_architecture.exe` | `NetworkArchitectureTest` | Networking Protocol | `tests/network/test_network_architecture.cpp` |
| 37 | `test_network_transport` | `test_network_transport.exe` | `NetworkTransportTest` | Transport / ENet | `tests/network/test_network_transport.cpp` |
| 38 | `test_entity_replication` | `test_entity_replication.exe` | `EntityReplicationTest` | Entity Sync | `tests/network/test_entity_replication.cpp` |
| 39 | `test_dedicated_server` | `test_dedicated_server.exe` | `DedicatedServerTest` | Dedicated Server | `tests/server/test_dedicated_server.cpp` |
| 40 | `test_matchmaking_sessions` | `test_matchmaking_sessions.exe` | `MatchmakingSessionsTest` | Auth & Sessions | `tests/matchmaking/test_sessions.cpp` |
| 41 | `test_matchmaking_lobbies_parties` | `test_matchmaking_lobbies_parties.exe` | `MatchmakingLobbiesPartiesTest` | Lobbies & Parties | `tests/matchmaking/test_lobbies_parties.cpp` |
| 42 | `test_matchmaking_invitations` | `test_matchmaking_invitations.exe` | `MatchmakingInvitationsTest` | Invitations | `tests/matchmaking/test_invitations.cpp` |
| 43 | `test_matchmaking_queues` | `test_matchmaking_queues.exe` | `MatchmakingQueuesTest` | Matchmaking Queues | `tests/matchmaking/test_queues.cpp` |
| 44 | `test_matchmaking_server_registry` | `test_matchmaking_server_registry.exe` | `MatchmakingServerRegistryTest` | Server Allocation | `tests/matchmaking/test_server_registry.cpp` |
| 45 | `test_matchmaking_assignment` | `test_matchmaking_assignment.exe` | `MatchmakingAssignmentTest` | Ticket Assignment | `tests/matchmaking/test_assignment.cpp` |
| 46 | `test_matchmaking_coordinator_e2e` | `test_matchmaking_coordinator_e2e.exe` | `MatchmakingCoordinatorE2ETest` | Coordinator E2E | `tests/matchmaking/test_coordinator_e2e.cpp` |
| 47 | `test_security_validation` | `test_security_validation.exe` | `SecurityValidationTest` | Security & Anti-Cheat | `tests/security/test_security_validation.cpp` |
| 48 | `test_multiplayer_load` | `test_multiplayer_load.exe` | `MultiplayerLoadTest` | Load & Scalability | `tests/load/test_multiplayer_load.cpp` |

---

## 3. Inventory Validation Metrics

- **Target Harmonization**: **100.0%** (47 of 47 test binaries present in both Debug and Release).
- **Production Target Harmonization**: **100.0%** (3 of 3 production binaries present in both Debug and Release).
- **Asymmetry Count**: **0** (Zero discrepancy between configurations).

