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

# Configure compiler options per platform
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
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -pthread
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.0")
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-template-body>)
    endif()
    add_compile_definitions(_GNU_SOURCE)
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

# 7. ENet (Reliable UDP Networking) — Tag: v1.3.18
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.18
)

# Populate dependencies
if(WIN32)
    FetchContent_MakeAvailable(raylib lua imgui rlimgui nlohmann_json enet)
else()
    # On Linux headless: avoid raylib desktop X11/Wayland dependencies
    FetchContent_MakeAvailable(lua imgui rlimgui nlohmann_json enet)
    FetchContent_GetProperties(raylib)
    if(NOT raylib_POPULATED)
        FetchContent_Populate(raylib)
    endif()
    add_library(raylib STATIC
        src/platform/headless/raylib_stub.c
    )
    target_include_directories(raylib PUBLIC ${raylib_SOURCE_DIR}/src)
    target_link_libraries(raylib PUBLIC m pthread)
endif()

if(TARGET enet)
    target_include_directories(enet PUBLIC ${enet_SOURCE_DIR}/include)
    if(WIN32)
        target_link_libraries(enet PUBLIC ws2_32 winmm)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(enet PUBLIC pthread m)
    endif()
endif()

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
if(WIN32)
    add_library(rlimgui STATIC
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    )
else()
    add_library(rlimgui STATIC
        src/platform/headless/rlimgui_stub.cpp
    )
endif()
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC raylib imgui)

# ============================================================================
# PLATFORM ABSTRACTION LAYER (Phase 40A)
# ============================================================================
set(PLATFORM_SOURCES
    src/platform/Platform.hpp
    src/platform/Platform.cpp
)
if(WIN32)
    list(APPEND PLATFORM_SOURCES
        src/platform/windows/WindowsPlatform.hpp
        src/platform/windows/WindowsPlatform.cpp
        src/platform/windows/WindowsConsole.hpp
        src/platform/windows/WindowsConsole.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND PLATFORM_SOURCES
        src/platform/linux/LinuxPlatform.hpp
        src/platform/linux/LinuxPlatform.cpp
        src/platform/linux/LinuxSignals.hpp
        src/platform/linux/LinuxSignals.cpp
    )
endif()

# ============================================================================
# WILDBOUND ENGINE CORE SOURCES & EXECUTABLE
# ============================================================================

set(WILDBOUND_SOURCES
    ${PLATFORM_SOURCES}

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

    # Match Lifecycle System (Phase 32)
    src/match/MatchState.hpp
    src/match/MatchState.cpp
    src/match/MatchParticipant.hpp
    src/match/MatchParticipant.cpp
    src/match/MatchParticipantRegistry.hpp
    src/match/MatchParticipantRegistry.cpp
    src/match/SpawnPoint.hpp
    src/match/SpawnPoint.cpp
    src/match/MatchSpawner.hpp
    src/match/MatchSpawner.cpp
    src/match/MatchCountdown.hpp
    src/match/MatchCountdown.cpp
    src/match/MatchEliminationSystem.hpp
    src/match/MatchEliminationSystem.cpp
    src/match/MatchStatistics.hpp
    src/match/MatchStatistics.cpp
    src/match/MatchResult.hpp
    src/match/MatchResult.cpp
    src/match/MatchConfiguration.hpp
    src/match/MatchConfiguration.cpp
    src/match/MatchManager.hpp
    src/match/MatchManager.cpp

    # Victory Conditions, Player Statistics & Results System (Phase 33)
    src/results/MatchOutcome.hpp
    src/results/MatchOutcome.cpp
    src/results/EliminationRecord.hpp
    src/results/PlayerPerformance.hpp
    src/results/MatchResultModel.hpp
    src/results/VictoryEvaluator.hpp
    src/results/VictoryEvaluator.cpp
    src/results/DefeatEvaluator.hpp
    src/results/DefeatEvaluator.cpp
    src/results/ResultCalculator.hpp
    src/results/ResultCalculator.cpp
    src/results/ResultSerializer.hpp
    src/results/ResultSerializer.cpp
    src/results/MatchResultManager.hpp
    src/results/MatchResultManager.cpp

    # Game Modes & Teams (Phase 34)
    src/gamemodes/Team.hpp
    src/gamemodes/Team.cpp
    src/gamemodes/TeamManager.hpp
    src/gamemodes/TeamManager.cpp
    src/gamemodes/TeamVictoryEvaluator.hpp
    src/gamemodes/TeamVictoryEvaluator.cpp
    src/gamemodes/GameModeDefinition.hpp
    src/gamemodes/GameModeDefinition.cpp
    src/gamemodes/GameModeRules.hpp
    src/gamemodes/GameModeRules.cpp
    src/gamemodes/BotMatchConfiguration.hpp
    src/gamemodes/BotMatchConfiguration.cpp
    src/gamemodes/PracticeMode.hpp
    src/gamemodes/PracticeMode.cpp
    src/gamemodes/GameModeRegistry.hpp
    src/gamemodes/GameModeRegistry.cpp
    src/gamemodes/GameModeManager.hpp
    src/gamemodes/GameModeManager.cpp

    # Menus
    src/ui/menus/ModeSelectMenu.hpp
    src/ui/menus/ModeSelectMenu.cpp
    src/ui/menus/MultiplayerMenu.hpp
    src/ui/menus/MultiplayerMenu.cpp
    src/ui/menus/AuthMenu.hpp
    src/ui/menus/AuthMenu.cpp

    # Coordinator Protocol
    src/coordinator/CoordinatorProtocol.hpp
    src/coordinator/CoordinatorProtocol.cpp

    # Network Architecture & Transport (Phase 35 & 36)
    src/network/NetworkTypes.hpp
    src/network/NetworkRole.hpp
    src/network/INetworkTransport.hpp
    src/network/NetworkProtocol.hpp
    src/network/NetworkProtocol.cpp
    src/network/NetworkMessage.hpp
    src/network/NetworkMessage.cpp
    src/network/NetworkEvent.hpp
    src/network/NetworkEvent.cpp
    src/network/ReplicationTypes.hpp
    src/network/ReplicationTypes.cpp
    src/network/NetworkConfiguration.hpp
    src/network/NetworkConfiguration.cpp
    src/network/ConnectionState.hpp
    src/network/ConnectionEvents.hpp
    src/network/NetworkStatistics.hpp
    src/network/NetworkStatistics.cpp
    src/network/NetworkConnection.hpp
    src/network/NetworkConnection.cpp
    src/network/ConnectionManager.hpp
    src/network/ConnectionManager.cpp
    src/network/PacketValidator.hpp
    src/network/PacketValidator.cpp
    src/network/ENetTransport.hpp
    src/network/ENetTransport.cpp
    src/network/ENetServer.hpp
    src/network/ENetServer.cpp
    src/network/ENetClient.hpp
    src/network/ENetClient.cpp
    src/network/NetworkManager.hpp
    src/network/NetworkManager.cpp

    # Entity Synchronization & Replication (Phase 37)
    src/network/replication/NetworkEntityTypes.hpp
    src/network/replication/ReplicationRegistry.hpp
    src/network/replication/ReplicationRegistry.cpp
    src/network/replication/Snapshot.hpp
    src/network/replication/Snapshot.cpp
    src/network/replication/SnapshotBuffer.hpp
    src/network/replication/SnapshotBuffer.cpp
    src/network/replication/ReplicationInterpolator.hpp
    src/network/replication/ReplicationInterpolator.cpp
    src/network/replication/ReplicationEvents.hpp
    src/network/replication/ReplicationEvents.cpp
    src/network/replication/ReplicationConfiguration.hpp
    src/network/replication/ReplicationConfiguration.cpp
    src/network/replication/ReplicationManager.hpp
    src/network/replication/ReplicationManager.cpp

    # HTTP Client & Backend Integration (Phase 42)
    src/network/http/HttpClient.hpp
    src/network/http/HttpClient.cpp
    src/network/http/BackendClient.hpp
    src/network/http/BackendClient.cpp

    # Client Matchmaking Connector (Phase 50E Stage 6B)
    src/network/matchmaking/MatchmakingConnector.hpp
    src/network/matchmaking/MatchmakingConnector.cpp

    # Security & Optimization (Phase 47)
    src/security/SecurityTypes.hpp
    src/security/RateLimiter.hpp
    src/security/RateLimiter.cpp
    src/security/ServerSecurityManager.hpp
    src/security/ServerSecurityManager.cpp
)

if(WIN32)
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
        ${enet_SOURCE_DIR}/include
    )

    target_link_libraries(wildbound PRIVATE
        raylib
        sol2
        lua_static
        imgui
        rlimgui
        nlohmann_json::nlohmann_json
        enet
        winhttp
    )

    target_compile_definitions(wildbound PRIVATE NOGDI WIN32_LEAN_AND_MEAN)

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
endif()

# ============================================================================
# DEDICATED SERVER EXECUTABLE (Phase 39)
# Headless standalone authoritative server
# ============================================================================
set(WILDBOUND_SERVER_SOURCES
    src/server/ServerLifecycle.hpp
    src/server/ServerConfig.hpp
    src/server/ServerConfig.cpp
    src/server/ServerCommandLine.hpp
    src/server/ServerCommandLine.cpp
    src/server/ServerDiagnostics.hpp
    src/server/ServerDiagnostics.cpp
    src/server/ServerLoop.hpp
    src/server/ServerLoop.cpp
    src/server/DedicatedServer.hpp
    src/server/DedicatedServer.cpp
    src/server/MatchResultsOutbox.hpp
    src/server/MatchResultsOutbox.cpp
)

add_executable(WildboundServer
    src/server/main_server.cpp
    ${WILDBOUND_SERVER_SOURCES}
    ${WILDBOUND_SOURCES}
)

target_include_directories(WildboundServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${lua_SOURCE_DIR}/lua-5.4.7/include
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${enet_SOURCE_DIR}/include
)

target_link_libraries(WildboundServer PRIVATE
    raylib
    sol2
    lua_static
    imgui
    rlimgui
    nlohmann_json::nlohmann_json
    enet
)

if(WIN32)
    target_compile_definitions(WildboundServer PRIVATE NOGDI WIN32_LEAN_AND_MEAN NOUSER)
    target_link_libraries(WildboundServer PRIVATE ws2_32 winmm winhttp)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(WildboundServer PRIVATE pthread m)
endif()

add_custom_command(TARGET WildboundServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:WildboundServer>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts
        $<TARGET_FILE_DIR:WildboundServer>/scripts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:WildboundServer>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/maps
        $<TARGET_FILE_DIR:WildboundServer>/maps
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:WildboundServer>/data
    COMMENT "Deploying runtime assets, scripts, and config to dedicated server output directory"
)

# ============================================================================
# COORDINATOR SERVICE EXECUTABLE (Phase 41)
# Standalone headless pre-match coordination, lobby, and matchmaking service
# ============================================================================
set(WILDBOUND_COORDINATOR_SOURCES
    src/core/Logger.hpp
    src/core/Logger.cpp
    src/network/NetworkTypes.hpp
    src/network/NetworkProtocol.hpp
    src/network/NetworkProtocol.cpp
    src/network/NetworkStatistics.hpp
    src/network/NetworkStatistics.cpp
    src/network/ConnectionState.hpp
    src/network/ConnectionEvents.hpp
    src/network/NetworkConnection.hpp
    src/network/NetworkConnection.cpp
    src/network/ConnectionManager.hpp
    src/network/ConnectionManager.cpp
    src/network/PacketValidator.hpp
    src/network/PacketValidator.cpp
    src/network/ENetTransport.hpp
    src/network/ENetTransport.cpp
    src/network/ENetServer.hpp
    src/network/ENetServer.cpp
    src/network/ReplicationTypes.hpp
    src/network/ReplicationTypes.cpp
    src/coordinator/CoordinatorProtocol.hpp
    src/coordinator/CoordinatorProtocol.cpp
    src/coordinator/PlayerSession.hpp
    src/coordinator/SessionManager.hpp
    src/coordinator/SessionManager.cpp
    src/coordinator/Party.hpp
    src/coordinator/PartyManager.hpp
    src/coordinator/PartyManager.cpp
    src/coordinator/Lobby.hpp
    src/coordinator/LobbyManager.hpp
    src/coordinator/LobbyManager.cpp
    src/coordinator/PlayerInvitation.hpp
    src/coordinator/InvitationManager.hpp
    src/coordinator/InvitationManager.cpp
    src/coordinator/MatchmakingQueue.hpp
    src/coordinator/MatchmakingManager.hpp
    src/coordinator/MatchmakingManager.cpp
    src/coordinator/DedicatedServerRegistry.hpp
    src/coordinator/DedicatedServerRegistry.cpp
    src/coordinator/MatchAssignment.hpp
    src/coordinator/MatchAssignmentManager.hpp
    src/coordinator/MatchAssignmentManager.cpp
    src/coordinator/CoordinatorConfig.hpp
    src/coordinator/CoordinatorConfig.cpp
    src/coordinator/CoordinatorDiagnostics.hpp
    src/coordinator/CoordinatorDiagnostics.cpp
    src/coordinator/CoordinatorServer.hpp
    src/coordinator/CoordinatorServer.cpp
)

add_executable(WildboundCoordinator
    src/coordinator/main_coordinator.cpp
    ${WILDBOUND_COORDINATOR_SOURCES}
)

target_include_directories(WildboundCoordinator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${enet_SOURCE_DIR}/include
)

target_link_libraries(WildboundCoordinator PRIVATE
    enet
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_compile_definitions(WildboundCoordinator PRIVATE NOGDI WIN32_LEAN_AND_MEAN)
    target_link_libraries(WildboundCoordinator PRIVATE ws2_32 winmm winhttp)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(WildboundCoordinator PRIVATE pthread m)
endif()

add_custom_command(TARGET WildboundCoordinator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:WildboundCoordinator>/assets
    COMMENT "Deploying runtime assets to coordinator output directory"
)

# ============================================================================
# AUTOMATED TESTING (CTest)
# ============================================================================
if(WILDBOUND_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
