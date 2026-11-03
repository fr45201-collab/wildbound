# ============================================================================
# WILDBOUND TEST SUITE (CTest)
# ============================================================================

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
)
target_include_directories(test_player PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(test_player PRIVATE cxx_std_20)
target_link_libraries(test_player PRIVATE raylib nlohmann_json::nlohmann_json)

add_test(NAME PlayerEntityTest COMMAND test_player)
set_tests_properties(PlayerEntityTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 7. Executable Launch / Smoke Test (--test-run)
add_test(NAME ExecutableSmokeTest COMMAND wildbound --test-run)
set_tests_properties(ExecutableSmokeTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
