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

struct CreatureInfo {
    std::string id;
    std::string category; // "combat" or "ambient"
    std::string dir_path;
    std::string spritesheet_id;
    int frame_w;
    int frame_h;
    int cols;
    int rows;
    int total_frames;
    bool is_flying;
    bool has_gameplay_config;
};

const std::vector<CreatureInfo> CREATURES = {
    // Category A: Hostile / Combat-Capable
    { "goblin", "combat", "assets/characters/enemies/goblin", "goblin_spritesheet", 32, 48, 4, 8, 32, false, true },
    { "skeleton", "combat", "assets/characters/enemies/skeleton", "skeleton_spritesheet", 32, 48, 4, 8, 32, false, true },
    { "wolf", "combat", "assets/characters/enemies/wolf", "wolf_spritesheet", 48, 40, 4, 8, 32, false, true },
    { "wild_boar", "combat", "assets/characters/enemies/wild_boar", "wild_boar_spritesheet", 48, 40, 4, 8, 32, false, true },
    { "bat", "combat", "assets/characters/enemies/bat", "bat_spritesheet", 32, 32, 4, 8, 32, true, true },

    // Category B: Ambient Wildlife
    { "small_bird", "ambient", "assets/characters/wildlife/birds/small_bird", "small_bird_spritesheet", 24, 24, 4, 4, 16, true, false },
    { "forest_butterfly", "ambient", "assets/characters/wildlife/butterflies/forest_butterfly", "forest_butterfly_spritesheet", 16, 16, 4, 2, 8, true, false },
    { "blue_butterfly", "ambient", "assets/characters/wildlife/butterflies/blue_butterfly", "blue_butterfly_spritesheet", 16, 16, 4, 2, 8, true, false },
    { "golden_butterfly", "ambient", "assets/characters/wildlife/butterflies/golden_butterfly", "golden_butterfly_spritesheet", 16, 16, 4, 2, 8, true, false }
};

void TestCreatureFileIntegrity() {
    std::cout << "[TEST] Running Creature Asset File Integrity Validation..." << std::endl;

    for (const auto& c : CREATURES) {
        fs::path dir(c.dir_path);
        assert(fs::exists(dir) && fs::is_directory(dir));

        // Spritesheet PNG & JSON
        fs::path png_file = dir / (c.id + "_spritesheet.png");
        fs::path json_file = dir / (c.id + "_spritesheet.json");
        fs::path anim_file = dir / (c.id + "_animations.json");
        fs::path meta_file = dir / "creature_metadata.json";

        assert(fs::exists(png_file) && "Missing creature PNG file");
        assert(fs::file_size(png_file) > 0 && "PNG file is empty");

        assert(fs::exists(json_file) && "Missing creature spritesheet JSON file");
        assert(fs::file_size(json_file) > 0 && "Spritesheet JSON file is empty");

        assert(fs::exists(anim_file) && "Missing creature animation JSON file");
        assert(fs::file_size(anim_file) > 0 && "Animation JSON file is empty");

        assert(fs::exists(meta_file) && "Missing creature metadata JSON file");
        assert(fs::file_size(meta_file) > 0 && "Creature metadata JSON file is empty");

        // Gameplay config check
        if (c.has_gameplay_config) {
            fs::path config_file = fs::path("data/characters/enemies") / (c.id + ".json");
            assert(fs::exists(config_file) && "Missing creature gameplay config file in data/characters/enemies");
            assert(fs::file_size(config_file) > 0 && "Gameplay config file is empty");
        }
    }

    std::cout << "  -> File integrity verified for all " << CREATURES.size() << " creatures." << std::endl;
}

void TestSpritesheetMetadata() {
    std::cout << "[TEST] Running Spritesheet JSON Specification Validation..." << std::endl;

    for (const auto& c : CREATURES) {
        fs::path json_file = fs::path(c.dir_path) / (c.id + "_spritesheet.json");
        std::ifstream file(json_file);
        assert(file.is_open());

        json data;
        file >> data;

        assert(data.contains("id") && data["id"] == c.spritesheet_id);
        assert(data.contains("frame_width") && data["frame_width"] == c.frame_w);
        assert(data.contains("frame_height") && data["frame_height"] == c.frame_h);
        assert(data.contains("columns") && data["columns"] == c.cols);
        assert(data.contains("rows") && data["rows"] == c.rows);
        assert(data.contains("frame_count") && data["frame_count"] == c.total_frames);
    }

    std::cout << "  -> Spritesheet specifications verified for all creatures." << std::endl;
}

void TestAnimationMetadata() {
    std::cout << "[TEST] Running Creature Animation Clip Validation..." << std::endl;

    for (const auto& c : CREATURES) {
        fs::path anim_file = fs::path(c.dir_path) / (c.id + "_animations.json");
        std::ifstream file(anim_file);
        assert(file.is_open());

        json data;
        file >> data;

        assert(data.contains("animations") && data["animations"].is_array());
        
        std::unordered_map<std::string, json> anims;
        for (const auto& clipObj : data["animations"]) {
            assert(clipObj.contains("animation"));
            anims[clipObj["animation"].get<std::string>()] = clipObj;
        }

        if (c.category == "combat") {
            // Directional ground or flying combat creatures
            std::vector<std::string> dirs = { "down", "up", "left", "right" };
            
            if (c.is_flying) {
                // Bat uses fly, move, attack, hurt, death
                for (const auto& dir : dirs) {
                    std::string fly_clip = c.id + "_fly_" + dir;
                    assert(anims.count(fly_clip) == 1 && "Missing directional fly clip");
                    assert(!anims[fly_clip]["frames"].empty());
                    assert(anims[fly_clip]["frame_duration"].get<float>() > 0.0f);
                }
            } else {
                // Ground quadrupeds/bipeds use idle, walk, run, attack, hurt, death
                for (const auto& dir : dirs) {
                    std::string prefix = (c.id == "wild_boar") ? "boar" : c.id;
                    std::string idle_clip = prefix + "_idle_" + dir;
                    std::string walk_clip = prefix + "_walk_" + dir;
                    std::string run_clip = prefix + "_run_" + dir;
                    std::string attack_clip = prefix + "_attack_" + dir;

                    assert(anims.count(idle_clip) == 1 && "Missing directional idle clip");
                    assert(anims.count(walk_clip) == 1 && "Missing directional walk clip");
                    assert(anims.count(run_clip) == 1 && "Missing directional run clip");
                    assert(anims.count(attack_clip) == 1 && "Missing directional attack clip");

                    assert(!anims[idle_clip]["frames"].empty());
                    assert(!anims[walk_clip]["frames"].empty());
                    assert(!anims[run_clip]["frames"].empty());
                    assert(!anims[attack_clip]["frames"].empty());

                    // Verify run duration is shorter/faster than walk duration
                    float walk_dur = anims[walk_clip]["frame_duration"].get<float>();
                    float run_dur = anims[run_clip]["frame_duration"].get<float>();
                    (void)walk_dur;
                    (void)run_dur;
                    assert(run_dur <= walk_dur && "Run duration should be faster than walk duration");
                }
            }

            // Hurt & death clips
            std::string prefix = (c.id == "wild_boar") ? "boar" : c.id;
            assert((anims.count(prefix + "_hurt_down") == 1 || anims.count(prefix + "_hurt") == 1) && "Missing hurt clip");
            assert((anims.count(prefix + "_death_down") == 1 || anims.count(prefix + "_death") == 1) && "Missing death clip");
        } else {
            // Ambient wildlife: bird & butterflies
            if (c.id == "small_bird") {
                assert(anims.count("bird_idle") == 1 && !anims["bird_idle"]["frames"].empty());
                assert(anims.count("bird_fly") == 1 && !anims["bird_fly"]["frames"].empty());
                assert(anims.count("bird_takeoff") == 1 && !anims["bird_takeoff"]["frames"].empty());
                assert(anims.count("bird_landing") == 1 && !anims["bird_landing"]["frames"].empty());
            } else {
                // Butterflies
                std::string resting_clip = c.id + "_resting";
                std::string fly_clip = c.id + "_fly";
                assert(anims.count(resting_clip) == 1 && !anims[resting_clip]["frames"].empty());
                assert(anims.count(fly_clip) == 1 && !anims[fly_clip]["frames"].empty());
            }
        }
    }

    std::cout << "  -> Animation clips and timing validated for all creatures." << std::endl;
}

void TestCreatureMetadataSchema() {
    std::cout << "[TEST] Running Creature Metadata Schema Validation..." << std::endl;

    for (const auto& c : CREATURES) {
        fs::path meta_file = fs::path(c.dir_path) / "creature_metadata.json";
        std::ifstream file(meta_file);
        assert(file.is_open());

        json meta;
        file >> meta;

        // Required schema keys
        assert(meta.contains("id") && meta["id"].is_string());
        assert(meta.contains("name") && !meta["name"].get<std::string>().empty());
        assert(meta.contains("category"));
        if (c.category == "combat") {
            assert(meta["category"] == "hostile_creature");
        } else {
            assert(meta["category"] == "ambient_wildlife");
        }

        assert(meta.contains("spritesheet") && meta["spritesheet"] == c.spritesheet_id);

        assert(meta.contains("dimensions"));
        assert(meta["dimensions"]["frame_width"] == c.frame_w);
        assert(meta["dimensions"]["frame_height"] == c.frame_h);
        assert(meta["dimensions"]["columns"] == c.cols);
        assert(meta["dimensions"]["rows"] == c.rows);
        assert(meta["dimensions"]["frame_count"] == c.total_frames);

        assert(meta.contains("anchors") && meta["anchors"].contains("ground"));
        int ax = meta["anchors"]["ground"][0].get<int>();
        int ay = meta["anchors"]["ground"][1].get<int>();
        (void)ax;
        (void)ay;
        assert(ax >= 0 && ax <= c.frame_w);
        assert(ay >= 0 && ay <= c.frame_h);

        if (c.category == "combat") {
            assert(meta.contains("collision"));
            assert(meta["collision"]["width"].get<float>() > 0.0f);
            assert(meta["collision"]["height"].get<float>() > 0.0f);
        }

        assert(meta.contains("provenance"));
        assert(meta["provenance"].contains("license"));
    }

    std::cout << "  -> Creature metadata schema verified for all creatures." << std::endl;
}

void TestManifestEntries() {
    std::cout << "[TEST] Running Asset Manifest Creature Entry Validation..." << std::endl;

    fs::path manifest_path("assets/manifests/asset_manifest.json");
    assert(fs::exists(manifest_path));

    std::ifstream file(manifest_path);
    assert(file.is_open());

    json manifest;
    file >> manifest;
    assert(manifest.contains("assets") && manifest["assets"].is_array());

    std::unordered_set<std::string> registered_ids;
    for (const auto& item : manifest["assets"]) {
        if (item.contains("id")) {
            registered_ids.insert(item["id"].get<std::string>());
        }
    }

    // Check newly added creatures
    std::vector<std::string> expected_ids = {
        "tex_wolf", "sheet_wolf",
        "tex_wild_boar", "sheet_wild_boar",
        "tex_bat", "sheet_bat",
        "tex_small_bird", "sheet_small_bird",
        "tex_forest_butterfly", "sheet_forest_butterfly",
        "tex_blue_butterfly", "sheet_blue_butterfly",
        "tex_golden_butterfly", "sheet_golden_butterfly"
    };

    for (const auto& expected : expected_ids) {
        (void)expected;
        assert(registered_ids.count(expected) == 1 && "Missing asset ID in manifest");
    }

    std::cout << "  -> Manifest entries verified (All " << expected_ids.size() << " creature assets present)." << std::endl;
}

int main() {
    wildbound::core::Logger::Init();
    std::cout << "====================================================" << std::endl;
    std::cout << "   WILDBOUND PHASE 9 — CREATURE TEST SUITE" << std::endl;
    std::cout << "====================================================" << std::endl;

    TestCreatureFileIntegrity();
    TestSpritesheetMetadata();
    TestAnimationMetadata();
    TestCreatureMetadataSchema();
    TestManifestEntries();

    std::cout << "====================================================" << std::endl;
    std::cout << "   ALL CREATURE TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "====================================================" << std::endl;
    return 0;
}
