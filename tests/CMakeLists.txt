# ============================================================================
# WILDBOUND TEST SUITE (CTest)
# ============================================================================

set(GAMEPLAY_INTEGRATION_SOURCES
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
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationStateMachine.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
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
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Enemy.cpp
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
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
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
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
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
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
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
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
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
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
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
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/SpriteSheet.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/animation/AnimationController.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Viewport.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/RenderTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/graphics/Camera2D.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/TextureManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AnimationMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/FontManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AudioManager.cpp
    ${CMAKE_SOURCE_DIR}/src/resources/AssetManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/physics/CollisionDebugRenderer.cpp
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
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
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
)
target_include_directories(test_ui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_ui PRIVATE cxx_std_20)
target_link_libraries(test_ui PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME UISystemTest COMMAND test_ui)
set_tests_properties(UISystemTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 20. Executable Launch / Smoke Test (--test-run)
add_test(NAME ExecutableSmokeTest COMMAND wildbound --test-run)
set_tests_properties(ExecutableSmokeTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 21. Gameplay Asset Integration Test Suite (Phase 19)
add_executable(test_gameplay_integration
    integration/test_gameplay_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
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
)
target_include_directories(test_gameplay_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_gameplay_integration PRIVATE cxx_std_20)
target_link_libraries(test_gameplay_integration PRIVATE raylib nlohmann_json::nlohmann_json)

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
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponInstance.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponController.cpp
    ${CMAKE_SOURCE_DIR}/src/weapons/WeaponManager.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
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
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
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

add_test(NAME CharacterCustomizationTest COMMAND test_customization)
set_tests_properties(CharacterCustomizationTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 28. Cosmetics Store & Wardrobe System Test (Phase 26)
add_executable(test_cosmetics_store
    cosmetics/test_cosmetics_store.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsOwnershipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsSearchFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/cosmetics/CosmeticsStoreManager.cpp
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
    ${CMAKE_SOURCE_DIR}/src/audio/AudioSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ItemManager.cpp
    ${CMAKE_SOURCE_DIR}/src/items/ConsumableSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/items/Inventory.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Entity.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HealthComponent.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/DamageSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/AttackSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/HitDetection.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/ProjectileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/combat/CombatSystem.cpp
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
)
target_include_directories(test_game_ui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_game_ui PRIVATE cxx_std_20)
target_link_libraries(test_game_ui PRIVATE raylib nlohmann_json::nlohmann_json)

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
    ${CMAKE_SOURCE_DIR}/src/entities/EnemyConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Character.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/CharacterController.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/ModularCharacter.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/EquipmentSystem.cpp
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
