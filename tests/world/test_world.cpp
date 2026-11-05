#include "../../src/world/Tileset.hpp"
#include "../../src/world/TileLayer.hpp"
#include "../../src/world/CollisionMetadata.hpp"
#include "../../src/world/EnvironmentObject.hpp"
#include "../../src/world/Tilemap.hpp"
#include "../../src/world/MapLoader.hpp"
#include "../../src/world/MapValidator.hpp"
#include "../../src/world/World.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>

using wildbound::core::Logger;
using namespace wildbound::world;
using namespace wildbound::resources;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

static bool ApproxEqual(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) <= epsilon;
}

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound World & Tilemap System Tests ===" << std::endl;

    // Headless raylib context for texture preloading in tests
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundTest");

    // Headless AssetManager for tests
    AssetManager assetMgr;
    assetMgr.Initialize("assets");

    // ========================================================================
    // Test 1: Tileset Geometry and GID Source Rect Calculation
    // ========================================================================
    {
        std::cout << "[TEST] 1. Tileset geometry and GID source rect calculation..." << std::endl;
        // 8 cols, 64 tiles, 32x32 tiles, firstGid = 1
        Tileset ts(1, "test_tileset", 32, 32, 8, 64, "tilesets/forest/forest_tileset.png");
        TEST_ASSERT(ts.GetFirstGid() == 1, "First GID should be 1");
        TEST_ASSERT(ts.GetColumns() == 8, "Columns should be 8");
        TEST_ASSERT(ts.GetTileCount() == 64, "Tile count should be 64");
        TEST_ASSERT(ts.ContainsGid(1), "Should contain GID 1");
        TEST_ASSERT(ts.ContainsGid(64), "Should contain GID 64");
        TEST_ASSERT(!ts.ContainsGid(0), "Should not contain GID 0 (empty)");
        TEST_ASSERT(!ts.ContainsGid(65), "Should not contain GID 65");

        // GID 1: row 0, col 0 -> (0, 0, 32, 32)
        Rectangle r1 = ts.GetTileSourceRect(1);
        TEST_ASSERT(ApproxEqual(r1.x, 0.0f) && ApproxEqual(r1.y, 0.0f), "GID 1 source should be (0, 0)");
        TEST_ASSERT(ApproxEqual(r1.width, 32.0f) && ApproxEqual(r1.height, 32.0f), "Tile size should be 32x32");

        // GID 10: local ID 9 -> row 1, col 1 -> (32, 32, 32, 32)
        Rectangle r10 = ts.GetTileSourceRect(10);
        TEST_ASSERT(ApproxEqual(r10.x, 32.0f) && ApproxEqual(r10.y, 32.0f), "GID 10 source should be (32, 32)");

        // GID 17: local ID 16 -> row 2, col 0 -> (0, 64, 32, 32)
        Rectangle r17 = ts.GetTileSourceRect(17);
        TEST_ASSERT(ApproxEqual(r17.x, 0.0f) && ApproxEqual(r17.y, 64.0f), "GID 17 source should be (0, 64)");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 2: Tile Layer Grid Storage & Out-of-Bounds Safety
    // ========================================================================
    {
        std::cout << "[TEST] 2. Tile layer grid storage & boundary safety..." << std::endl;
        TileLayer layer(1, "Ground", 80, 60);
        TEST_ASSERT(layer.GetWidth() == 80, "Width should be 80");
        TEST_ASSERT(layer.GetHeight() == 60, "Height should be 60");
        TEST_ASSERT(layer.GetTotalTiles() == 4800, "Total tiles should be 4800");

        // Default should be 0
        TEST_ASSERT(layer.GetTileGid(10, 10) == 0, "Default tile should be 0");

        // Set and get valid tile
        layer.SetTileGid(15, 20, 9);
        TEST_ASSERT(layer.GetTileGid(15, 20) == 9, "Tile at (15, 20) should be 9");

        // Out-of-bounds access should return 0 safely without crashing
        TEST_ASSERT(layer.GetTileGid(-1, 0) == 0, "Negative x should return 0");
        TEST_ASSERT(layer.GetTileGid(0, -1) == 0, "Negative y should return 0");
        TEST_ASSERT(layer.GetTileGid(80, 10) == 0, "Out-of-bounds x should return 0");
        TEST_ASSERT(layer.GetTileGid(10, 60) == 0, "Out-of-bounds y should return 0");

        // Out-of-bounds set should be safely ignored
        layer.SetTileGid(-5, 100, 42);
        TEST_ASSERT(layer.GetTileGid(-5, 100) == 0, "Out-of-bounds set should not crash");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 3: Layer Visibility and Opacity Toggling
    // ========================================================================
    {
        std::cout << "[TEST] 3. Layer visibility and opacity..." << std::endl;
        TileLayer layer(2, "Water", 10, 10, 1.0f, true);
        TEST_ASSERT(layer.IsVisible(), "Layer should be visible by default");
        TEST_ASSERT(ApproxEqual(layer.GetOpacity(), 1.0f), "Opacity should be 1.0");

        layer.SetVisible(false);
        TEST_ASSERT(!layer.IsVisible(), "Layer visibility should toggle to false");

        layer.SetOpacity(0.65f);
        TEST_ASSERT(ApproxEqual(layer.GetOpacity(), 0.65f), "Opacity should update to 0.65");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 4: Coordinate Conversions (Tile <-> World)
    // ========================================================================
    {
        std::cout << "[TEST] 4. Coordinate conversions (Tile <-> World)..." << std::endl;
        Tilemap map("TestMap", 80, 60, 32, 32);

        // World to Tile
        Vector2 tileCoords1 = map.WorldToTile(Vector2{ 0.0f, 0.0f });
        TEST_ASSERT(tileCoords1.x == 0 && tileCoords1.y == 0, "(0, 0) world should map to (0, 0) tile");

        Vector2 tileCoords2 = map.WorldToTile(Vector2{ 64.0f, 96.0f });
        TEST_ASSERT(tileCoords2.x == 2 && tileCoords2.y == 3, "(64, 96) world should map to (2, 3) tile");

        Vector2 tileCoordsMid = map.WorldToTile(Vector2{ 70.0f, 100.0f });
        TEST_ASSERT(tileCoordsMid.x == 2 && tileCoordsMid.y == 3, "(70, 100) world should map to (2, 3) tile");

        // Tile to World
        Vector2 worldCoords = map.TileToWorld(2, 3);
        TEST_ASSERT(ApproxEqual(worldCoords.x, 64.0f) && ApproxEqual(worldCoords.y, 96.0f), "Tile (2, 3) should map to world (64, 96)");

        // Map World Bounds
        Rectangle bounds = map.GetWorldBounds();
        TEST_ASSERT(ApproxEqual(bounds.width, 80.0f * 32.0f), "Map world width should be 2560");
        TEST_ASSERT(ApproxEqual(bounds.height, 60.0f * 32.0f), "Map world height should be 1920");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 5: Collision Metadata and Intersection Testing
    // ========================================================================
    {
        std::cout << "[TEST] 5. Collision metadata and intersection..." << std::endl;
        CollisionBox box1;
        box1.bounds = Rectangle{ 100.0f, 100.0f, 50.0f, 50.0f };
        box1.type = CollisionType::Solid;

        // Intersection test
        Rectangle intersectingRect{ 120.0f, 120.0f, 40.0f, 40.0f };
        Rectangle separateRect{ 200.0f, 200.0f, 50.0f, 50.0f };
        TEST_ASSERT(box1.Intersects(intersectingRect), "box1 should intersect with intersectingRect");
        TEST_ASSERT(!box1.Intersects(separateRect), "box1 should not intersect with separateRect");

        // Point test
        TEST_ASSERT(box1.ContainsPoint(Vector2{ 125.0f, 125.0f }), "Point (125, 125) should be inside box1");
        TEST_ASSERT(!box1.ContainsPoint(Vector2{ 90.0f, 125.0f }), "Point (90, 125) should be outside box1");

        // String to Type conversion
        TEST_ASSERT(CollisionBox::StringToType("solid") == CollisionType::Solid, "String 'solid' should parse to Solid");
        TEST_ASSERT(CollisionBox::StringToType("water") == CollisionType::Water, "String 'water' should parse to Water");
        TEST_ASSERT(CollisionBox::StringToType("impassable") == CollisionType::Impassable, "String 'impassable' should parse to Impassable");
        TEST_ASSERT(CollisionBox::StringToType("walkable") == CollisionType::Walkable, "String 'walkable' should parse to Walkable");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 6: Environmental Object Depth (Y-Sorting) & Render Bounds
    // ========================================================================
    {
        std::cout << "[TEST] 6. Environmental object depth (Y-sorting)..." << std::endl;
        Vector2 pos1{ 100.0f, 200.0f };
        Vector2 size1{ 48.0f, 64.0f };
        Vector2 origin1{ 24.0f, 64.0f }; // Bottom-center anchor
        EnvironmentObject obj1(1, "Tree1", "oak_tree", pos1, size1, "tex_oak_tree",
                              Rectangle{ 0, 0, 48, 64 }, origin1);

        Vector2 pos2{ 100.0f, 150.0f };
        EnvironmentObject obj2(2, "Tree2", "oak_tree", pos2, size1, "tex_oak_tree",
                              Rectangle{ 0, 0, 48, 64 }, origin1);

        // Y-order for depth sorting
        TEST_ASSERT(obj2.GetZOrder() < obj1.GetZOrder(), "Object at y=150 should have lower ZOrder than object at y=200");

        // Render bounds offset by origin
        Rectangle rBounds = obj1.GetWorldRenderBounds();
        TEST_ASSERT(ApproxEqual(rBounds.x, 100.0f - 24.0f), "RenderBounds x should be 76");
        TEST_ASSERT(ApproxEqual(rBounds.y, 200.0f - 64.0f), "RenderBounds y should be 136");
        TEST_ASSERT(ApproxEqual(rBounds.width, 48.0f), "RenderBounds width should be 48");
        TEST_ASSERT(ApproxEqual(rBounds.height, 64.0f), "RenderBounds height should be 64");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 7: Map Loading from Tiled JSON (forest_01.json)
    // ========================================================================
    {
        std::cout << "[TEST] 7. Map loading from Tiled JSON (forest_01.json)..." << std::endl;
        auto map = MapLoader::LoadFromJson("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(map != nullptr, "MapLoader failed to load forest_01.json");
        TEST_ASSERT(map->GetName() == "forest_01", "Map name should be 'forest_01'");
        TEST_ASSERT(map->GetWidth() == 160, "Map width should be 160");
        TEST_ASSERT(map->GetHeight() == 120, "Map height should be 120");
        TEST_ASSERT(map->GetTileWidth() == 32, "Tile width should be 32");
        TEST_ASSERT(map->GetTileHeight() == 32, "Tile height should be 32");
        TEST_ASSERT(map->GetLayerCount() >= 5, "Map should have at least 5 layers");
        TEST_ASSERT(map->GetObjects().size() > 50, "Map should have parsed environmental objects (>50)");

        // Verify layers exist
        const TileLayer* ground = map->GetLayer("Ground");
        TEST_ASSERT(ground != nullptr, "Map should contain 'Ground' layer");
        TEST_ASSERT(ground->GetTileGid(0, 0) > 0, "Ground layer at (0, 0) should have valid tile GID");

        const TileLayer* water = map->GetLayer("Water");
        TEST_ASSERT(water != nullptr, "Map should contain 'Water' layer");

        std::cout << "  -> PASSED (Loaded " << map->GetObjects().size() << " objects across "
                  << map->GetLayerCount() << " layers)" << std::endl;
    }

    // ========================================================================
    // Test 8: Map Loading from Tiled TMX (forest_01.tmx)
    // ========================================================================
    {
        std::cout << "[TEST] 8. Map loading from Tiled TMX XML (forest_01.tmx)..." << std::endl;
        auto map = MapLoader::LoadFromTmx("maps/forest/forest_01.tmx", assetMgr);
        TEST_ASSERT(map != nullptr, "MapLoader failed to parse forest_01.tmx");
        TEST_ASSERT(map->GetWidth() == 160 && map->GetHeight() == 120, "TMX map dimensions should be 160x120");
        TEST_ASSERT(map->GetTileWidth() == 32 && map->GetTileHeight() == 32, "TMX tile size should be 32x32");
        TEST_ASSERT(map->GetLayerCount() >= 4, "TMX should have parsed tile layers");
        TEST_ASSERT(map->GetObjects().size() > 50, "TMX should have parsed environmental objects");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 9: Spatial Queries (Collision Boxes & Visible Objects)
    // ========================================================================
    {
        std::cout << "[TEST] 9. World spatial queries (Collision & Visible objects)..." << std::endl;
        World world;
        bool loaded = world.LoadMap("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(loaded, "World failed to initialize forest_01.json");

        // Query small area around cabin courtyard (x: 1800..2100, y: 350..650)
        Rectangle courtyardArea{ 1800.0f, 350.0f, 300.0f, 300.0f };
        auto colBoxes = world.GetCollisionBoxes(courtyardArea);
        TEST_ASSERT(!colBoxes.empty(), "Should find collision boxes in cabin courtyard area");

        auto visibleObjs = world.GetVisibleObjects(courtyardArea);
        TEST_ASSERT(!visibleObjs.empty(), "Should find visible objects in cabin courtyard area");

        // Query empty/remote area off-map
        Rectangle offMapArea{ 10000.0f, 10000.0f, 100.0f, 100.0f };
        auto emptyCol = world.GetCollisionBoxes(offMapArea);
        TEST_ASSERT(emptyCol.empty(), "Off-map query should return empty collision list");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 10: Map Validation Diagnostics (MapValidator)
    // ========================================================================
    {
        std::cout << "[TEST] 10. Map validation diagnostics (MapValidator)..." << std::endl;
        auto map = MapLoader::LoadFromJson("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(map != nullptr, "Map must load for validator test");

        MapValidationReport validReport = MapValidator::Validate(*map);
        TEST_ASSERT(validReport.IsValid(), "Valid forest_01 map should pass validation with zero fatal errors");
        TEST_ASSERT(validReport.errorsCount == 0, "Validator error count should be 0");

        // Now test validation failures on a corrupted map
        Tilemap corruptedMap("CorruptedMap", 0, 60, 32, 32); // Invalid width = 0
        MapValidationReport failReport1 = MapValidator::Validate(corruptedMap);
        TEST_ASSERT(!failReport1.IsValid(), "Validator must reject zero-dimension map");

        Tilemap missingTsMap("MissingTilesetMap", 10, 10, 32, 32); // No tilesets added
        TileLayer layer(1, "Ground", 10, 10);
        layer.SetTileGid(0, 0, 999); // GID without tileset
        missingTsMap.AddLayer(std::move(layer));
        MapValidationReport failReport2 = MapValidator::Validate(missingTsMap);
        TEST_ASSERT(!failReport2.IsValid(), "Validator must reject map with missing tilesets for GIDs");

        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 11: World Lifecycle & Cleanup
    // ========================================================================
    {
        std::cout << "[TEST] 11. World lifecycle and memory cleanup..." << std::endl;
        World world;
        TEST_ASSERT(world.GetTilemap() == nullptr, "Tilemap should be null initially");

        world.LoadMap("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(world.GetTilemap() != nullptr, "Tilemap should not be null after LoadMap");
        TEST_ASSERT(world.GetMapName() == "forest_01", "World map name should match");

        world.UnloadMap();
        TEST_ASSERT(world.GetTilemap() == nullptr, "Tilemap should be null after UnloadMap");
        TEST_ASSERT(world.GetWorldBounds().width == 0.0f, "World bounds should be zero after UnloadMap");

        // Reload to verify safe re-initialization
        world.LoadMap("maps/forest/forest_01.tmx", assetMgr);
        TEST_ASSERT(world.GetTilemap() != nullptr, "World should successfully reload map");
        world.UnloadMap();

        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "\n========================================================" << std::endl;
    std::cout << "All Wildbound World & Tilemap System tests passed! (11/11)" << std::endl;
    std::cout << "========================================================" << std::endl;

    CloseWindow();
    return 0;
}

