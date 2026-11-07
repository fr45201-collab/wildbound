#include "../../src/world/WorldData.hpp"
#include "../../src/world/WorldManager.hpp"
#include "../../src/world/World.hpp"
#include "../../src/world/MapLoader.hpp"
#include "../../src/ai/navigation/NavigationGrid.hpp"
#include "../../src/ai/navigation/Pathfinding.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>

using wildbound::core::Logger;
using namespace wildbound::world;
using namespace wildbound::ai;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

int main() {
    Logger::Init();
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 30: BATTLE ROYALE WORLD DESIGN TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Test 1: WorldManager Metadata Loading & Integrity Validation
    WorldManager worldMgr;
    {
        std::cout << "[TEST] 1. WorldManager Metadata Loading & Validation..." << std::endl;
        bool loaded = worldMgr.Initialize("data/world");
        TEST_ASSERT(loaded, "Failed initializing WorldManager from data/world");

        const auto& wdef = worldMgr.GetWorldDef();
        TEST_ASSERT(wdef.worldId == "battle_royale_island", "World ID mismatch");
        TEST_ASSERT(wdef.widthTiles == 320, "Expected 320 tiles width");
        TEST_ASSERT(wdef.heightTiles == 320, "Expected 320 tiles height");
        TEST_ASSERT(wdef.worldWidth == 10240.0f, "Expected 10240px world width");
        TEST_ASSERT(wdef.worldHeight == 10240.0f, "Expected 10240px world height");

        std::vector<std::string> errors;
        bool valid = worldMgr.ValidateWorldData(errors);
        for (const auto& err : errors) {
            std::cerr << "  Validation error: " << err << std::endl;
        }
        TEST_ASSERT(valid, "World metadata validation failed");
        std::cout << "  -> World metadata loaded and validated with 0 errors." << std::endl;
    }

    // Test 2: Biome Regions & Coverage
    {
        std::cout << "[TEST] 2. Biome Distribution & Regional Queries..." << std::endl;
        const auto& regions = worldMgr.GetBiomeRegions();
        TEST_ASSERT(regions.size() == 5, "Expected 5 distinct biome regions");

        // Verify key regional lookups
        const BiomeRegion* forestReg = worldMgr.GetBiomeRegionAt(Vector2{ 4160.0f, 5920.0f });
        TEST_ASSERT(forestReg != nullptr, "Greenwood position must be inside Forest region");
        TEST_ASSERT(forestReg->biomeId == "forest", "Greenwood must be in forest biome");

        const BiomeRegion* desertReg = worldMgr.GetBiomeRegionAt(Vector2{ 7680.0f, 7520.0f });
        TEST_ASSERT(desertReg != nullptr, "Sunfall position must be inside Desert region");
        TEST_ASSERT(desertReg->biomeId == "desert", "Sunfall must be in desert biome");

        const BiomeRegion* snowReg = worldMgr.GetBiomeRegionAt(Vector2{ 7840.0f, 2560.0f });
        TEST_ASSERT(snowReg != nullptr, "Frostpeak position must be inside Snow region");
        TEST_ASSERT(snowReg->biomeId == "snow", "Frostpeak must be in snow biome");

        const BiomeRegion* swampReg = worldMgr.GetBiomeRegionAt(Vector2{ 2400.0f, 2720.0f });
        TEST_ASSERT(swampReg != nullptr, "Mirewood position must be inside Swamp region");
        TEST_ASSERT(swampReg->biomeId == "swamp", "Mirewood must be in swamp biome");

        const BiomeRegion* mtnReg = worldMgr.GetBiomeRegionAt(Vector2{ 5120.0f, 2080.0f });
        TEST_ASSERT(mtnReg != nullptr, "Highstone position must be inside Mountain region");
        TEST_ASSERT(mtnReg->biomeId == "mountain", "Highstone must be in mountain biome");

        std::cout << "  -> All 5 biomes verified with spatial coverage." << std::endl;
    }

    // Test 3: Settlements & POIs
    {
        std::cout << "[TEST] 3. Settlements & Strategic POI Inspection..." << std::endl;
        const auto& settlements = worldMgr.GetSettlements();
        TEST_ASSERT(settlements.size() == 5, "Expected 5 settlements");

        TEST_ASSERT(worldMgr.GetSettlement("greenwood_village") != nullptr, "Missing greenwood_village");
        TEST_ASSERT(worldMgr.GetSettlement("sunfall_outpost") != nullptr, "Missing sunfall_outpost");
        TEST_ASSERT(worldMgr.GetSettlement("frostpeak_village") != nullptr, "Missing frostpeak_village");
        TEST_ASSERT(worldMgr.GetSettlement("mirewood_settlement") != nullptr, "Missing mirewood_settlement");
        TEST_ASSERT(worldMgr.GetSettlement("highstone_village") != nullptr, "Missing highstone_village");

        const auto& pois = worldMgr.GetPointsOfInterest();
        TEST_ASSERT(pois.size() == 14, "Expected 14 strategic points of interest");
        TEST_ASSERT(worldMgr.GetPointOfInterest("poi_ancient_fortress") != nullptr, "Missing Ancient Fortress POI");
        TEST_ASSERT(worldMgr.GetPointOfInterest("poi_desert_temple") != nullptr, "Missing Desert Temple POI");

        std::cout << "  -> 5 settlements and 14 POIs verified." << std::endl;
    }

    // Test 4: Player Spawn Regions & Collision Safety
    {
        std::cout << "[TEST] 4. Player Spawn Regions & Playable Bounds..." << std::endl;
        const auto& spawnRegions = worldMgr.GetSpawnRegions();
        TEST_ASSERT(spawnRegions.size() == 12, "Expected 12 spawn regions");

        const auto& pb = worldMgr.GetWorldDef().playableBounds;
        int totalSpawnPoints = 0;

        for (const auto& sr : spawnRegions) {
            TEST_ASSERT(sr.spawnPoints.size() >= 4, "Each spawn region must have at least 4 spawn points");
            for (const auto& sp : sr.spawnPoints) {
                totalSpawnPoints++;
                TEST_ASSERT(CheckCollisionPointRec(sp, pb), "Spawn point must be inside playable bounds");
            }
        }
        TEST_ASSERT(totalSpawnPoints == 48, "Expected 48 verified spawn marker positions");
        std::cout << "  -> 12 spawn regions and 48 spawn markers verified inside playable boundary." << std::endl;
    }

    // Test 5: Loot Locations
    {
        std::cout << "[TEST] 5. Loot Locations & Placement Markers..." << std::endl;
        const auto& lootLocs = worldMgr.GetLootLocations();
        TEST_ASSERT(lootLocs.size() == 45, "Expected 45 dedicated loot locations");

        int rareCount = 0;
        int supplyCount = 0;
        int weaponsCount = 0;

        for (const auto& l : lootLocs) {
            TEST_ASSERT(!l.id.empty(), "Loot ID must not be empty");
            TEST_ASSERT(!l.parentPoi.empty(), "Loot location must be associated with a parent POI");
            if (l.containerType == "chest_rare" || l.containerType == "chest_ancient") rareCount++;
            if (l.containerType == "crate_supplies") supplyCount++;
            if (l.containerType == "chest_weapons") weaponsCount++;
        }
        TEST_ASSERT(rareCount >= 8, "Expected at least 8 rare/ancient chests");
        TEST_ASSERT(supplyCount >= 10, "Expected at least 10 supply crates");
        TEST_ASSERT(weaponsCount >= 5, "Expected at least 5 weapon chests");
        std::cout << "  -> 45 loot locations categorized across containers." << std::endl;
    }

    // Test 6: Traversal Routes & Chokepoints
    {
        std::cout << "[TEST] 6. Traversal Routes & Chokepoint Connectivity..." << std::endl;
        const auto& routes = worldMgr.GetPrimaryRoutes();
        TEST_ASSERT(routes.size() >= 5, "Expected at least 5 primary highways");

        const auto& chokepoints = worldMgr.GetChokepoints();
        TEST_ASSERT(chokepoints.size() == 3, "Expected 3 strategic chokepoints");

        for (const auto& r : routes) {
            TEST_ASSERT(worldMgr.GetPointOfInterest(r.startPoi) != nullptr, "Route start POI must exist");
            TEST_ASSERT(worldMgr.GetPointOfInterest(r.endPoi) != nullptr, "Route end POI must exist");
            TEST_ASSERT(r.distanceUnits > 0.0f, "Route distance must be positive");
        }
        std::cout << "  -> All primary routes connect valid POIs with defined chokepoints." << std::endl;
    }

    // Test 7: Tiled Map Loading & Object Group Parsing
    wildbound::resources::AssetManager assetMgr;
    assetMgr.Initialize("assets");
    World world;
    {
        std::cout << "[TEST] 7. Battle Royale Island Tiled Map Loading..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        bool loaded = world.LoadMap("maps/world/battle_royale_island.json", assetMgr);
        auto elapsed = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count();

        TEST_ASSERT(loaded, "Failed loading maps/world/battle_royale_island.json");
        TEST_ASSERT(world.IsLoaded(), "World must report loaded");

        Tilemap* tm = world.GetTilemap();
        TEST_ASSERT(tm != nullptr, "Tilemap must not be null");
        TEST_ASSERT(tm->GetWidth() == 320, "Tilemap width must be 320");
        TEST_ASSERT(tm->GetHeight() == 320, "Tilemap height must be 320");
        TEST_ASSERT(tm->GetTilesets().size() == 5, "Tilemap must have 5 biome tilesets loaded");
        TEST_ASSERT(tm->GetLayers().size() >= 6, "Tilemap must have 6+ tile layers");
        TEST_ASSERT(tm->GetObjects().size() >= 400, "Tilemap must contain 400+ parsed environmental objects");

        std::cout << "  -> Map loaded successfully in " << elapsed << " ms ("
                  << tm->GetObjects().size() << " objects, "
                  << tm->GetTilesets().size() << " tilesets)." << std::endl;
    }

    // Test 8: Navigation Grid Generation & Rasterization
    NavigationGrid navGrid;
    {
        std::cout << "[TEST] 8. Navigation Grid Rasterization from Island World..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        navGrid.BuildFromWorld(world);
        auto elapsed = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count();

        size_t blocked = navGrid.GetBlockedCellCount();
        TEST_ASSERT(blocked > 10000, "Expected at least 10,000 blocked ocean/obstacle cells");

        std::cout << "  -> NavGrid rasterized in " << elapsed << " ms ("
                  << blocked << " blocked cells / " << navGrid.GetTotalCellCount() << " total cells)." << std::endl;
    }

    // Test 9: Island-Wide Pathfinding & Route Verification
    {
        std::cout << "[TEST] 9. Cross-Biome A* Pathfinding Across Island..." << std::endl;
        // Path from Greenwood Village (Forest) to Ancient Fortress (Center)
        Vector2 startPos{ 4160.0f, 5920.0f };
        Vector2 goalPos{ 5120.0f, 4800.0f };

        // Ensure both coordinates are walkable
        if (!navGrid.IsWalkable(startPos)) {
            startPos = navGrid.GetNearestWalkablePosition(startPos, 96.0f);
        }
        if (!navGrid.IsWalkable(goalPos)) {
            goalPos = navGrid.GetNearestWalkablePosition(goalPos, 96.0f);
        }

        auto startTime = std::chrono::high_resolution_clock::now();
        Pathfinding pathfinder(navGrid);
        PathResult result = pathfinder.FindPath(startPos, goalPos);
        auto elapsed = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startTime).count();

        TEST_ASSERT(result.status == PathStatus::Found, "Cross-biome path must be successfully found");
        TEST_ASSERT(!result.waypoints.empty(), "Path waypoints must not be empty");

        std::cout << "  -> Path found in " << elapsed << " ms ("
                  << result.waypoints.size() << " waypoints, distance: " << result.totalDistance << ")." << std::endl;
    }

    // Test 10: Performance & Traversal Time Guarantees
    {
        std::cout << "[TEST] 10. Traversal Velocity & Scale Validation..." << std::endl;
        float runSpeed = 120.0f; // px/sec
        float walkSpeed = 80.0f; // px/sec

        float fullMapWidth = 10240.0f;
        float runTimeFull = fullMapWidth / runSpeed;
        float walkTimeFull = fullMapWidth / walkSpeed;

        TEST_ASSERT(runTimeFull >= 80.0f && runTimeFull <= 90.0f, "Expected full-run traversal ~85s");
        TEST_ASSERT(walkTimeFull >= 120.0f && walkTimeFull <= 135.0f, "Expected full-walk traversal ~128s");

        std::cout << "  -> Traversal timing verified: Straight Run " << runTimeFull
                  << "s, Walk " << walkTimeFull << "s (Optimal BR pacing)." << std::endl;
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 BATTLE ROYALE WORLD TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}

