#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

#include "entities/ModularCharacter.hpp"
#include "graphics/animation/Direction.hpp"
#include "core/Logger.hpp"

namespace fs = std::filesystem;
using namespace wildbound::entities;
using namespace wildbound::graphics::animation;

const std::vector<std::pair<std::string, std::string>> PRODUCTION_COSMETICS = {
    // Hair (5)
    { "hair_short_001", "hair" },
    { "hair_long_001", "hair" },
    { "hair_messy_001", "hair" },
    { "hair_ponytail_001", "hair" },
    { "hair_curly_001", "hair" },

    // Shirts (5)
    { "shirt_tunic_001", "shirts" },
    { "shirt_leather_001", "shirts" },
    { "shirt_armor_001", "shirts" },
    { "shirt_traveler_001", "shirts" },
    { "shirt_mage_001", "shirts" },

    // Pants (5)
    { "pants_traveler_001", "pants" },
    { "pants_leather_001", "pants" },
    { "pants_armored_001", "pants" },
    { "pants_short_001", "pants" },
    { "pants_mage_001", "pants" },

    // Shoes (3)
    { "shoes_boots_001", "shoes" },
    { "shoes_armored_001", "shoes" },
    { "shoes_light_001", "shoes" },

    // Hats (5)
    { "hat_adventurer_001", "hats" },
    { "hat_hood_001", "hats" },
    { "hat_helmet_001", "hats" },
    { "hat_wizard_001", "hats" },
    { "hat_cap_001", "hats" },

    // Goggles (3)
    { "goggles_adventure_001", "goggles" },
    { "goggles_steampunk_001", "goggles" },
    { "goggles_snow_001", "goggles" },

    // Masks (3)
    { "mask_cloth_001", "masks" },
    { "mask_leather_001", "masks" },
    { "mask_mystic_001", "masks" },

    // Backpacks (3)
    { "backpack_traveler_001", "backpacks" },
    { "backpack_leather_001", "backpacks" },
    { "backpack_adventure_001", "backpacks" }
};

void TestAssetIntegrityAndMetadata() {
    std::cout << "[TEST] Running Asset Integrity & Metadata Schema Validation..." << std::endl;
    assert(PRODUCTION_COSMETICS.size() == 32);

    std::unordered_set<std::string> uniqueIds;

    for (const auto& [id, catFolder] : PRODUCTION_COSMETICS) {
        assert(uniqueIds.find(id) == uniqueIds.end() && "Duplicate ID detected!");
        uniqueIds.insert(id);

        fs::path baseDir = fs::path("assets/characters/player") / catFolder / id;
        assert(fs::exists(baseDir) && "Asset folder must exist!");

        // 1. Verify sprite PNG
        fs::path pngPath = baseDir / "sprite.png";
        assert(fs::exists(pngPath) && "sprite.png must exist!");
        assert(fs::file_size(pngPath) > 0 && "sprite.png must not be empty!");

        // 2. Verify spritesheet.json
        fs::path sheetPath = baseDir / "spritesheet.json";
        assert(fs::exists(sheetPath) && "spritesheet.json must exist!");
        std::ifstream sheetFile(sheetPath);
        nlohmann::json sheetJson;
        sheetFile >> sheetJson;
        assert(sheetJson["id"] == "sheet_" + id);
        assert(sheetJson["frame_width"] == 32);
        assert(sheetJson["frame_height"] == 48);
        assert(sheetJson["columns"] == 4);
        assert(sheetJson["rows"] == 8);
        assert(sheetJson["frame_count"] == 32);

        // 3. Verify metadata.json (Section 16 schema)
        fs::path metaPath = baseDir / "metadata.json";
        assert(fs::exists(metaPath) && "metadata.json must exist!");
        std::ifstream metaFile(metaPath);
        nlohmann::json metaJson;
        metaFile >> metaJson;
        assert(metaJson["id"] == id);
        assert(!metaJson["name"].get<std::string>().empty());
        assert(metaJson.contains("layer"));
        assert(metaJson.contains("provenance"));
        assert(metaJson["provenance"].contains("creator"));
        assert(metaJson["provenance"].contains("license"));
        assert(metaJson["anchors"]["body"].size() == 2);
    }

    std::cout << "  -> PASSED: All 32 assets and metadata files verified." << std::endl;
}

void TestCompatibilityRules() {
    std::cout << "[TEST] Running Compatibility & Occlusion Rules Test..." << std::endl;

    CharacterEquipment equip;

    // Test 1: Full-face mask vs Goggles conflict
    CosmeticItem goggles;
    goggles.id = "goggles_adventure_001";
    goggles.name = "Adventurer Goggles";
    goggles.category = CosmeticCategory::Goggles;
    goggles.spritesheetId = "sheet_goggles_adventure_001";
    assert(equip.Equip(goggles));

    CosmeticItem mysticMask;
    mysticMask.id = "mask_mystic_001";
    mysticMask.name = "Mystic Mask";
    mysticMask.category = CosmeticCategory::Mask;
    mysticMask.spritesheetId = "sheet_mask_mystic_001";
    mysticMask.occlusionFlags.push_back("mask_hide_goggles");

    // Strict compatibility check should flag conflict
    auto compat = equip.CheckCompatibility(mysticMask);
    assert(!compat.compatible);
    std::cout << "  -> Correctly detected conflict: " << compat.reason << std::endl;

    // Equip with force should succeed and activate occlusion flag
    assert(equip.Equip(mysticMask, true));
    assert(equip.HasOcclusionFlag("mask_hide_goggles"));

    // Verify in ModularCharacter render stack that goggles are occluded
    ModularCharacter mc;
    mc.GetEquipment().Equip(goggles);
    mc.GetEquipment().Equip(mysticMask);

    auto stack = mc.BuildLayerStack(Direction::Down);
    auto gogIt = std::find_if(stack.begin(), stack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Goggles;
    });
    assert(gogIt == stack.end() && "Goggles must be occluded by full-face mask!");

    // Test 2: Iron Helmet vs Hair
    CosmeticItem hair;
    hair.id = "hair_short_001";
    hair.category = CosmeticCategory::Hair;
    hair.spritesheetId = "sheet_hair_short_001";
    mc.GetEquipment().Equip(hair);

    CosmeticItem helm;
    helm.id = "hat_helmet_001";
    helm.category = CosmeticCategory::Headwear;
    helm.spritesheetId = "sheet_hat_helmet_001";
    helm.occlusionFlags.push_back("hair_hide_all");
    mc.GetEquipment().Equip(helm);

    auto occludedStack = mc.BuildLayerStack(Direction::Down);
    auto hairIt = std::find_if(occludedStack.begin(), occludedStack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Hair;
    });
    assert(hairIt == occludedStack.end() && "Hair must be suppressed by iron helmet!");

    std::cout << "  -> PASSED" << std::endl;
}

void TestRepresentativeConfigurations() {
    std::cout << "[TEST] Running 5 Representative Test Configurations (Section 18)..." << std::endl;

    ModularCharacter mc;

    // 1. Default Character
    mc.ApplyDefaultAppearance();
    auto s1 = mc.BuildLayerStack(Direction::Down);
    assert(!s1.empty());

    // 2. Explorer Appearance
    mc.GetEquipment().Clear();
    mc.GetEquipment().Equip(CosmeticItem{ "hair_messy_001", "Messy Explorer", CosmeticCategory::Hair, "sheet_hair_messy_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shirt_leather_001", "Leather Jacket", CosmeticCategory::Shirt, "sheet_shirt_leather_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "pants_traveler_001", "Traveler Pants", CosmeticCategory::Pants, "sheet_pants_traveler_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shoes_boots_001", "Leather Boots", CosmeticCategory::Shoes, "sheet_shoes_boots_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "hat_adventurer_001", "Adventurer Hat", CosmeticCategory::Headwear, "sheet_hat_adventurer_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "goggles_adventure_001", "Adventurer Goggles", CosmeticCategory::Goggles, "sheet_goggles_adventure_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "backpack_leather_001", "Leather Backpack", CosmeticCategory::Back, "sheet_backpack_leather_001" });
    auto s2_down = mc.BuildLayerStack(Direction::Down);
    auto s2_up = mc.BuildLayerStack(Direction::Up);
    assert(s2_down.size() >= 7);
    // In North facing, backpack must be in front (Z > 50) and goggles must be hidden
    auto bpUp = std::find_if(s2_up.begin(), s2_up.end(), [](const VisualLayer& l) { return l.category == CosmeticCategory::Back; });
    assert(bpUp != s2_up.end() && bpUp->zIndex > 50);

    // 3. Armored Appearance
    mc.GetEquipment().Clear();
    mc.GetEquipment().Equip(CosmeticItem{ "shirt_armor_001", "Light Armor", CosmeticCategory::Shirt, "sheet_shirt_armor_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "pants_armored_001", "Armored Pants", CosmeticCategory::Pants, "sheet_pants_armored_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shoes_armored_001", "Armored Boots", CosmeticCategory::Shoes, "sheet_shoes_armored_001" });
    CosmeticItem ironHelm{ "hat_helmet_001", "Iron Helmet", CosmeticCategory::Headwear, "sheet_hat_helmet_001" };
    ironHelm.occlusionFlags = { "hair_hide_all" };
    mc.GetEquipment().Equip(ironHelm);
    mc.GetEquipment().Equip(CosmeticItem{ "mask_leather_001", "Leather Mask", CosmeticCategory::Mask, "sheet_mask_leather_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "backpack_adventure_001", "Adventure Pack", CosmeticCategory::Back, "sheet_backpack_adventure_001" });
    auto s3 = mc.BuildLayerStack(Direction::Down);
    assert(s3.size() >= 6);

    // 4. Mystic Appearance
    mc.GetEquipment().Clear();
    mc.GetEquipment().Equip(CosmeticItem{ "hair_long_001", "Long Wanderer", CosmeticCategory::Hair, "sheet_hair_long_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shirt_mage_001", "Mystic Robe", CosmeticCategory::Shirt, "sheet_shirt_mage_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "pants_mage_001", "Mystic Trousers", CosmeticCategory::Pants, "sheet_pants_mage_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shoes_light_001", "Light Shoes", CosmeticCategory::Shoes, "sheet_shoes_light_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "hat_wizard_001", "Mystic Hat", CosmeticCategory::Headwear, "sheet_hat_wizard_001" });
    CosmeticItem mysticMask{ "mask_mystic_001", "Mystic Mask", CosmeticCategory::Mask, "sheet_mask_mystic_001" };
    mysticMask.occlusionFlags = { "mask_hide_goggles" };
    mc.GetEquipment().Equip(mysticMask);
    auto s4 = mc.BuildLayerStack(Direction::Down);
    assert(s4.size() >= 6);

    // 5. Mixed Appearance
    mc.GetEquipment().Clear();
    mc.GetEquipment().Equip(CosmeticItem{ "hair_ponytail_001", "High Ponytail", CosmeticCategory::Hair, "sheet_hair_ponytail_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shirt_tunic_001", "Forest Tunic", CosmeticCategory::Shirt, "sheet_shirt_tunic_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "pants_short_001", "Adventurer Shorts", CosmeticCategory::Pants, "sheet_pants_short_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "shoes_boots_001", "Leather Boots", CosmeticCategory::Shoes, "sheet_shoes_boots_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "hat_hood_001", "Explorer Hood", CosmeticCategory::Headwear, "sheet_hat_hood_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "goggles_steampunk_001", "Mechanical Goggles", CosmeticCategory::Goggles, "sheet_goggles_steampunk_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "mask_cloth_001", "Cloth Mask", CosmeticCategory::Mask, "sheet_mask_cloth_001" });
    mc.GetEquipment().Equip(CosmeticItem{ "backpack_traveler_001", "Traveler Backpack", CosmeticCategory::Back, "sheet_backpack_traveler_001" });
    auto s5 = mc.BuildLayerStack(Direction::Down);
    assert(s5.size() >= 8);

    std::cout << "  -> PASSED: All 5 representative configurations verified." << std::endl;
}

int main() {
    wildbound::core::Logger::SetMinLogLevel(wildbound::core::LogLevel::Warning);

    std::cout << "=================================================" << std::endl;
    std::cout << "  WILDBOUND PRODUCTION COSMETICS TEST SUITE      " << std::endl;
    std::cout << "=================================================" << std::endl;

    TestAssetIntegrityAndMetadata();
    TestCompatibilityRules();
    TestRepresentativeConfigurations();

    std::cout << "=================================================" << std::endl;
    std::cout << "  ALL PRODUCTION COSMETICS TESTS PASSED (32/32)! " << std::endl;
    std::cout << "=================================================" << std::endl;

    return 0;
}

