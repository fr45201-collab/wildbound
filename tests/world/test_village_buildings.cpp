#include "../../src/world/BuildingData.hpp"
#include "../../src/world/BuildingManager.hpp"
#include "../../src/world/World.hpp"
#include "../../src/world/MapLoader.hpp"
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
    std::cout << "=== Running Wildbound Village Construction & Interior System Tests (Phase 13) ===" << std::endl;

    // Headless raylib context for texture preloading in tests
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundVillageTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");

    // ========================================================================
    // Test 1: Building Catalog Initialization & Metadata Integrity
    // ========================================================================
    {
        std::cout << "[TEST] 1. Village building catalog initialization..." << std::endl;
        BuildingManager mgr;
        bool ok = mgr.Initialize("data/buildings/village_buildings.json", "maps/forest/forest_01.json");
        TEST_ASSERT(ok, "BuildingManager::Initialize should succeed");

        TEST_ASSERT(mgr.GetVillageName() == "Greenwood Village", "Village name should match");
        TEST_ASSERT(mgr.GetTotalBuildingCount() == 13, "Total buildings should be 13 (target: 10-15)");
        TEST_ASSERT(mgr.GetEnterableBuildingCount() == 6, "Enterable buildings should be 6 (target: 5-7)");
        TEST_ASSERT(!mgr.IsIndoors(), "Should be initially outdoors");
        TEST_ASSERT(mgr.GetCurrentInteriorId().empty(), "Current interior id should be empty");
        std::cout << "  -> Initialized " << mgr.GetTotalBuildingCount()
                  << " buildings with " << mgr.GetEnterableBuildingCount()
                  << " enterable interiors. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 2: Building Lookup & District Categorization
    // ========================================================================
    {
        std::cout << "[TEST] 2. Building lookup and district categorization..." << std::endl;
        BuildingManager mgr;
        mgr.Initialize("data/buildings/village_buildings.json");

        const char* expectedIds[] = {
            "village_general_store",
            "village_blacksmith",
            "village_town_hall",
            "village_cottage",
            "village_twostory_residence",
            "village_warehouse",
            "village_mason_cottage",
            "village_weaver_house",
            "village_trapper_cabin",
            "village_market_pavilion",
            "village_tool_shed",
            "village_north_gate_watchtower",
            "village_south_guard_post"
        };

        for (const char* bid : expectedIds) {
            const Building* b = mgr.GetBuilding(bid);
            TEST_ASSERT(b != nullptr, std::string("Building should exist: ") + bid);
            TEST_ASSERT(!b->name.empty(), "Building name must not be empty");
            TEST_ASSERT(!b->buildingType.empty(), "Building type must not be empty");
            TEST_ASSERT(!b->district.empty(), "Building district must not be empty");
            TEST_ASSERT(b->exteriorBounds.width > 0.0f, "Width must be positive");
            TEST_ASSERT(b->exteriorBounds.height > 0.0f, "Height must be positive");
            TEST_ASSERT(b->entranceRadius > 0.0f, "Entrance radius must be positive");
        }

        // Test non-existent building lookup
        TEST_ASSERT(mgr.GetBuilding("non_existent_castle") == nullptr, "Non-existent building should return nullptr");
        std::cout << "  -> All 13 buildings correctly indexed with valid districts and footprints. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 3: Interior Map Files Validation & Tiled Layers Verification
    // ========================================================================
    {
        std::cout << "[TEST] 3. Interior map files validation & Tiled layers..." << std::endl;
        BuildingManager mgr;
        mgr.Initialize("data/buildings/village_buildings.json");

        for (const auto& b : mgr.GetBuildings()) {
            if (!b.isEnterable) continue;

            TEST_ASSERT(!b.interiorMapPath.empty(), "Enterable building must have interior map path");
            TEST_ASSERT(fs::exists(b.interiorMapPath), "Interior map file must exist on disk: " + b.interiorMapPath);

            // Parse JSON directly
            std::ifstream file(b.interiorMapPath);
            TEST_ASSERT(file.is_open(), "Could not open interior JSON: " + b.interiorMapPath);
            nlohmann::json root;
            file >> root;

            int w = root.value("width", 0);
            int h = root.value("height", 0);
            TEST_ASSERT(w >= 16 && h >= 12, "Interior dimensions must be at least 16x12 tiles");

            // Verify layers: Ground, Walls, Collision, Objects
            std::vector<std::string> layerNames;
            for (const auto& layer : root["layers"]) {
                layerNames.push_back(layer.value("name", ""));
            }
            auto hasLayer = [&](const std::string& ln) {
                return std::find(layerNames.begin(), layerNames.end(), ln) != layerNames.end();
            };
            TEST_ASSERT(hasLayer("Ground"), "Must have Ground layer in " + b.interiorMapPath);
            TEST_ASSERT(hasLayer("Walls"), "Must have Walls layer in " + b.interiorMapPath);
            TEST_ASSERT(hasLayer("Collision"), "Must have Collision layer in " + b.interiorMapPath);
            TEST_ASSERT(hasLayer("Objects"), "Must have Objects layer in " + b.interiorMapPath);

            // Verify spawn and exit coordinates are defined
            TEST_ASSERT(b.interiorSpawnPosition.x > 0.0f && b.interiorSpawnPosition.y > 0.0f, "Interior spawn must be positive");
            TEST_ASSERT(b.interiorExitPosition.x > 0.0f && b.interiorExitPosition.y > 0.0f, "Interior exit must be positive");
            TEST_ASSERT(b.exteriorReturnPosition.x > 0.0f && b.exteriorReturnPosition.y > 0.0f, "Exterior return must be positive");

            // Test loading map into Tilemap
            auto tilemap = MapLoader::LoadMap(b.interiorMapPath, assetMgr);
            TEST_ASSERT(tilemap != nullptr, "MapLoader should successfully load interior map: " + b.interiorMapPath);
            TEST_ASSERT(tilemap->GetWidth() == w, "Loaded tilemap width should match");
            TEST_ASSERT(tilemap->GetHeight() == h, "Loaded tilemap height should match");
        }
        std::cout << "  -> All 6 interior maps validated (layers, collision, objects, coordinates). [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 4: Entrance & Exit Proximity Detection
    // ========================================================================
    {
        std::cout << "[TEST] 4. Entrance & exit proximity detection..." << std::endl;
        BuildingManager mgr;
        mgr.Initialize("data/buildings/village_buildings.json");

        const Building* bStore = mgr.GetBuilding("village_general_store");
        TEST_ASSERT(bStore != nullptr, "General store should exist");

        // Player directly at store entrance
        const Building* prompt = mgr.CheckEntrancePrompt(bStore->entrancePosition);
        TEST_ASSERT(prompt != nullptr && prompt->id == "village_general_store", "Prompt should return general store");

        // Player just inside entrance radius
        Vector2 nearPos{ bStore->entrancePosition.x + 20.0f, bStore->entrancePosition.y };
        TEST_ASSERT(mgr.CheckEntrancePrompt(nearPos) != nullptr, "Should detect within entrance radius");

        // Player outside entrance radius
        Vector2 farPos{ bStore->entrancePosition.x + 100.0f, bStore->entrancePosition.y };
        TEST_ASSERT(mgr.CheckEntrancePrompt(farPos) == nullptr, "Should not detect far outside entrance radius");

        // Nearest building finder
        const Building* nearest = mgr.FindNearestBuilding(bStore->entrancePosition, 50.0f);
        TEST_ASSERT(nearest != nullptr && nearest->id == "village_general_store", "Nearest building should match");
        std::cout << "  -> Proximity checks accurately trigger entrance prompts. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 5: Indoor/Outdoor Transitions & State Machine
    // ========================================================================
    {
        std::cout << "[TEST] 5. Indoor/outdoor transitions & state machine..." << std::endl;
        BuildingManager mgr;
        mgr.Initialize("data/buildings/village_buildings.json", "maps/forest/forest_01.json");

        World world;
        bool mapLoaded = world.LoadMap("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(mapLoaded, "Should load forest_01 map");

        wildbound::entities::Player player(1, "TestHero");
        player.Spawn(Vector2{ 3868.0f, 816.0f });

        wildbound::graphics::Camera2DSystem camera(640, 360);
        camera.SetWorldBounds(world.GetWorldBounds());

        // Initial state
        TEST_ASSERT(!mgr.IsIndoors(), "Initially outdoors");
        TEST_ASSERT(mgr.GetCurrentInteriorBuilding() == nullptr, "No interior building initially");
        Rectangle exteriorBounds = world.GetWorldBounds();
        TEST_ASSERT(exteriorBounds.width == 5120.0f && exteriorBounds.height == 3840.0f, "Exterior bounds should be 5120x3840");

        // 1. Enter General Store
        bool enterOk = mgr.EnterBuilding("village_general_store", world, player, camera, assetMgr);
        TEST_ASSERT(enterOk, "EnterBuilding should succeed");
        TEST_ASSERT(mgr.IsIndoors(), "Should now be indoors");
        TEST_ASSERT(mgr.GetCurrentInteriorId() == "village_general_store", "Current interior should be village_general_store");
        TEST_ASSERT(mgr.GetCurrentInteriorBuilding() != nullptr, "Current interior building should not be null");

        // Player position should be at interior spawn
        Vector2 pPos = player.GetPosition();
        TEST_ASSERT(ApproxEqual(pPos.x, 320.0f) && ApproxEqual(pPos.y, 416.0f), "Player should be at interior spawn point");

        // Camera world bounds should be clamped to interior (640x480)
        Rectangle interiorBounds = world.GetWorldBounds();
        TEST_ASSERT(interiorBounds.width == 640.0f && interiorBounds.height == 480.0f, "Camera bounds should clamp to 640x480");

        // Proximity checks inside: entrance prompts disabled, exit prompt enabled near door
        TEST_ASSERT(mgr.CheckEntrancePrompt(player.GetPosition()) == nullptr, "Entrance prompt should be null indoors");
        player.SetPosition(Vector2{ 320.0f, 448.0f }); // Move to door
        TEST_ASSERT(mgr.CheckExitPrompt(player.GetPosition()), "Exit prompt should be true near door");

        // Cannot enter another building while indoors
        TEST_ASSERT(!mgr.EnterBuilding("village_blacksmith", world, player, camera, assetMgr), "Cannot enter while already indoors");

        // 2. Exit to Exterior
        bool exitOk = mgr.ExitBuilding(world, player, camera, assetMgr);
        TEST_ASSERT(exitOk, "ExitBuilding should succeed");
        TEST_ASSERT(!mgr.IsIndoors(), "Should now be outdoors");
        TEST_ASSERT(mgr.GetCurrentInteriorId().empty(), "Current interior should be empty");

        // Player should be back outside at exteriorReturnPosition (3868, 816)
        Vector2 returnPos = player.GetPosition();
        TEST_ASSERT(ApproxEqual(returnPos.x, 3868.0f) && ApproxEqual(returnPos.y, 816.0f), "Player should be returned to exterior position");

        // Camera bounds should be restored to 5120x3840
        Rectangle restoredBounds = world.GetWorldBounds();
        TEST_ASSERT(restoredBounds.width == 5120.0f && restoredBounds.height == 3840.0f, "Camera bounds restored to exterior map");

        // Cannot exit while already outdoors
        TEST_ASSERT(!mgr.ExitBuilding(world, player, camera, assetMgr), "Cannot exit while outdoors");

        // Cannot enter non-enterable building
        TEST_ASSERT(!mgr.EnterBuilding("village_mason_cottage", world, player, camera, assetMgr), "Non-enterable building must fail to enter");

        std::cout << "  -> Complete transition lifecycle verified (Exterior <-> Interior, camera clamping, coordinate restoration). [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 6: Interior Collision Cache Rebuilding
    // ========================================================================
    {
        std::cout << "[TEST] 6. Interior collision cache rebuilding..." << std::endl;
        World world;
        bool loaded = world.LoadMap("maps/interiors/interior_blacksmith.json", assetMgr);
        TEST_ASSERT(loaded, "Blacksmith interior map should load");

        // Should have solid tile-level collision on walls and solid boxes on props
        auto collisionBoxes = world.GetCollisionBoxes(Rectangle{ 0.0f, 0.0f, 640.0f, 480.0f });
        TEST_ASSERT(!collisionBoxes.empty(), "Interior map must generate collision boxes");
        TEST_ASSERT(collisionBoxes.size() >= 50, "Perimeter walls and forge must produce substantial collision boxes");

        // Check boundary wall collision
        Rectangle topWallPoint{ 320.0f, 16.0f, 10.0f, 10.0f };
        TEST_ASSERT(world.CheckCollision(topWallPoint), "Top perimeter wall must be impassable");

        // Check center floor is traversable
        Rectangle centerFloorPoint{ 320.0f, 250.0f, 10.0f, 10.0f };
        TEST_ASSERT(!world.CheckCollision(centerFloorPoint), "Center floor must be traversable");
        std::cout << "  -> Interior collision cache built with " << collisionBoxes.size()
                  << " collision bounds. [PASSED]" << std::endl;
    }

    CloseWindow();
    std::cout << "\n>>> ALL VILLAGE & INTERIOR SYSTEM TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
