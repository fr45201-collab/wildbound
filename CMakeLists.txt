cmake_minimum_required(VERSION 3.20)

# ============================================================================
# WILDBOUND — 2D Fantasy Survival Adventure Engine
# Phase 5: Combat System, Enemy Entities & Basic Enemy AI
# ============================================================================

project(Wildbound
    VERSION 0.6.0
    DESCRIPTION "2D top-down fantasy battle royale game engine"
    LANGUAGES CXX C
)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build automated tests
option(WILDBOUND_BUILD_TESTS "Build Wildbound automated test suite" ON)

# Configure MSVC compiler options
if(MSVC)
    add_compile_options(
        /W4          # High warning level
        /EHsc        # Enable C++ standard exception handling
        /utf-8       # Set source and execution character sets to UTF-8
        /permissive- # Enforce standards conformance
        /Zc:__cplusplus # Correct __cplusplus macro
        /Z7          # Embed debug info in .obj files to eliminate PDB server mismatch
    )
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# DEPENDENCY MANAGEMENT (CMake FetchContent)
# Pinned versions and immutable release tags
# ============================================================================
include(FetchContent)

# 1. raylib (Graphics & Audio Framework) — Tag: 5.5
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

# 2. Lua 5.4 (Scripting Language) — Tag: v5.4.7
set(LUA_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(LUA_BUILD_BINARY OFF CACHE BOOL "" FORCE)
set(LUA_BUILD_COMPILER OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua.git
    GIT_TAG        v5.4.7
)

# 3. sol2 (Lua-C++ Binding) — Tag: v3.3.1
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.1
)

# 4. Dear ImGui (Developer Interface) — Tag: v1.93.0
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.93.0
)

# 5. rlImGui (raylib Dear ImGui backend) — Tag: Raylib_5_5
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        Raylib_5_5
)

# 6. nlohmann/json (JSON Manifest & Metadata Parser) — Tag: v3.11.3
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

# Populate dependencies
FetchContent_MakeAvailable(raylib lua imgui rlimgui nlohmann_json)

# Setup sol2 interface target
FetchContent_GetProperties(sol2)
if(NOT sol2_POPULATED)
    FetchContent_Populate(sol2)
    add_library(sol2 INTERFACE)
    add_library(sol2::sol2 ALIAS sol2)
    target_include_directories(sol2 INTERFACE ${sol2_SOURCE_DIR}/include)
    target_link_libraries(sol2 INTERFACE lua_static)
endif()

# Setup Dear ImGui static library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# Setup rlImGui static library
add_library(rlimgui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC raylib imgui)

# ============================================================================
# WILDBOUND ENGINE CORE SOURCES & EXECUTABLE
# ============================================================================

set(WILDBOUND_SOURCES
    # Core
    src/core/Game.cpp
    src/core/Game.hpp
    src/core/EngineConfig.cpp
    src/core/EngineConfig.hpp
    src/core/Logger.cpp
    src/core/Logger.hpp

    # Graphics
    src/graphics/Viewport.cpp
    src/graphics/Viewport.hpp
    src/graphics/RenderTexture.cpp
    src/graphics/RenderTexture.hpp
    src/graphics/Camera2D.cpp
    src/graphics/Camera2D.hpp
    src/graphics/TestScene.cpp
    src/graphics/TestScene.hpp
    src/graphics/Renderer.cpp
    src/graphics/Renderer.hpp
    src/graphics/ColorPalette.hpp
    src/graphics/DrawContext.hpp

    # Resources & Asset Management
    src/resources/AssetMetadata.cpp
    src/resources/AssetMetadata.hpp
    src/resources/AssetRegistry.cpp
    src/resources/AssetRegistry.hpp
    src/resources/TextureManager.cpp
    src/resources/TextureManager.hpp
    src/resources/SpriteSheet.cpp
    src/resources/SpriteSheet.hpp
    src/resources/AnimationMetadata.cpp
    src/resources/AnimationMetadata.hpp
    src/resources/FontManager.cpp
    src/resources/FontManager.hpp
    src/resources/AudioManager.cpp
    src/resources/AudioManager.hpp
    src/resources/AssetValidator.cpp
    src/resources/AssetValidator.hpp
    src/resources/AssetManager.cpp
    src/resources/AssetManager.hpp

    # World & Tilemap System (Phase 3 & 11)
    src/world/Tileset.cpp
    src/world/Tileset.hpp
    src/world/TileLayer.cpp
    src/world/TileLayer.hpp
    src/world/CollisionMetadata.cpp
    src/world/CollisionMetadata.hpp
    src/world/EnvironmentObject.cpp
    src/world/EnvironmentObject.hpp
    src/world/Tilemap.cpp
    src/world/Tilemap.hpp
    src/world/MapLoader.cpp
    src/world/MapLoader.hpp
    src/world/MapValidator.cpp
    src/world/MapValidator.hpp
    src/world/World.cpp
    src/world/World.hpp
    src/world/WorldRenderer.cpp
    src/world/WorldRenderer.hpp
    src/world/EnemySpawner.cpp
    src/world/EnemySpawner.hpp
    src/world/environment/EnvironmentalAnimationSystem.cpp
    src/world/environment/EnvironmentalAnimationSystem.hpp
    src/world/BuildingData.hpp
    src/world/BuildingManager.cpp
    src/world/BuildingManager.hpp
    src/world/BiomeData.hpp
    src/world/BiomeManager.cpp
    src/world/BiomeManager.hpp
    src/world/WorldItem.hpp
    src/world/WorldItem.cpp
    src/world/LootContainer.hpp
    src/world/LootContainer.cpp
    src/world/WorldData.hpp
    src/world/WorldManager.hpp
    src/world/WorldManager.cpp
    src/weapons/WeaponData.hpp
    src/weapons/WeaponInstance.hpp
    src/weapons/WeaponInstance.cpp
    src/weapons/WeaponController.hpp
    src/weapons/WeaponController.cpp
    src/weapons/WeaponManager.cpp
    src/weapons/WeaponManager.hpp
    src/items/ItemData.hpp
    src/items/ItemInstance.hpp
    src/items/Inventory.hpp
    src/items/Inventory.cpp
    src/items/ItemManager.cpp
    src/items/ItemManager.hpp
    src/items/ConsumableSystem.hpp
    src/items/ConsumableSystem.cpp

    # Physics & Collision Subsystem (Phase 20)
    src/physics/CollisionTypes.hpp
    src/physics/CollisionFilter.hpp
    src/physics/OverlapDetector.hpp
    src/physics/Collider.hpp
    src/physics/CollisionEvent.hpp
    src/physics/TriggerVolume.hpp
    src/physics/CollisionManager.hpp
    src/physics/CollisionManager.cpp
    src/physics/CollisionDebugRenderer.hpp
    src/physics/CollisionDebugRenderer.cpp

    # Advanced Input Management Subsystem (Phase 21)
    src/input/InputAction.hpp
    src/input/InputBinding.hpp
    src/input/InputContext.hpp
    src/input/InputBuffer.hpp
    src/input/GamepadInput.hpp
    src/input/GamepadInput.cpp
    src/input/InputSettings.hpp
    src/input/InputSettings.cpp
    src/input/InputManager.hpp
    src/input/InputManager.cpp

    # World Interaction System (Phase 22)
    src/interaction/InteractionTypes.hpp
    src/interaction/InteractionDefinition.hpp
    src/interaction/InteractionContext.hpp
    src/interaction/InteractableComponent.hpp
    src/interaction/InteractionEvent.hpp
    src/interaction/InteractionPrompt.hpp
    src/interaction/Door.hpp
    src/interaction/Door.cpp
    src/interaction/Mechanism.hpp
    src/interaction/Mechanism.cpp
    src/interaction/InteractionValidator.hpp
    src/interaction/InteractionValidator.cpp
    src/interaction/InteractionExecutor.hpp
    src/interaction/InteractionExecutor.cpp
    src/interaction/InteractionManager.hpp
    src/interaction/InteractionManager.cpp

    # Animation Subsystem (Phase 4)
    src/graphics/animation/Direction.hpp
    src/graphics/animation/AnimationState.hpp
    src/graphics/animation/AnimationController.cpp
    src/graphics/animation/AnimationController.hpp
    src/graphics/animation/AnimationStateMachine.cpp
    src/graphics/animation/AnimationStateMachine.hpp

    # Entities & Character System (Phase 4 & 5)
    src/entities/Entity.cpp
    src/entities/Entity.hpp
    src/entities/CharacterStats.hpp
    src/entities/CharacterConfig.cpp
    src/entities/CharacterConfig.hpp
    src/entities/CharacterController.cpp
    src/entities/CharacterController.hpp
    src/entities/Character.cpp
    src/entities/Character.hpp
    src/entities/Player.cpp
    src/entities/Player.hpp
    src/entities/EquipmentSystem.cpp
    src/entities/EquipmentSystem.hpp
    src/entities/ModularCharacter.cpp
    src/entities/ModularCharacter.hpp
    src/entities/EnemyConfig.cpp
    src/entities/EnemyConfig.hpp
    src/entities/Enemy.cpp
    src/entities/Enemy.hpp

    # Character Customization Subsystem (Phase 25)
    src/customization/CosmeticTypes.hpp
    src/customization/CosmeticDefinition.hpp
    src/customization/CosmeticDefinition.cpp
    src/customization/CosmeticRegistry.hpp
    src/customization/CosmeticRegistry.cpp
    src/customization/CharacterAppearance.hpp
    src/customization/CharacterAppearance.cpp
    src/customization/AppearancePreview.hpp
    src/customization/AppearancePreview.cpp
    src/customization/AppearanceLoadout.hpp
    src/customization/AppearanceSerializer.hpp
    src/customization/AppearanceSerializer.cpp
    src/customization/CharacterCustomizationManager.hpp
    src/customization/CharacterCustomizationManager.cpp

    # Cosmetics Store & Wardrobe Subsystem (Phase 26)
    src/cosmetics/CosmeticsRarity.hpp
    src/cosmetics/CosmeticsColorVariant.hpp
    src/cosmetics/CosmeticsOwnership.hpp
    src/cosmetics/CosmeticsOwnershipManager.hpp
    src/cosmetics/CosmeticsOwnershipManager.cpp
    src/cosmetics/CosmeticsSearchFilter.hpp
    src/cosmetics/CosmeticsSearchFilter.cpp
    src/cosmetics/CosmeticsStoreManager.hpp
    src/cosmetics/CosmeticsStoreManager.cpp

    # Combat Subsystem (Phase 5 & 19)
    src/combat/CombatTypes.hpp
    src/combat/CombatEvents.hpp
    src/combat/CombatRules.hpp
    src/combat/HealthComponent.cpp
    src/combat/HealthComponent.hpp
    src/combat/DamageSystem.cpp
    src/combat/DamageSystem.hpp
    src/combat/AttackSystem.cpp
    src/combat/AttackSystem.hpp
    src/combat/ProjectileSystem.cpp
    src/combat/ProjectileSystem.hpp
    src/combat/HitDetection.cpp
    src/combat/HitDetection.hpp
    src/combat/Combatant.hpp
    src/combat/DeathHandler.hpp
    src/combat/CombatSystem.cpp
    src/combat/CombatSystem.hpp

    # Scripting & UI
    src/scripting/LuaManager.cpp
    src/scripting/LuaManager.hpp
    src/audio/AudioData.hpp
    src/audio/AudioSystem.hpp
    src/audio/AudioSystem.cpp
    src/ui/UITheme.hpp
    src/ui/UINotification.hpp
    src/ui/hud/Minimap.hpp
    src/ui/hud/Minimap.cpp
    src/ui/hud/HUDManager.hpp
    src/ui/hud/HUDManager.cpp
    src/ui/menus/SettingsMenu.hpp
    src/ui/menus/SettingsMenu.cpp
    src/ui/menus/PauseMenu.hpp
    src/ui/menus/PauseMenu.cpp
    src/ui/menus/MainMenu.hpp
    src/ui/menus/MainMenu.cpp
    src/ui/inventory/InventoryUI.hpp
    src/ui/inventory/InventoryUI.cpp
    src/ui/UIManager.hpp
    src/ui/UIManager.cpp
    src/ui/DevUI.cpp
    src/ui/DevUI.hpp

    # AI & Navigation System (Phase 29)
    src/ai/core/AITypes.hpp
    src/ai/core/AIConfiguration.hpp
    src/ai/core/AIConfiguration.cpp
    src/ai/core/AIController.hpp
    src/ai/core/AIController.cpp
    src/ai/navigation/NavigationQuery.hpp
    src/ai/navigation/NavigationGrid.hpp
    src/ai/navigation/NavigationGrid.cpp
    src/ai/navigation/Pathfinding.hpp
    src/ai/navigation/Pathfinding.cpp
    src/ai/movement/AIMovementController.hpp
    src/ai/movement/AIMovementController.cpp
    src/ai/movement/ObstacleAvoidance.hpp
    src/ai/movement/ObstacleAvoidance.cpp
    src/ai/movement/StuckDetector.hpp
    src/ai/movement/StuckDetector.cpp
    src/ai/targeting/PerceptionSystem.hpp
    src/ai/targeting/PerceptionSystem.cpp
    src/ai/targeting/TargetMemory.hpp
    src/ai/targeting/TargetMemory.cpp
    src/ai/targeting/TargetSelector.hpp
    src/ai/targeting/TargetSelector.cpp
    src/ai/behaviours/PatrolBehaviour.hpp
    src/ai/behaviours/PatrolBehaviour.cpp
    src/ai/behaviours/InvestigateBehaviour.hpp
    src/ai/behaviours/InvestigateBehaviour.cpp
    src/ai/AIManager.hpp
    src/ai/AIManager.cpp

    # Corruption Storm & Safe Zone System (Phase 31)
    src/storm/StormTypes.hpp
    src/storm/SafeZone.hpp
    src/storm/SafeZone.cpp
    src/storm/StormDamageSystem.hpp
    src/storm/StormDamageSystem.cpp
    src/storm/StormBoundaryRenderer.hpp
    src/storm/StormBoundaryRenderer.cpp
    src/storm/StormManager.hpp
    src/storm/StormManager.cpp
)

add_executable(wildbound
    src/main.cpp
    ${WILDBOUND_SOURCES}
)

target_include_directories(wildbound PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)

target_link_libraries(wildbound PRIVATE
    raylib
    sol2
    lua_static
    imgui
    rlimgui
    nlohmann_json::nlohmann_json
)

# Copy runtime assets, scripts, and configs to build target output directory
add_custom_command(TARGET wildbound POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:wildbound>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts
        $<TARGET_FILE_DIR:wildbound>/scripts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:wildbound>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/maps
        $<TARGET_FILE_DIR:wildbound>/maps
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:wildbound>/data
    COMMENT "Deploying runtime assets, scripts, and config to target output directory"
)

# ============================================================================
# AUTOMATED TESTING (CTest)
# ============================================================================
if(WILDBOUND_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
