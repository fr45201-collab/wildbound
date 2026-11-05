#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

#include "core/Logger.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct EnvironmentTestDef {
    std::string id;
    std::string category;     // "nature", "building", "prop"
    std::string subcategory;  // "trees/oak", "houses", "fences", etc.
    std::string rel_path;
    int expected_w;
    int expected_h;
    bool has_collision;
    bool is_building;
};

const std::vector<EnvironmentTestDef> ENVIRONMENT_ASSETS = {
    // 1. Nature: Oak Trees (4)
    { "tree_oak_001", "nature", "trees/oak", "environment/nature/trees/oak/tree_oak_001", 48, 64, true, false },
    { "tree_oak_002", "nature", "trees/oak", "environment/nature/trees/oak/tree_oak_002", 64, 80, true, false },
    { "tree_oak_003", "nature", "trees/oak", "environment/nature/trees/oak/tree_oak_003", 80, 96, true, false },
    { "tree_oak_004", "nature", "trees/oak", "environment/nature/trees/oak/tree_oak_004", 112, 128, true, false },

    // 1. Nature: Pine Trees (3)
    { "tree_pine_001", "nature", "trees/pine", "environment/nature/trees/pine/tree_pine_001", 32, 64, true, false },
    { "tree_pine_002", "nature", "trees/pine", "environment/nature/trees/pine/tree_pine_002", 48, 80, true, false },
    { "tree_pine_003", "nature", "trees/pine", "environment/nature/trees/pine/tree_pine_003", 64, 96, true, false },

    // 1. Nature: Bushes (5)
    { "bush_small_001", "nature", "bushes", "environment/nature/bushes/bush_small_001", 24, 24, false, false },
    { "bush_round_001", "nature", "bushes", "environment/nature/bushes/bush_round_001", 32, 32, true, false },
    { "bush_dense_001", "nature", "bushes", "environment/nature/bushes/bush_dense_001", 40, 32, true, false },
    { "bush_flowering_001", "nature", "bushes", "environment/nature/bushes/bush_flowering_001", 32, 32, true, false },
    { "bush_large_001", "nature", "bushes", "environment/nature/bushes/bush_large_001", 48, 36, true, false },

    // 1. Nature: Grass (6)
    { "grass_small_001", "nature", "grass", "environment/nature/grass/grass_small_001", 16, 16, false, false },
    { "grass_dense_001", "nature", "grass", "environment/nature/grass/grass_dense_001", 24, 20, false, false },
    { "grass_tall_001", "nature", "grass", "environment/nature/grass/grass_tall_001", 24, 32, false, false },
    { "grass_dry_001", "nature", "grass", "environment/nature/grass/grass_dry_001", 20, 20, false, false },
    { "grass_flowering_001", "nature", "grass", "environment/nature/grass/grass_flowering_001", 24, 24, false, false },
    { "grass_forest_001", "nature", "grass", "environment/nature/grass/grass_forest_001", 32, 24, false, false },

    // 1. Nature: Flowers (5)
    { "flower_white_001", "nature", "flowers", "environment/nature/flowers/flower_white_001", 16, 16, false, false },
    { "flower_yellow_001", "nature", "flowers", "environment/nature/flowers/flower_yellow_001", 16, 16, false, false },
    { "flower_red_001", "nature", "flowers", "environment/nature/flowers/flower_red_001", 16, 16, false, false },
    { "flower_blue_001", "nature", "flowers", "environment/nature/flowers/flower_blue_001", 16, 16, false, false },
    { "flower_wildflower_001", "nature", "flowers", "environment/nature/flowers/flower_wildflower_001", 24, 20, false, false },

    // 1. Nature: Rocks (5)
    { "rock_small_001", "nature", "rocks", "environment/nature/rocks/rock_small_001", 16, 16, false, false },
    { "rock_medium_001", "nature", "rocks", "environment/nature/rocks/rock_medium_001", 24, 20, true, false },
    { "rock_boulder_001", "nature", "rocks", "environment/nature/rocks/rock_boulder_001", 48, 40, true, false },
    { "rock_flat_001", "nature", "rocks", "environment/nature/rocks/rock_flat_001", 32, 20, false, false },
    { "rock_mossy_001", "nature", "rocks", "environment/nature/rocks/rock_mossy_001", 32, 28, true, false },

    // 1. Nature: Mushrooms (4)
    { "mushroom_small_001", "nature", "mushrooms", "environment/nature/mushrooms/mushroom_small_001", 16, 16, false, false },
    { "mushroom_red_001", "nature", "mushrooms", "environment/nature/mushrooms/mushroom_red_001", 16, 20, false, false },
    { "mushroom_brown_001", "nature", "mushrooms", "environment/nature/mushrooms/mushroom_brown_001", 20, 20, false, false },
    { "mushroom_cluster_001", "nature", "mushrooms", "environment/nature/mushrooms/mushroom_cluster_001", 24, 20, false, false },

    // 1. Nature: Logs (3)
    { "log_branch_001", "nature", "logs", "environment/nature/logs/log_branch_001", 24, 16, false, false },
    { "log_medium_001", "nature", "logs", "environment/nature/logs/log_medium_001", 48, 24, true, false },
    { "log_trunk_001", "nature", "logs", "environment/nature/logs/log_trunk_001", 64, 32, true, false },

    // 2. Buildings: Small Houses (3)
    { "house_cottage_001", "building", "houses", "environment/buildings/houses/house_cottage_001", 80, 80, true, true },
    { "house_stone_small_001", "building", "houses", "environment/buildings/houses/house_stone_small_001", 80, 80, true, true },
    { "house_village_001", "building", "houses", "environment/buildings/houses/house_village_001", 96, 88, true, true },

    // 2. Buildings: Large Houses (2)
    { "house_residence_large_001", "building", "houses", "environment/buildings/houses/house_residence_large_001", 112, 96, true, true },
    { "house_twostory_001", "building", "houses", "environment/buildings/houses/house_twostory_001", 120, 112, true, true },

    // 2. Buildings: Cabins (3)
    { "cabin_forest_001", "building", "cabins", "environment/buildings/cabins/cabin_forest_001", 72, 72, true, true },
    { "cabin_hunting_001", "building", "cabins", "environment/buildings/cabins/cabin_hunting_001", 72, 72, true, true },
    { "cabin_woodland_001", "building", "cabins", "environment/buildings/cabins/cabin_woodland_001", 80, 76, true, true },

    // 2. Buildings: Shops (2)
    { "shop_general_001", "building", "shops", "environment/buildings/shops/shop_general_001", 96, 88, true, true },
    { "shop_market_001", "building", "shops", "environment/buildings/shops/shop_market_001", 88, 80, true, true },

    // 2. Buildings: Blacksmith (1)
    { "blacksmith_forge_001", "building", "blacksmith", "environment/buildings/blacksmith/blacksmith_forge_001", 112, 96, true, true },

    // 2. Buildings: Storage (2)
    { "storage_shed_001", "building", "storage", "environment/buildings/storage/storage_shed_001", 64, 64, true, true },
    { "storage_warehouse_001", "building", "storage", "environment/buildings/storage/storage_warehouse_001", 96, 88, true, true },

    // 2. Buildings: Watchtowers (2)
    { "watchtower_wood_001", "building", "watchtowers", "environment/buildings/watchtowers/watchtower_wood_001", 64, 112, true, true },
    { "watchtower_stone_001", "building", "watchtowers", "environment/buildings/watchtowers/watchtower_stone_001", 64, 120, true, true },

    // 2. Buildings: Ruins (3)
    { "ruin_wall_broken_001", "building", "ruins", "environment/buildings/ruins/ruin_wall_broken_001", 64, 40, true, true },
    { "ruin_building_abandoned_001", "building", "ruins", "environment/buildings/ruins/ruin_building_abandoned_001", 96, 80, true, true },
    { "ruin_tower_small_001", "building", "ruins", "environment/buildings/ruins/ruin_tower_small_001", 64, 88, true, true },

    // 3. Props: Fences (4)
    { "fence_straight_001", "prop", "fences", "environment/props/fences/fence_straight_001", 32, 24, true, false },
    { "fence_corner_001", "prop", "fences", "environment/props/fences/fence_corner_001", 32, 32, true, false },
    { "fence_gate_001", "prop", "fences", "environment/props/fences/fence_gate_001", 32, 24, true, false },
    { "fence_broken_001", "prop", "fences", "environment/props/fences/fence_broken_001", 32, 24, true, false },

    // 3. Props: Barrels (3)
    { "barrel_standard_001", "prop", "barrels", "environment/props/barrels/barrel_standard_001", 20, 24, true, false },
    { "barrel_small_001", "prop", "barrels", "environment/props/barrels/barrel_small_001", 16, 18, true, false },
    { "barrel_banded_001", "prop", "barrels", "environment/props/barrels/barrel_banded_001", 22, 26, true, false },

    // 3. Props: Crates (3)
    { "crate_small_001", "prop", "crates", "environment/props/crates/crate_small_001", 20, 20, true, false },
    { "crate_large_001", "prop", "crates", "environment/props/crates/crate_large_001", 32, 32, true, false },
    { "crate_reinforced_001", "prop", "crates", "environment/props/crates/crate_reinforced_001", 28, 28, true, false },

    // 3. Props: Wells (2)
    { "well_stone_001", "prop", "wells", "environment/props/wells/well_stone_001", 36, 40, true, false },
    { "well_wooden_001", "prop", "wells", "environment/props/wells/well_wooden_001", 36, 44, true, false },

    // 3. Props: Lamps (3)
    { "lamp_post_001", "prop", "lamps", "environment/props/lamps/lamp_post_001", 20, 48, true, false },
    { "lamp_wall_001", "prop", "lamps", "environment/props/lamps/lamp_wall_001", 16, 24, false, false },
    { "lamp_hanging_001", "prop", "lamps", "environment/props/lamps/lamp_hanging_001", 16, 32, false, false },

    // 3. Props: Signs (4)
    { "sign_direction_001", "prop", "signs", "environment/props/signs/sign_direction_001", 24, 36, true, false },
    { "sign_shop_001", "prop", "signs", "environment/props/signs/sign_shop_001", 24, 28, false, false },
    { "sign_noticeboard_001", "prop", "signs", "environment/props/signs/sign_noticeboard_001", 36, 36, true, false },
    { "sign_warning_001", "prop", "signs", "environment/props/signs/sign_warning_001", 20, 32, true, false },

    // 3. Props: Bridges (3)
    { "bridge_wood_small_001", "prop", "bridges", "environment/props/bridges/bridge_wood_small_001", 64, 32, false, false },
    { "bridge_wood_large_001", "prop", "bridges", "environment/props/bridges/bridge_wood_large_001", 96, 48, false, false },
    { "bridge_stone_001", "prop", "bridges", "environment/props/bridges/bridge_stone_001", 96, 48, false, false },

    // 3. Props: Benches (2)
    { "bench_wooden_001", "prop", "benches", "environment/props/benches/bench_wooden_001", 32, 20, true, false },
    { "bench_stone_001", "prop", "benches", "environment/props/benches/bench_stone_001", 32, 20, true, false }
};

void TestAssetFileIntegrity() {
    std::cout << "[TEST] Running Environment Asset File Integrity Validation..." << std::endl;
    assert(ENVIRONMENT_ASSETS.size() == 77 && "Expected exactly 77 production environment assets");

    for (const auto& a : ENVIRONMENT_ASSETS) {
        fs::path png_path = fs::path("assets") / (a.rel_path + ".png");
        fs::path json_path = fs::path("assets") / (a.rel_path + ".json");

        assert(fs::exists(png_path) && "PNG file missing on disk");
        assert(fs::file_size(png_path) > 0 && "PNG file has zero size");

        assert(fs::exists(json_path) && "Metadata JSON file missing on disk");
        assert(fs::file_size(json_path) > 0 && "Metadata JSON file has zero size");
    }

    std::cout << "  -> Verified all 77 PNG and JSON metadata files exist on disk." << std::endl;
}

void TestMetadataSchema() {
    std::cout << "[TEST] Running Metadata Schema & Technical Validation..." << std::endl;

    for (const auto& a : ENVIRONMENT_ASSETS) {
        fs::path json_path = fs::path("assets") / (a.rel_path + ".json");
        std::ifstream file(json_path);
        assert(file.is_open());

        json meta;
        file >> meta;

        // Core schema
        assert(meta.contains("id") && meta["id"] == a.id);
        assert(meta.contains("name") && !meta["name"].get<std::string>().empty());
        assert(meta.contains("category") && meta["category"] == a.category);
        assert(meta.contains("subcategory") && meta["subcategory"] == a.subcategory);
        assert(meta.contains("biomes") && meta["biomes"].is_array());

        // Dimensions
        assert(meta.contains("dimensions"));
        int w = meta["dimensions"]["width"].get<int>();
        int h = meta["dimensions"]["height"].get<int>();
        (void)w;
        (void)h;
        assert(w == a.expected_w && "Width mismatch");
        assert(h == a.expected_h && "Height mismatch");

        // Anchors
        assert(meta.contains("anchors") && meta["anchors"].contains("ground"));
        int ax = meta["anchors"]["ground"][0].get<int>();
        int ay = meta["anchors"]["ground"][1].get<int>();
        (void)ax;
        (void)ay;
        assert(ax >= 0 && ax <= w);
        assert(ay >= 0 && ay <= h);

        // Collision
        assert(meta.contains("collision") && meta["collision"].contains("enabled"));
        bool colEnabled = meta["collision"]["enabled"].get<bool>();
        (void)colEnabled;
        assert(colEnabled == a.has_collision);
        if (colEnabled) {
            assert(meta["collision"].contains("shape") && meta["collision"]["shape"] == "rectangle");
            assert(meta["collision"].contains("size"));
            int cw = meta["collision"]["size"][0].get<int>();
            int ch = meta["collision"]["size"][1].get<int>();
            (void)cw;
            (void)ch;
            assert(cw > 0 && ch > 0);
        }

        // Rendering
        assert(meta.contains("rendering"));
        assert(meta["rendering"].contains("layer"));
        assert(meta["rendering"].contains("y_sort"));

        // Provenance
        assert(meta.contains("provenance"));
        assert(meta["provenance"].contains("license"));

        // Building architecture specific
        if (a.is_building) {
            assert(meta.contains("interior_compatibility"));
            assert(meta["interior_compatibility"].contains("supported"));
        }
    }

    std::cout << "  -> Metadata schema and bounds verified for all 77 assets." << std::endl;
}

void TestManifestRegistration() {
    std::cout << "[TEST] Running Asset Manifest Registration Validation..." << std::endl;

    fs::path manifest_path("assets/manifests/asset_manifest.json");
    assert(fs::exists(manifest_path));

    std::ifstream file(manifest_path);
    assert(file.is_open());

    json manifest;
    file >> manifest;
    assert(manifest.contains("assets") && manifest["assets"].is_array());
    assert(manifest["assets"].size() >= 200 && "Expected at least 200 registered assets in manifest");

    std::unordered_set<std::string> registered_ids;
    for (const auto& item : manifest["assets"]) {
        if (item.contains("id")) {
            registered_ids.insert(item["id"].get<std::string>());
        }
    }

    // Verify all 77 environment texture IDs
    for (const auto& a : ENVIRONMENT_ASSETS) {
        std::string expected_tex_id = "tex_" + a.id;
        (void)expected_tex_id;
        assert(registered_ids.count(expected_tex_id) == 1 && "Missing environment asset texture in manifest");
    }

    std::cout << "  -> Asset manifest contains all 77 environment textures (Total: "
              << manifest["assets"].size() << " assets)." << std::endl;
}

int main() {
    wildbound::core::Logger::Init();
    std::cout << "====================================================" << std::endl;
    std::cout << "   WILDBOUND PHASE 10 — ENVIRONMENT ASSET TEST" << std::endl;
    std::cout << "====================================================" << std::endl;

    TestAssetFileIntegrity();
    TestMetadataSchema();
    TestManifestRegistration();

    std::cout << "====================================================" << std::endl;
    std::cout << "   ALL ENVIRONMENT ASSET TESTS PASSED!" << std::endl;
    std::cout << "====================================================" << std::endl;
    return 0;
}

