#include "../../src/world/BiomeData.hpp"
#include "../../src/world/BiomeManager.hpp"
#include "../../src/world/BuildingData.hpp"
#include "../../src/world/BuildingManager.hpp"
#include "../../src/world/World.hpp"
#include "../../src/world/MapLoader.hpp"
#include "../../src/world/environment/EnvironmentalAnimationSystem.hpp"
#include "../../src/entities/Player.hpp"
#include "../../src/graphics/Camera2D.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using wildbound::core::Logger;
using namespace wildbound::world;
using namespace wildbound::world::environment;
using namespace wildbound::resources;
using namespace wildbound::entities;
using namespace wildbound::graphics;
namespace fs = std::filesystem;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

static bool ApproxEqual(float a, float b, float epsilon = 0.05f) {
    return std::fabs(a - b) <= epsilon;
}

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound Biome Expansion System Tests (Phase 14) ===" << std::endl;

    // Headless raylib context for texture preloading in tests
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundBiomeTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");

    // ========================================================================
    // Test 1: Biome Catalog Loading & Metadata Integrity
    // ========================================================================
    {
        std::cout << "[TEST] 1. Biome catalog initialization & metadata verification..." << std::endl;
        BiomeManager mgr;
        bool ok = mgr.Initialize("data/biomes");
        TEST_ASSERT(ok, "BiomeManager::Initialize should succeed");
        TEST_ASSERT(mgr.GetBiomeCount() == 5, "Must load exactly 5 biomes (forest, desert, snow, swamp, mountain)");

        const std::vector<std::string> expectedBiomes = { "forest", "desert", "snow", "swamp", "mountain" };
        for (const auto& id : expectedBiomes) {
            TEST_ASSERT(mgr.HasBiome(id), ("BiomeManager must contain biome: " + id).c_str());
            const BiomeConfig* b = mgr.GetBiome(id);
            TEST_ASSERT(b != nullptr, "BiomeConfig pointer must not be null");
            TEST_ASSERT(!b->displayName.empty(), "Biome display name must not be empty");
            TEST_ASSERT(!b->description.empty(), "Biome description must not be empty");
            TEST_ASSERT(!b->tileset.empty(), "Tileset ID must not be empty");
            TEST_ASSERT(!b->tilesetImage.empty(), "Tileset image path must not be empty");
            TEST_ASSERT(!b->maps.empty(), "Biome must contain at least 1 map path");
            TEST_ASSERT(b->terrain.size() >= 5, "Biome must specify at least 5 terrain tile types");
            TEST_ASSERT(!b->vegetation.empty(), "Biome must specify vegetation assets");
            TEST_ASSERT(!b->environmentObjects.empty(), "Biome must specify environment objects");
            TEST_ASSERT(!b->animations.empty(), "Biome must specify animations");
            TEST_ASSERT(!b->ambientEntities.empty(), "Biome must specify ambient entities");
        }

        std::cout << "  -> All 5 biomes verified with comprehensive metadata. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 2: AMS Tileset Resolution for all 5 Biomes
    // ========================================================================
    {
        std::cout << "[TEST] 2. AMS tileset asset registration & disk verification..." << std::endl;
        BiomeManager mgr;
        mgr.Initialize("data/biomes");

        const auto& allBiomes = mgr.GetAllBiomes();
        for (const auto& b : allBiomes) {
            const auto* meta = assetMgr.GetRegistry().GetMetadata(b.tileset);
            TEST_ASSERT(meta != nullptr, ("Tileset asset '" + b.tileset + "' must exist in AssetRegistry").c_str());
            TEST_ASSERT(meta->category.find("tileset") != std::string::npos, "Tileset asset category must contain 'tileset'");
            TEST_ASSERT(meta->type == AssetType::Texture, "Tileset asset type must be Texture");
            TEST_ASSERT(fs::exists(fs::path("assets") / meta->file), ("Tileset image file must exist on disk: " + meta->file).c_str());
        }

        std::cout << "  -> All 5 biome tilesets confirmed registered and present on disk. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 3: Prototype & Biome Map Loading Integrity
    // ========================================================================
    {
        std::cout << "[TEST] 3. Biome prototype map loading via MapLoader & World..." << std::endl;
        BiomeManager mgr;
        mgr.Initialize("data/biomes");

        const auto& allBiomes = mgr.GetAllBiomes();
        for (const auto& b : allBiomes) {
            World world;
            const std::string& primaryMap = b.maps[0];
            bool loaded = world.LoadMap(primaryMap, assetMgr);
            TEST_ASSERT(loaded, ("Failed to load map: " + primaryMap).c_str());
            TEST_ASSERT(world.IsLoaded(), "World::IsLoaded() must be true");

            const Tilemap* tilemap = world.GetTilemap();
            TEST_ASSERT(tilemap != nullptr, "Tilemap must not be null");
            TEST_ASSERT(tilemap->GetLayerCount() >= 4, "Biome map must contain at least 4 layers");

            // For the 4 new prototype maps: 60x45 tiles, 1920x1440 world pixels
            if (b.id != "forest") {
                TEST_ASSERT(tilemap->GetWidth() == 60, "Prototype map width must be 60 tiles");
                TEST_ASSERT(tilemap->GetHeight() == 45, "Prototype map height must be 45 tiles");
                Rectangle bounds = world.GetWorldBounds();
                TEST_ASSERT(bounds.width == 1920.0f, "Prototype map width must be 1920 px");
                TEST_ASSERT(bounds.height == 1440.0f, "Prototype map height must be 1440 px");
            } else {
                // Forest expanded map: 160x120 tiles, 5120x3840 px
                TEST_ASSERT(tilemap->GetWidth() == 160, "Forest map width must be 160 tiles");
                TEST_ASSERT(tilemap->GetHeight() == 120, "Forest map height must be 120 tiles");
            }

            // Object layer verification
            TEST_ASSERT(!tilemap->GetObjects().empty(), "Biome map must contain environment objects");
        }

        std::cout << "  -> All 5 biome maps successfully loaded and verified. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 4: Biome Collision Cache Generation & Traversal Checks
    // ========================================================================
    {
        std::cout << "[TEST] 4. Biome collision cache generation & traversal tests..." << std::endl;
        BiomeManager mgr;
        mgr.Initialize("data/biomes");

        const auto& allBiomes = mgr.GetAllBiomes();
        for (const auto& b : allBiomes) {
            World world;
            world.LoadMap(b.maps[0], assetMgr);

            // Fetch collision boxes in the map area
            auto collisionBoxes = world.GetCollisionBoxes(world.GetWorldBounds());
            TEST_ASSERT(!collisionBoxes.empty(), ("Biome " + b.id + " must produce collision boxes").c_str());

            // Default spawn must be collision-free
            Rectangle spawnBox{ b.defaultSpawn.x - 12.0f, b.defaultSpawn.y - 12.0f, 24.0f, 24.0f };
            TEST_ASSERT(!world.CheckCollision(spawnBox), ("Biome " + b.id + " default spawn must be collision-free").c_str());
        }

        std::cout << "  -> Collision caches and spawn traversability verified across all biomes. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 5: Biome Switching Lifecycle & Atmospheric Reconfiguration
    // ========================================================================
    {
        std::cout << "[TEST] 5. Biome switching lifecycle & environmental reconfiguration..." << std::endl;
        BiomeManager mgr;
        mgr.Initialize("data/biomes");

        World world;
        Player player;
        Camera2DSystem camera;
        camera.SetVirtualDimensions(640, 360);

        EnvironmentalAnimationSystem envAnim;
        envAnim.Initialize(assetMgr);

        // Transition through each biome in sequence
        const std::vector<std::string> switchSequence = {
            "forest", "desert", "snow", "swamp", "mountain", "forest"
        };

        for (const auto& targetBiome : switchSequence) {
            bool ok = mgr.SwitchBiome(targetBiome, world, player, camera, assetMgr, &envAnim, 0);
            TEST_ASSERT(ok, ("SwitchBiome to " + targetBiome + " must succeed").c_str());
            TEST_ASSERT(mgr.GetActiveBiomeId() == targetBiome, "Active biome ID must match target");

            const BiomeConfig* active = mgr.GetActiveBiome();
            TEST_ASSERT(active != nullptr, "Active biome pointer must not be null");

            // Verify player was spawned at the designated position
            Vector2 pPos = player.GetPosition();
            TEST_ASSERT(pPos.x > 0.0f && pPos.y > 0.0f, "Player position must be valid and positive");

            // Verify camera bounds aligned to the new map
            Rectangle camBounds = camera.GetWorldBounds();
            Rectangle worldBounds = world.GetWorldBounds();
            TEST_ASSERT(ApproxEqual(camBounds.width, worldBounds.width), "Camera width must match world bounds");
            TEST_ASSERT(ApproxEqual(camBounds.height, worldBounds.height), "Camera height must match world bounds");

            // Verify atmospheric synchronization
            TEST_ASSERT(ApproxEqual(envAnim.GetFogOpacity(), active->atmosphere.fogDensity),
                        "EnvironmentalAnimationSystem fog opacity must match biome config");
            TEST_ASSERT(envAnim.IsCloudsEnabled() == active->atmosphere.cloudLayerEnabled,
                        "Clouds enabled state must match biome config");
        }

        std::cout << "  -> Full biome switching lifecycle, camera framing, and atmosphere sync verified. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 6: Forest Biome & Village Building Compatibility
    // ========================================================================
    {
        std::cout << "[TEST] 6. Verifying Phase 13 Greenwood Village compatibility..." << std::endl;
        BuildingManager bldgMgr;
        bool bOk = bldgMgr.Initialize("data/buildings/village_buildings.json", "maps/forest/forest_01.json");
        TEST_ASSERT(bOk, "BuildingManager must initialize cleanly");
        TEST_ASSERT(bldgMgr.GetTotalBuildingCount() == 13, "Village must retain 13 buildings");
        TEST_ASSERT(bldgMgr.GetEnterableBuildingCount() == 6, "Village must retain 6 enterable interiors");

        World world;
        Player player;
        Camera2DSystem camera;
        bool loadedForest = world.LoadMap("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(loadedForest, "Forest exterior map must load");

        // Verify entering a village interior still works 100%
        bool entered = bldgMgr.EnterBuilding("village_blacksmith", world, player, camera, assetMgr);
        TEST_ASSERT(entered, "Entering Greenwood Blacksmith interior must succeed");
        TEST_ASSERT(bldgMgr.IsIndoors(), "Should be indoors");
        TEST_ASSERT(world.GetWorldBounds().width == 640.0f, "Interior map width must be 640 px");

        // Exit building
        bool exited = bldgMgr.ExitBuilding(world, player, camera, assetMgr);
        TEST_ASSERT(exited, "Exiting Greenwood Blacksmith must succeed");
        TEST_ASSERT(!bldgMgr.IsIndoors(), "Should be outdoors");
        TEST_ASSERT(world.GetWorldBounds().width == 5120.0f, "Restored map width must be 5120 px");

        std::cout << "  -> Complete Phase 13 Village compatibility confirmed. [PASSED]" << std::endl;
    }

    CloseWindow();
    std::cout << "\n>>> ALL PHASE 14 BIOME EXPANSION SYSTEM TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}

