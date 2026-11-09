# ============================================================================
# WILDBOUND TEST SUITE (CTest)
# ============================================================================

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${lua_SOURCE_DIR}/lua-5.4.7/include
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${enet_SOURCE_DIR}/include
)

link_libraries(sol2 lua_static)

set(GAMEMODES_SOURCES
    ${CMAKE_SOURCE_DIR}/src/gamemodes/Team.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/TeamManager.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/TeamVictoryEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/GameModeDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/GameModeRules.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/BotMatchConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/PracticeMode.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/GameModeRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/gamemodes/GameModeManager.cpp
)

set(MATCH_RESULTS_STORM_SOURCES
    ${CMAKE_SOURCE_DIR}/src/ui/menus/ModeSelectMenu.cpp
    ${GAMEMODES_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/match/MatchState.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipant.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipantRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/match/SpawnPoint.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchSpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchCountdown.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchEliminationSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchResult.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchManager.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchOutcome.cpp
    ${CMAKE_SOURCE_DIR}/src/results/VictoryEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/DefeatEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchResultManager.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/SafeZone.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormDamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormBoundaryRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormManager.cpp
)

set(GAMEPLAY_INTEGRATION_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/Door.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/Mechanism.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionExecutor.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
)

set(BACKEND_HTTP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/network/http/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/src/network/http/BackendClient.cpp
)

set(MATCHMAKING_CONNECTOR_SOURCES
    ${CMAKE_SOURCE_DIR}/src/ui/menus/MultiplayerMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/network/matchmaking/MatchmakingConnector.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetClient.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetTransport.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/PacketValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
)

# 1. Core Configuration Test
add_executable(test_config
    core/test_config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_config PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(test_config PRIVATE cxx_std_20)

add_test(NAME CoreConfigTest COMMAND test_config)
set_tests_properties(CoreConfigTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 2. Lua Scripting & Config Parser Test
add_executable(test_lua
    scripting/test_lua.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_lua PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${raylib_SOURCE_DIR}/src
)
target_compile_features(test_lua PRIVATE cxx_std_20)
target_link_libraries(test_lua PRIVATE sol2 lua_static)

add_test(NAME LuaScriptingTest COMMAND test_lua)
set_tests_properties(LuaScriptingTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 3. Graphics, Viewport & Camera System Test
add_executable(test_rendering
    graphics/test_rendering.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_rendering PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_rendering PRIVATE cxx_std_20)
target_link_libraries(test_rendering PRIVATE raylib)

add_test(NAME RenderingCameraTest COMMAND test_rendering)
set_tests_properties(RenderingCameraTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 4. Asset Management System & Metadata Test (Phase 2)
add_executable(test_assets
    resources/test_assets.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_assets PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_assets PRIVATE cxx_std_20)
target_link_libraries(test_assets PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME AssetManagementTest COMMAND test_assets)
set_tests_properties(AssetManagementTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 5. World, Tilemap & Environment Test (Phase 3)
add_executable(test_world
    world/test_world.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_world PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_world PRIVATE cxx_std_20)
target_link_libraries(test_world PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME WorldTilemapTest COMMAND test_world)
set_tests_properties(WorldTilemapTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 6. Player, Character & Animation Test (Phase 4)
add_executable(test_player
    entities/test_player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_player PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_player PRIVATE cxx_std_20)
target_link_libraries(test_player PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME PlayerEntityTest COMMAND test_player)
set_tests_properties(PlayerEntityTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 7. Combat System, Enemy Entities & AI Test (Phase 5)
add_executable(test_combat
    combat/test_combat.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnemySpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_combat PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
)
target_compile_features(test_combat PRIVATE cxx_std_20)
target_link_libraries(test_combat PRIVATE raylib sol2 lua_static nlohmann_json::nlohmann_json)

add_test(NAME CombatSystemTest COMMAND test_combat)
set_tests_properties(CombatSystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 8. Modular Character & Cosmetic Equipment Test (Phase 7)
add_executable(test_modular_character
    entities/test_modular_character.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_modular_character PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_modular_character PRIVATE cxx_std_20)
target_link_libraries(test_modular_character PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME ModularCharacterTest COMMAND test_modular_character)
set_tests_properties(ModularCharacterTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 9. Phase 8 Production Cosmetics Collection Test (32 Designs)
add_executable(test_production_cosmetics
    entities/test_production_cosmetics.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_production_cosmetics PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_production_cosmetics PRIVATE cxx_std_20)
target_link_libraries(test_production_cosmetics PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME ProductionCosmeticsTest COMMAND test_production_cosmetics)
set_tests_properties(ProductionCosmeticsTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 10. Phase 9 Enemy & Wildlife Asset Collection Test
add_executable(test_creatures
    entities/test_creatures.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_creatures PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_creatures PRIVATE cxx_std_20)
target_link_libraries(test_creatures PRIVATE nlohmann_json::nlohmann_json)

add_test(NAME CreaturesTest COMMAND test_creatures)
set_tests_properties(CreaturesTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 11. Phase 10 Environment Asset Collection Test
add_executable(test_environment_assets
    world/test_environment_assets.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_environment_assets PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_environment_assets PRIVATE cxx_std_20)
target_link_libraries(test_environment_assets PRIVATE nlohmann_json::nlohmann_json)

add_test(NAME EnvironmentAssetsTest COMMAND test_environment_assets)
set_tests_properties(EnvironmentAssetsTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 12. Phase 11 Environmental Animation System Test
add_executable(test_environmental_animation
    world/test_environmental_animation.cpp
    ${CMAKE_SOURCE_DIR}/src/world/environment/EnvironmentalAnimationSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_environmental_animation PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_environmental_animation PRIVATE cxx_std_20)
target_link_libraries(test_environmental_animation PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME EnvironmentalAnimationTest COMMAND test_environmental_animation)
set_tests_properties(EnvironmentalAnimationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 13. Village Construction & Interior System Test (Phase 13)
add_executable(test_village_buildings
    world/test_village_buildings.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_village_buildings PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_village_buildings PRIVATE cxx_std_20)
target_link_libraries(test_village_buildings PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME VillageBuildingsTest COMMAND test_village_buildings)
set_tests_properties(VillageBuildingsTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 14. Biome Expansion & Multi-Biome Architecture Test (Phase 14)
add_executable(test_biomes
    world/test_biomes.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BiomeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/environment/EnvironmentalAnimationSystem.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_biomes PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_biomes PRIVATE cxx_std_20)
target_link_libraries(test_biomes PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME BiomeSystemTest COMMAND test_biomes)
set_tests_properties(BiomeSystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 15. Weapon Asset Collection & Metadata Test (Phase 15)
add_executable(test_weapons
    weapons/test_weapons.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_weapons PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_weapons PRIVATE cxx_std_20)
target_link_libraries(test_weapons PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME WeaponSystemTest COMMAND test_weapons)
set_tests_properties(WeaponSystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 16. Item & Collectible Asset Collection Test (Phase 16)
add_executable(test_items
    items/test_items.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_items PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_items PRIVATE cxx_std_20)
target_link_libraries(test_items PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME ItemSystemTest COMMAND test_items)
set_tests_properties(ItemSystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 17. Costumes & Armor Collection Test (Phase 17)
add_executable(test_costumes_armor
    entities/test_costumes_armor.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_costumes_armor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_costumes_armor PRIVATE cxx_std_20)
target_link_libraries(test_costumes_armor PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME CostumesArmorTest COMMAND test_costumes_armor)
set_tests_properties(CostumesArmorTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 18. Centralized Audio System Test (Phase 18)
add_executable(test_audio
    audio/test_audio.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_audio PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_audio PRIVATE cxx_std_20)
target_link_libraries(test_audio PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME AudioSystemTest COMMAND test_audio)
set_tests_properties(AudioSystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 19. Player-Facing UI Management System Test (Phase 18)
add_executable(test_ui
    ui/test_ui.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/UIManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/Minimap.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/HUDManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/SettingsMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/PauseMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/MainMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/AuthMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/inventory/InventoryUI.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterAppearance.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearancePreview.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearanceSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterCustomizationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterAppearance.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearancePreview.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearanceSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterCustomizationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsOwnershipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsSearchFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsStoreManager.cpp
    ${BACKEND_HTTP_SOURCES}
    ${MATCHMAKING_CONNECTOR_SOURCES}
    ${MATCH_RESULTS_STORM_SOURCES}
)
target_include_directories(test_ui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_ui PRIVATE cxx_std_20)
target_link_libraries(test_ui PRIVATE raylib nlohmann_json::nlohmann_json enet)
if(WIN32)
    target_link_libraries(test_ui PRIVATE winhttp)
endif()

add_test(NAME UISystemTest COMMAND test_ui)
set_tests_properties(UISystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 20. Executable Launch / Smoke Test (--test-run)
if(TARGET wildbound)
    add_test(NAME ExecutableSmokeTest COMMAND wildbound --test-run)
else()
    add_test(NAME ExecutableSmokeTest COMMAND WildboundServer --test-run)
endif()
set_tests_properties(ExecutableSmokeTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 21. Gameplay Asset Integration Test Suite (Phase 19)
add_executable(test_gameplay_integration
    integration/test_gameplay_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterAppearance.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearancePreview.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearanceSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterCustomizationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsOwnershipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsSearchFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsStoreManager.cpp
    ${BACKEND_HTTP_SOURCES}
    ${MATCH_RESULTS_STORM_SOURCES}
)
target_include_directories(test_gameplay_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_gameplay_integration PRIVATE cxx_std_20)
target_link_libraries(test_gameplay_integration PRIVATE raylib nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_gameplay_integration PRIVATE winhttp)
endif()

add_test(NAME GameplayIntegrationTest COMMAND test_gameplay_integration)
set_tests_properties(GameplayIntegrationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 22. Physics & Collision System Test Suite (Phase 20)
add_executable(test_collision
    physics/test_collision.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_collision PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_collision PRIVATE cxx_std_20)
target_link_libraries(test_collision PRIVATE raylib)

add_test(NAME PhysicsCollisionTest COMMAND test_collision)
set_tests_properties(PhysicsCollisionTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 23. Advanced Input Management Test Suite (Phase 21)
add_executable(test_input
    input/test_input.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_input PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_input PRIVATE cxx_std_20)
target_link_libraries(test_input PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME InputSystemTest COMMAND test_input)
set_tests_properties(InputSystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 24. World Interaction System Test Suite (Phase 22)
add_executable(test_interaction
    interaction/test_interaction.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/Door.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/Mechanism.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionExecutor.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_interaction PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_interaction PRIVATE cxx_std_20)
target_link_libraries(test_interaction PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME WorldInteractionTest COMMAND test_interaction)
set_tests_properties(WorldInteractionTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 25. Loot & Inventory System Test (Phase 23)
add_executable(test_inventory
    inventory/test_inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/Door.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/Mechanism.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionExecutor.cpp
    ${CMAKE_SOURCE_DIR}/src/interaction/InteractionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
)
target_include_directories(test_inventory PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_inventory PRIVATE cxx_std_20)
target_link_libraries(test_inventory PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME LootInventoryTest COMMAND test_inventory)
set_tests_properties(LootInventoryTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 26. Weapons & Equipment System Test (Phase 24)
add_executable(test_weapons_equipment
    weapons/test_weapons_equipment.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_weapons_equipment PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_weapons_equipment PRIVATE cxx_std_20)
target_link_libraries(test_weapons_equipment PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME WeaponEquipmentTest COMMAND test_weapons_equipment)
set_tests_properties(WeaponEquipmentTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 27. Character Customization System Test (Phase 25)
add_executable(test_customization
    customization/test_customization.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterAppearance.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearancePreview.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearanceSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterCustomizationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsOwnershipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsSearchFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsStoreManager.cpp
    ${BACKEND_HTTP_SOURCES}
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_customization PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_customization PRIVATE cxx_std_20)
target_link_libraries(test_customization PRIVATE raylib nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_customization PRIVATE winhttp)
endif()

add_test(NAME CharacterCustomizationTest COMMAND test_customization)
set_tests_properties(CharacterCustomizationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 28. Cosmetics Store & Wardrobe System Test (Phase 26)
add_executable(test_cosmetics_store
    cosmetics/test_cosmetics_store.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsOwnershipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsSearchFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsStoreManager.cpp
    ${BACKEND_HTTP_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterAppearance.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearancePreview.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearanceSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterCustomizationManager.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
)
target_include_directories(test_cosmetics_store PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_cosmetics_store PRIVATE cxx_std_20)
target_link_libraries(test_cosmetics_store PRIVATE raylib nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_cosmetics_store PRIVATE winhttp)
endif()

add_test(NAME CosmeticsStoreTest COMMAND test_cosmetics_store)
set_tests_properties(CosmeticsStoreTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 29. Player HUD, Menus & Game UI Test (Phase 27)
add_executable(test_game_ui
    ui/test_game_ui.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/UIManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/Minimap.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/HUDManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/SettingsMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/PauseMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/MainMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/menus/AuthMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/inventory/InventoryUI.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CosmeticRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterAppearance.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearancePreview.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/AppearanceSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/customization/CharacterCustomizationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsOwnershipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsSearchFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsStoreManager.cpp
    ${BACKEND_HTTP_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${MATCHMAKING_CONNECTOR_SOURCES}
    ${MATCH_RESULTS_STORM_SOURCES}
)
target_include_directories(test_game_ui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_game_ui PRIVATE cxx_std_20)
target_link_libraries(test_game_ui PRIVATE raylib nlohmann_json::nlohmann_json enet)
if(WIN32)
    target_link_libraries(test_game_ui PRIVATE winhttp)
endif()

add_test(NAME GameUITest COMMAND test_game_ui)
set_tests_properties(GameUITest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 30. AI & Navigation Expansion Test (Phase 29)
add_executable(test_ai
    ai/test_ai.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/core/AIConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/core/AIController.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/navigation/NavigationGrid.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/navigation/Pathfinding.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/movement/AIMovementController.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/movement/ObstacleAvoidance.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/movement/StuckDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/targeting/PerceptionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/targeting/TargetMemory.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/targeting/TargetSelector.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/behaviours/PatrolBehaviour.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/behaviours/InvestigateBehaviour.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/AIManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnemySpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/BuildingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/input/GamepadInput.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSettings.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputManager.cpp
)
target_include_directories(test_ai PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
)
target_compile_features(test_ai PRIVATE cxx_std_20)
target_link_libraries(test_ai PRIVATE raylib sol2 lua_static nlohmann_json::nlohmann_json)

add_test(NAME AINavigationTest COMMAND test_ai)
set_tests_properties(AINavigationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")



# 31. Battle Royale World Design Test (Phase 30)
add_executable(test_battle_royale_world
    world/test_battle_royale_world.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldItem.cpp
    ${CMAKE_SOURCE_DIR}/src/world/LootContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/navigation/NavigationGrid.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/navigation/Pathfinding.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
)
target_include_directories(test_battle_royale_world PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_battle_royale_world PRIVATE cxx_std_20)
target_link_libraries(test_battle_royale_world PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME BattleRoyaleWorldTest COMMAND test_battle_royale_world)
set_tests_properties(BattleRoyaleWorldTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")


# 32. Corruption Storm & Safe Zone Test (Phase 31)
add_executable(test_corruption_storm
    storm/test_corruption_storm.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/SafeZone.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormDamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormBoundaryRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormManager.cpp
    ${GAMEMODES_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipant.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipantRegistry.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/HUDManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/Minimap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
)
target_include_directories(test_corruption_storm PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_corruption_storm PRIVATE cxx_std_20)
target_link_libraries(test_corruption_storm PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME CorruptionStormTest COMMAND test_corruption_storm)
set_tests_properties(CorruptionStormTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 33. Battle Royale Match Lifecycle System Test (Phase 32)
add_executable(test_match_lifecycle
    match/test_match_lifecycle.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchState.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipant.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipantRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/match/SpawnPoint.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchSpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchCountdown.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchEliminationSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchResult.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchManager.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchOutcome.cpp
    ${CMAKE_SOURCE_DIR}/src/results/VictoryEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/DefeatEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchResultManager.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/SafeZone.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormDamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormBoundaryRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormManager.cpp
    ${GAMEMODES_SOURCES}
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/HUDManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/Minimap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
)
target_include_directories(test_match_lifecycle PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_match_lifecycle PRIVATE cxx_std_20)
target_link_libraries(test_match_lifecycle PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME MatchLifecycleTest COMMAND test_match_lifecycle)
set_tests_properties(MatchLifecycleTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 34. Victory Conditions, Player Statistics & Results System Test (Phase 33)
add_executable(test_match_results
    results/test_match_results.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchOutcome.cpp
    ${CMAKE_SOURCE_DIR}/src/results/VictoryEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/DefeatEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchResultManager.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchState.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipant.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipantRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/match/SpawnPoint.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchSpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchCountdown.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchEliminationSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchResult.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchManager.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchOutcome.cpp
    ${CMAKE_SOURCE_DIR}/src/results/VictoryEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/DefeatEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchResultManager.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/SafeZone.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormDamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormBoundaryRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormManager.cpp
    ${GAMEMODES_SOURCES}
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/HUDManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/Minimap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
)
target_include_directories(test_match_results PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_match_results PRIVATE cxx_std_20)
target_link_libraries(test_match_results PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME MatchResultsTest COMMAND test_match_results)
set_tests_properties(MatchResultsTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 35. Game Modes Architecture & Implementation Test (Phase 34)
add_executable(test_game_modes
    modes/test_game_modes.cpp
    ${GAMEMODES_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/results/MatchOutcome.cpp
    ${CMAKE_SOURCE_DIR}/src/results/VictoryEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/DefeatEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/results/ResultSerializer.cpp
    ${CMAKE_SOURCE_DIR}/src/results/MatchResultManager.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchState.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipant.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchParticipantRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/match/SpawnPoint.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchSpawner.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchCountdown.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchEliminationSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchResult.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/match/MatchManager.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/SafeZone.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormDamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormBoundaryRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/storm/StormManager.cpp
    ${GAMEPLAY_INTEGRATION_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/HUDManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/hud/Minimap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_SOURCE_DIR}/src/world/WorldManager.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tilemap.cpp
    ${CMAKE_SOURCE_DIR}/src/world/Tileset.cpp
    ${CMAKE_SOURCE_DIR}/src/world/TileLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/world/CollisionMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/world/EnvironmentObject.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/world/MapValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
)
target_include_directories(test_game_modes PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_game_modes PRIVATE cxx_std_20)
target_link_libraries(test_game_modes PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME GameModesTest COMMAND test_game_modes)
set_tests_properties(GameModesTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 36. Multiplayer Architecture & Protocol Test (Phase 35)
add_executable(test_network_architecture
    network/test_network_architecture.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkMessage.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkEvent.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ReplicationTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConfiguration.cpp
)
target_include_directories(test_network_architecture PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_network_architecture PRIVATE cxx_std_20)
target_link_libraries(test_network_architecture PRIVATE nlohmann_json::nlohmann_json)

add_test(NAME NetworkArchitectureTest COMMAND test_network_architecture)
set_tests_properties(NetworkArchitectureTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 37. ENet Networking & Transport Layer Test (Phase 36)
add_executable(test_network_transport
    network/test_network_transport.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkMessage.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkEvent.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ReplicationTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConnection.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ConnectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/network/PacketValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetTransport.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetServer.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetClient.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkManager.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/Snapshot.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/SnapshotBuffer.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationInterpolator.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationEvents.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationManager.cpp
)
target_include_directories(test_network_transport PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_network_transport PRIVATE cxx_std_20)
target_link_libraries(test_network_transport PRIVATE enet nlohmann_json::nlohmann_json)

add_test(NAME NetworkTransportTest COMMAND test_network_transport)
set_tests_properties(NetworkTransportTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 38. Entity Synchronization & Replication Test (Phase 37)
add_executable(test_entity_replication
    network/test_entity_replication.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkMessage.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkEvent.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ReplicationTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConnection.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ConnectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/network/PacketValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetTransport.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetServer.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetClient.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkManager.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/Snapshot.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/SnapshotBuffer.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationInterpolator.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationEvents.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/network/replication/ReplicationManager.cpp
)
target_include_directories(test_entity_replication PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_entity_replication PRIVATE cxx_std_20)
target_link_libraries(test_entity_replication PRIVATE enet nlohmann_json::nlohmann_json)

add_test(NAME EntityReplicationTest COMMAND test_entity_replication)
set_tests_properties(EntityReplicationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 39. Dedicated Server Foundation Test (Phase 39)
set(WILDBOUND_SOURCES_ABSOLUTE "")
foreach(src ${WILDBOUND_SOURCES})
    list(APPEND WILDBOUND_SOURCES_ABSOLUTE "${CMAKE_SOURCE_DIR}/${src}")
endforeach()

add_executable(test_dedicated_server
    server/test_dedicated_server.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerCommandLine.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerDiagnostics.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerLoop.cpp
    ${CMAKE_SOURCE_DIR}/src/server/DedicatedServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/MatchResultsOutbox.cpp
    ${WILDBOUND_SOURCES_ABSOLUTE}
)
target_include_directories(test_dedicated_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${enet_SOURCE_DIR}/include
)

link_libraries(sol2 lua_static)
target_compile_features(test_dedicated_server PRIVATE cxx_std_20)
target_link_libraries(test_dedicated_server PRIVATE
    raylib
    sol2
    lua_static
    imgui
    rlimgui
    nlohmann_json::nlohmann_json
    enet
)
if(WIN32)
    target_compile_definitions(test_dedicated_server PRIVATE NOGDI WIN32_LEAN_AND_MEAN NOUSER)
    target_link_libraries(test_dedicated_server PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_dedicated_server PRIVATE pthread m)
endif()

add_test(NAME DedicatedServerTest COMMAND test_dedicated_server)
set_tests_properties(DedicatedServerTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# ============================================================================
# PHASE 41: LOBBY & MATCHMAKING TEST SUITES
# ============================================================================

set(COORDINATOR_COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkStatistics.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConnection.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ConnectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/network/PacketValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetTransport.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetServer.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ENetClient.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ReplicationTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/SessionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/PartyManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/LobbyManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/PlayerInvitation.hpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/InvitationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/MatchmakingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/DedicatedServerRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/MatchAssignmentManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorDiagnostics.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorServer.cpp
)

# 40. Sessions Test
add_executable(test_matchmaking_sessions
    matchmaking/test_sessions.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/SessionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
)
target_include_directories(test_matchmaking_sessions PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_sessions PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_sessions PRIVATE enet nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_matchmaking_sessions PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_sessions PRIVATE pthread m)
endif()
add_test(NAME MatchmakingSessionsTest COMMAND test_matchmaking_sessions)
set_tests_properties(MatchmakingSessionsTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 41. Parties & Lobbies Test
add_executable(test_matchmaking_lobbies_parties
    matchmaking/test_lobbies_parties.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/PartyManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/LobbyManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
)
target_include_directories(test_matchmaking_lobbies_parties PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_lobbies_parties PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_lobbies_parties PRIVATE enet nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_matchmaking_lobbies_parties PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_lobbies_parties PRIVATE pthread m)
endif()
add_test(NAME MatchmakingLobbiesPartiesTest COMMAND test_matchmaking_lobbies_parties)
set_tests_properties(MatchmakingLobbiesPartiesTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 42. Invitations Test
add_executable(test_matchmaking_invitations
    matchmaking/test_invitations.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/InvitationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
)
target_include_directories(test_matchmaking_invitations PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_invitations PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_invitations PRIVATE enet nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_matchmaking_invitations PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_invitations PRIVATE pthread m)
endif()
add_test(NAME MatchmakingInvitationsTest COMMAND test_matchmaking_invitations)
set_tests_properties(MatchmakingInvitationsTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 43. Queues Test
add_executable(test_matchmaking_queues
    matchmaking/test_matchmaking_queues.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/MatchmakingManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
)
target_include_directories(test_matchmaking_queues PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_queues PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_queues PRIVATE enet nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_matchmaking_queues PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_queues PRIVATE pthread m)
endif()
add_test(NAME MatchmakingQueuesTest COMMAND test_matchmaking_queues)
set_tests_properties(MatchmakingQueuesTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 44. Server Registry Test
add_executable(test_matchmaking_server_registry
    matchmaking/test_server_registry.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/DedicatedServerRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
)
target_include_directories(test_matchmaking_server_registry PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_server_registry PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_server_registry PRIVATE enet nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_matchmaking_server_registry PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_server_registry PRIVATE pthread m)
endif()
add_test(NAME MatchmakingServerRegistryTest COMMAND test_matchmaking_server_registry)
set_tests_properties(MatchmakingServerRegistryTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 45. Match Assignment Test
add_executable(test_matchmaking_assignment
    matchmaking/test_match_assignment.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/MatchAssignmentManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/DedicatedServerRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/SessionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/coordinator/CoordinatorProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
)
target_include_directories(test_matchmaking_assignment PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_assignment PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_assignment PRIVATE enet nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(test_matchmaking_assignment PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_assignment PRIVATE pthread m)
endif()
add_test(NAME MatchmakingAssignmentTest COMMAND test_matchmaking_assignment)
set_tests_properties(MatchmakingAssignmentTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 46. Coordinator E2E Integration Test
add_executable(test_matchmaking_coordinator_e2e
    matchmaking/test_coordinator_e2e.cpp
    ${COORDINATOR_COMMON_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/server/ServerConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerCommandLine.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerDiagnostics.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerLoop.cpp
    ${CMAKE_SOURCE_DIR}/src/server/DedicatedServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/MatchResultsOutbox.cpp
    ${WILDBOUND_SOURCES_ABSOLUTE}
)
target_include_directories(test_matchmaking_coordinator_e2e PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_matchmaking_coordinator_e2e PRIVATE cxx_std_20)
target_link_libraries(test_matchmaking_coordinator_e2e PRIVATE
    raylib
    sol2
    lua_static
    imgui
    rlimgui
    nlohmann_json::nlohmann_json
    enet
)
if(WIN32)
    target_compile_definitions(test_matchmaking_coordinator_e2e PRIVATE NOGDI WIN32_LEAN_AND_MEAN NOUSER)
    target_link_libraries(test_matchmaking_coordinator_e2e PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_matchmaking_coordinator_e2e PRIVATE pthread m)
endif()
add_test(NAME MatchmakingCoordinatorE2ETest COMMAND test_matchmaking_coordinator_e2e)
set_tests_properties(MatchmakingCoordinatorE2ETest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 47. Security Validation Test (Phase 47)
add_executable(test_security_validation
    security/test_security_validation.cpp
    ${CMAKE_SOURCE_DIR}/src/security/RateLimiter.cpp
    ${CMAKE_SOURCE_DIR}/src/security/ServerSecurityManager.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkProtocol.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkMessage.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkEvent.cpp
    ${CMAKE_SOURCE_DIR}/src/network/ReplicationTypes.cpp
    ${CMAKE_SOURCE_DIR}/src/network/NetworkConfiguration.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_security_validation PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${raylib_SOURCE_DIR}/src
)
target_compile_features(test_security_validation PRIVATE cxx_std_20)
target_link_libraries(test_security_validation PRIVATE
    raylib
    nlohmann_json::nlohmann_json
)
if(WIN32)
    target_compile_definitions(test_security_validation PRIVATE NOGDI WIN32_LEAN_AND_MEAN NOUSER)
    target_link_libraries(test_security_validation PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_security_validation PRIVATE pthread m)
endif()
add_test(NAME SecurityValidationTest COMMAND test_security_validation)
set_tests_properties(SecurityValidationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 48. Multiplayer Load, Stress & Stability Test (Phase 48)
add_executable(test_multiplayer_load
    load/test_multiplayer_load.cpp
    load/LoadTestClient.cpp
    load/LoadTestController.cpp
    ${COORDINATOR_COMMON_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/server/ServerConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerCommandLine.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerDiagnostics.cpp
    ${CMAKE_SOURCE_DIR}/src/server/ServerLoop.cpp
    ${CMAKE_SOURCE_DIR}/src/server/DedicatedServer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/MatchResultsOutbox.cpp
    ${WILDBOUND_SOURCES_ABSOLUTE}
)
target_include_directories(test_multiplayer_load PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${enet_SOURCE_DIR}/include
)
target_compile_features(test_multiplayer_load PRIVATE cxx_std_20)
target_link_libraries(test_multiplayer_load PRIVATE
    raylib
    sol2
    lua_static
    imgui
    rlimgui
    nlohmann_json::nlohmann_json
    enet
)
if(WIN32)
    target_compile_definitions(test_multiplayer_load PRIVATE NOGDI WIN32_LEAN_AND_MEAN NOUSER)
    target_link_libraries(test_multiplayer_load PRIVATE ws2_32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_multiplayer_load PRIVATE pthread m)
endif()
add_test(NAME MultiplayerLoadTest COMMAND test_multiplayer_load)
set_tests_properties(MultiplayerLoadTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
