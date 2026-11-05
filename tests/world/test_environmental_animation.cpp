#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <unordered_set>
#include <nlohmann/json.hpp>

#include "core/Logger.hpp"
#include "world/environment/EnvironmentalAnimationSystem.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace wildbound::world::environment;

struct EnvAnimAssetTestDef {
    std::string id;
    std::string category;     // "water", "waterfalls", "fire", "grass", "trees", "clouds", "fog", "smoke", "airplanes"
    std::string rel_path;
    int expected_w;
    int expected_h;
    bool is_spritesheet;
    int expected_frames;
};

const std::vector<EnvAnimAssetTestDef> PHASE_11_ASSETS = {
    // Category A: Water (5)
    { "water_pond_anim", "water", "environment/animations/water/water_pond_anim", 128, 32, true, 4 },
    { "water_river_anim", "water", "environment/animations/water/water_river_anim", 128, 32, true, 4 },
    { "water_lake_anim", "water", "environment/animations/water/water_lake_anim", 256, 64, true, 4 },
    { "water_flow_anim", "water", "environment/animations/water/water_flow_anim", 128, 32, true, 4 },
    { "water_ripple_anim", "water", "environment/animations/water/water_ripple_anim", 64, 16, true, 4 },

    // Category A: Waterfalls (3) - 6 frames each
    { "waterfall_small_anim", "waterfalls", "environment/animations/waterfalls/waterfall_small_anim", 192, 48, true, 6 },
    { "waterfall_medium_anim", "waterfalls", "environment/animations/waterfalls/waterfall_medium_anim", 288, 64, true, 6 },
    { "waterfall_large_anim", "waterfalls", "environment/animations/waterfalls/waterfall_large_anim", 384, 80, true, 6 },

    // Category A: Fire (3) - 6 frames each
    { "fire_small_anim", "fire", "environment/animations/fire/fire_small_anim", 96, 24, true, 6 },
    { "fire_medium_anim", "fire", "environment/animations/fire/fire_medium_anim", 144, 32, true, 6 },
    { "fire_large_anim", "fire", "environment/animations/fire/fire_large_anim", 192, 48, true, 6 },

    // Category A: Grass (4) - 4 frames each
    { "grass_small_wind_anim", "grass", "environment/animations/grass/grass_small_wind_anim", 64, 16, true, 4 },
    { "grass_tall_wind_anim", "grass", "environment/animations/grass/grass_tall_wind_anim", 96, 32, true, 4 },
    { "grass_dense_wind_anim", "grass", "environment/animations/grass/grass_dense_wind_anim", 96, 20, true, 4 },
    { "grass_flowering_wind_anim", "grass", "environment/animations/grass/grass_flowering_wind_anim", 96, 24, true, 4 },

    // Category A: Trees (4)
    { "tree_oak_breeze_anim", "trees", "environment/animations/trees/tree_oak_breeze_anim", 192, 48, true, 4 },
    { "tree_pine_breeze_anim", "trees", "environment/animations/trees/tree_pine_breeze_anim", 128, 48, true, 4 },
    { "bush_rustle_anim", "trees", "environment/animations/trees/bush_rustle_anim", 128, 32, true, 4 },
    { "leaves_falling_anim", "trees", "environment/animations/trees/leaves_falling_anim", 192, 32, true, 6 },

    // Category C: Clouds (5)
    { "cloud_small", "clouds", "environment/atmosphere/clouds/cloud_small", 64, 32, false, 1 },
    { "cloud_medium", "clouds", "environment/atmosphere/clouds/cloud_medium", 96, 48, false, 1 },
    { "cloud_large", "clouds", "environment/atmosphere/clouds/cloud_large", 128, 64, false, 1 },
    { "cloud_thin", "clouds", "environment/atmosphere/clouds/cloud_thin", 160, 48, false, 1 },
    { "cloud_cluster", "clouds", "environment/atmosphere/clouds/cloud_cluster", 192, 96, false, 1 },

    // Category C: Fog (4)
    { "fog_light_layer", "fog", "environment/atmosphere/fog/fog_light_layer", 256, 128, false, 1 },
    { "fog_dense_layer", "fog", "environment/atmosphere/fog/fog_dense_layer", 256, 128, false, 1 },
    { "fog_ground_drift", "fog", "environment/atmosphere/fog/fog_ground_drift", 192, 64, false, 1 },
    { "fog_distant_haze", "fog", "environment/atmosphere/fog/fog_distant_haze", 256, 128, false, 1 },

    // Category C: Smoke (3) - 6 frames each
    { "smoke_chimney_anim", "smoke", "environment/atmosphere/smoke/smoke_chimney_anim", 96, 32, true, 6 },
    { "smoke_campfire_anim", "smoke", "environment/atmosphere/smoke/smoke_campfire_anim", 144, 40, true, 6 },
    { "smoke_plume_anim", "smoke", "environment/atmosphere/smoke/smoke_plume_anim", 192, 48, true, 6 },

    // Category D: Airplane (1) - 4 frames
    { "airplane_recon_001", "airplanes", "environment/events/airplanes/airplane_recon_001", 256, 64, true, 4 }
};

int main() {
    std::cout << "=================================================================\n";
    std::cout << "WILDBOUND PHASE 11: ENVIRONMENTAL ANIMATION SYSTEM TEST SUITE\n";
    std::cout << "=================================================================\n";

    fs::path assetRoot = "assets";
    assert(fs::exists(assetRoot) && "Assets root folder must exist");

    // 1. Validate File Existence and JSON Metadata
    std::cout << "\n[TEST 1] Verifying 32 Phase 11 Asset Files and JSON Metadata...\n";
    int assetsChecked = 0;
    for (const auto& item : PHASE_11_ASSETS) {
        fs::path pngPath = assetRoot / (item.rel_path + ".png");
        assert(fs::exists(pngPath) && ("PNG file must exist: " + pngPath.string()).c_str());

        if (item.is_spritesheet) {
            fs::path ssJsonPath = assetRoot / (item.rel_path + "_spritesheet.json");
            fs::path animJsonPath = assetRoot / (item.rel_path + "_animations.json");
            assert(fs::exists(ssJsonPath) && ("Spritesheet JSON must exist: " + ssJsonPath.string()).c_str());
            assert(fs::exists(animJsonPath) && ("Animations JSON must exist: " + animJsonPath.string()).c_str());

            std::ifstream ssFile(ssJsonPath);
            json ssJson;
            ssFile >> ssJson;
            assert(ssJson.contains("frame_count") && "Spritesheet JSON must specify frame_count");
            assert(ssJson["frame_count"].get<int>() == item.expected_frames && "Frame count mismatch");

            std::ifstream animFile(animJsonPath);
            json animJson;
            animFile >> animJson;
            assert(animJson.contains("animations") && "Animations JSON must contain animations array");
        } else {
            fs::path metaJsonPath = assetRoot / (item.rel_path + ".json");
            assert(fs::exists(metaJsonPath) && ("Metadata JSON must exist: " + metaJsonPath.string()).c_str());

            std::ifstream metaFile(metaJsonPath);
            json metaJson;
            metaFile >> metaJson;
            assert(metaJson.contains("id") && "Metadata must have id");
            assert(metaJson.contains("dimensions") && "Metadata must have dimensions");
        }
        assetsChecked++;
    }
    std::cout << "  -> PASSED: Verified all " << assetsChecked << " artwork files and metadata JSONs.\n";

    // 2. Validate Asset Manifest Registration
    std::cout << "\n[TEST 2] Verifying Manifest Registration in asset_manifest.json...\n";
    fs::path manifestPath = assetRoot / "manifests" / "asset_manifest.json";
    assert(fs::exists(manifestPath) && "Asset manifest must exist");

    std::ifstream mf(manifestPath);
    json manifestJson;
    mf >> manifestJson;
    assert(manifestJson.contains("assets") && "Manifest must contain assets array");

    std::unordered_set<std::string> registeredIds;
    for (const auto& entry : manifestJson["assets"]) {
        if (entry.contains("id")) {
            registeredIds.insert(entry["id"].get<std::string>());
        }
    }

    // Check specific Phase 11 IDs
    assert(registeredIds.count("tex_water_pond_anim") > 0 && "Missing tex_water_pond_anim");
    assert(registeredIds.count("sheet_water_pond_anim") > 0 && "Missing sheet_water_pond_anim");
    assert(registeredIds.count("sheet_waterfall_large_anim") > 0 && "Missing sheet_waterfall_large_anim");
    assert(registeredIds.count("sheet_fire_medium_anim") > 0 && "Missing sheet_fire_medium_anim");
    assert(registeredIds.count("sheet_grass_tall_wind_anim") > 0 && "Missing sheet_grass_tall_wind_anim");
    assert(registeredIds.count("sheet_tree_oak_breeze_anim") > 0 && "Missing sheet_tree_oak_breeze_anim");
    assert(registeredIds.count("tex_cloud_medium") > 0 && "Missing tex_cloud_medium");
    assert(registeredIds.count("tex_fog_light_layer") > 0 && "Missing tex_fog_light_layer");
    assert(registeredIds.count("sheet_smoke_campfire_anim") > 0 && "Missing sheet_smoke_campfire_anim");
    assert(registeredIds.count("sheet_airplane_recon_001") > 0 && "Missing sheet_airplane_recon_001");
    std::cout << "  -> PASSED: Verified Phase 11 entries in manifest (Total manifest count: "
              << registeredIds.size() << " assets).\n";

    // 3. EnvironmentalAnimationSystem Logic and State Transitions
    std::cout << "\n[TEST 3] Testing EnvironmentalAnimationSystem Logic & State Engine...\n";
    EnvironmentalAnimationSystem envSys;

    // Toggle settings
    envSys.SetWaterEnabled(false);
    assert(!envSys.IsWaterEnabled() && "Water toggle failed");
    envSys.SetWaterEnabled(true);
    assert(envSys.IsWaterEnabled() && "Water toggle failed");

    envSys.SetFireEnabled(false);
    assert(!envSys.IsFireEnabled() && "Fire toggle failed");
    envSys.SetFireEnabled(true);
    assert(envSys.IsFireEnabled() && "Fire toggle failed");

    envSys.SetWindEnabled(false);
    assert(!envSys.IsWindEnabled() && "Wind toggle failed");
    envSys.SetWindEnabled(true);
    assert(envSys.IsWindEnabled() && "Wind toggle failed");

    envSys.SetGlobalPlaybackSpeed(1.5f);
    assert(std::fabs(envSys.GetGlobalPlaybackSpeed() - 1.5f) < 0.001f && "Global speed failed");

    // Ambient wildlife spawning
    Rectangle testBounds{ 100.0f, 100.0f, 400.0f, 400.0f };
    envSys.SpawnAmbientBird(Vector2{ 200.0f, 200.0f }, testBounds);
    envSys.SpawnAmbientButterfly(Vector2{ 250.0f, 250.0f }, "forest_butterfly", testBounds);
    envSys.SpawnAmbientButterfly(Vector2{ 280.0f, 280.0f }, "blue_butterfly", testBounds);
    assert(envSys.GetAmbientCreatureCount() == 3 && "Ambient creature count mismatch");

    envSys.ClearAmbientCreatures();
    assert(envSys.GetAmbientCreatureCount() == 0 && "Clear ambient creatures failed");

    // Scripted Airplane Flyover Event
    std::cout << "\n[TEST 4] Testing Airplane Flyover Event Lifecycle...\n";
    Vector2 start{ 0.0f, 0.0f };
    Vector2 end{ 100.0f, 0.0f };
    envSys.TriggerAirplaneFlyover(start, end, 50.0f); // 50 px/s speed, 100 px distance = 2.0s duration

    const auto& event = envSys.GetAirplaneEvent();
    assert(event.active && "Airplane event should be active immediately upon trigger");
    assert(std::fabs(event.currentPos.x - 0.0f) < 0.01f && "Start pos X mismatch");
    assert(std::fabs(event.speed - 50.0f) < 0.01f && "Speed mismatch");
    assert(event.altitude > 0.0f && "Airplane should have cruising altitude");

    std::cout << "  -> PASSED: Airplane event initialized cleanly.\n";

    std::cout << "\n=================================================================\n";
    std::cout << "ALL PHASE 11 TESTS PASSED SUCCESSFULLY! (4/4 test groups)\n";
    std::cout << "=================================================================\n";

    return 0;
}
