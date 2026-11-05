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
#include "resources/AssetManager.hpp"
#include "core/Logger.hpp"

namespace fs = std::filesystem;
using wildbound::core::Logger;
using namespace wildbound::entities;
using namespace wildbound::resources;
using namespace wildbound::graphics::animation;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

struct CosmeticTestDef {
    std::string id;
    std::string name;
    std::string category;
    std::string slot;
    std::string subDir;
    std::vector<std::string> occlusionFlags;
};

const std::vector<CosmeticTestDef> PHASE_17_COSMETICS = {
    // 1. Clothing (8)
    { "clo_basic_traveler_001", "Basic Traveler Shirt", "clothing", "shirt", "clothing", {} },
    { "clo_forest_tunic_001", "Forest Tunic", "clothing", "shirt", "clothing", {} },
    { "clo_desert_robe_001", "Desert Robe", "clothing", "shirt", "clothing", {} },
    { "clo_snow_jacket_001", "Snow Jacket", "clothing", "shirt", "clothing", {} },
    { "clo_leather_jacket_001", "Leather Jacket", "clothing", "shirt", "clothing", {} },
    { "clo_mystic_robe_001", "Mystic Robe", "clothing", "shirt", "clothing", {} },
    { "clo_royal_outfit_001", "Royal Outfit", "clothing", "shirt", "clothing", {} },
    { "clo_dark_adventurer_001", "Dark Adventurer Outfit", "clothing", "shirt", "clothing", {} },

    // 2. Armor (8)
    { "arm_leather_armor_001", "Leather Armor", "armor", "armor", "armor", {} },
    { "arm_reinforced_leather_001", "Reinforced Leather Armor", "armor", "armor", "armor", {} },
    { "arm_iron_armor_001", "Iron Armor", "armor", "armor", "armor", {} },
    { "arm_steel_armor_001", "Steel Armor", "armor", "armor", "armor", {} },
    { "arm_forest_guardian_001", "Forest Guardian Armor", "armor", "armor", "armor", {} },
    { "arm_desert_armor_001", "Desert Armor", "armor", "armor", "armor", {} },
    { "arm_frost_armor_001", "Frost Armor", "armor", "armor", "armor", {} },
    { "arm_ancient_rune_001", "Ancient Rune Armor", "armor", "armor", "armor", {} },

    // 3. Pants (6)
    { "clo_p_traveler_pants_001", "Traveler Pants", "pants", "pants", "pants", {} },
    { "clo_p_leather_pants_001", "Leather Pants", "pants", "pants", "pants", {} },
    { "clo_p_armored_pants_001", "Armored Pants", "pants", "pants", "pants", {} },
    { "clo_p_desert_trousers_001", "Desert Trousers", "pants", "pants", "pants", {} },
    { "clo_p_snow_trousers_001", "Snow Trousers", "pants", "pants", "pants", {} },
    { "clo_p_mystic_pants_001", "Mystic Pants", "pants", "pants", "pants", {} },

    // 4. Shoes and boots (5)
    { "sho_leather_boots_001", "Leather Boots", "footwear", "shoes", "footwear", {} },
    { "sho_iron_boots_001", "Iron Boots", "footwear", "shoes", "footwear", {} },
    { "sho_traveler_shoes_001", "Traveler Shoes", "footwear", "shoes", "footwear", {} },
    { "sho_snow_boots_001", "Snow Boots", "footwear", "shoes", "footwear", {} },
    { "sho_desert_sandals_001", "Desert Sandals", "footwear", "shoes", "footwear", {} },

    // 5. Headwear (6)
    { "hat_adventurer_hat_001", "Adventurer Hat", "headwear", "headwear", "headwear", {} },
    { "hat_explorer_hood_001", "Explorer Hood", "headwear", "headwear", "headwear", {} },
    { "hat_iron_helmet_001", "Iron Helmet", "headwear", "headwear", "headwear", { "hair_hide_all" } },
    { "hat_steel_helmet_001", "Steel Helmet", "headwear", "headwear", "headwear", { "hair_hide_all" } },
    { "hat_mystic_hat_001", "Mystic Hat", "headwear", "headwear", "headwear", {} },
    { "hat_forest_ranger_hood_001", "Forest Ranger Hood", "headwear", "headwear", "headwear", {} },

    // 6. Face accessories (5)
    { "fac_cloth_mask_001", "Cloth Mask", "face_accessories", "mask", "face_accessories", {} },
    { "fac_leather_mask_001", "Leather Mask", "face_accessories", "mask", "face_accessories", {} },
    { "fac_mystic_mask_001", "Mystic Mask", "face_accessories", "mask", "face_accessories", { "mask_hide_goggles" } },
    { "fac_adventurer_goggles_001", "Adventurer Goggles", "face_accessories", "goggles", "face_accessories", {} },
    { "fac_mechanical_goggles_001", "Mechanical Goggles", "face_accessories", "goggles", "face_accessories", {} },

    // 7. Backpacks (5)
    { "bag_small_traveler_001", "Small Traveler Backpack", "backpacks", "backpack", "backpacks", {} },
    { "bag_leather_backpack_001", "Leather Backpack", "backpacks", "backpack", "backpacks", {} },
    { "bag_adventurer_backpack_001", "Adventurer Backpack", "backpacks", "backpack", "backpacks", {} },
    { "bag_explorer_backpack_001", "Explorer Backpack", "backpacks", "backpack", "backpacks", {} },
    { "bag_military_backpack_001", "Military-Style Backpack", "backpacks", "backpack", "backpacks", {} },

    // 8. Character variations (7)
    { "var_skin_pale_001", "Pale Skin Appearance", "character_variations", "skin", "character_variations", {} },
    { "var_skin_dark_001", "Umber Skin Appearance", "character_variations", "skin", "character_variations", {} },
    { "var_skin_olive_001", "Olive Skin Appearance", "character_variations", "skin", "character_variations", {} },
    { "var_hair_braided_red_001", "Braided Crimson Hair", "character_variations", "hair", "character_variations", {} },
    { "var_hair_spiky_black_001", "Spiky Obsidian Hair", "character_variations", "hair", "character_variations", {} },
    { "var_face_warpaint_001", "Azure Hunter Warpaint", "character_variations", "face", "character_variations", {} },
    { "var_face_eyepatch_001", "Veteran Leather Eyepatch", "character_variations", "face", "character_variations", {} }
};

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound Costumes & Armor Collection Tests (Phase 17) ===" << std::endl;

    // Headless raylib context
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundCostumeTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");
    const auto& reg = assetMgr.GetRegistry();

    // ========================================================================
    // Test 1: Asset Count, Unique IDs & File Existence (50 Designs)
    // ========================================================================
    {
        std::cout << "[TEST] 1. Asset count, unique IDs & filesystem existence..." << std::endl;
        TEST_ASSERT(PHASE_17_COSMETICS.size() == 50, "Phase 17 must contain exactly 50 cosmetic items");

        std::unordered_set<std::string> uniqueIds;
        for (const auto& item : PHASE_17_COSMETICS) {
            TEST_ASSERT(uniqueIds.find(item.id) == uniqueIds.end(), ("Duplicate cosmetic ID: " + item.id).c_str());
            uniqueIds.insert(item.id);

            fs::path baseDir = fs::path("assets/cosmetics") / item.subDir / item.id;
            TEST_ASSERT(fs::exists(baseDir), ("Asset directory missing: " + baseDir.string()).c_str());
            TEST_ASSERT(fs::exists(baseDir / "sprite.png"), ("PNG missing: " + (baseDir / "sprite.png").string()).c_str());
            TEST_ASSERT(fs::exists(baseDir / "spritesheet.json"), ("JSON missing: " + (baseDir / "spritesheet.json").string()).c_str());
            TEST_ASSERT(fs::exists(baseDir / "metadata.json"), ("Metadata missing: " + (baseDir / "metadata.json").string()).c_str());

            // Check AMS registry registration
            std::string texKey = "tex_" + item.id;
            std::string sheetKey = "sheet_" + item.id;

            const auto* texMeta = reg.GetMetadata(texKey);
            TEST_ASSERT(texMeta != nullptr, ("Texture missing in AMS: " + texKey).c_str());
            TEST_ASSERT(texMeta->width == 128 && texMeta->height == 384, "Spritesheet texture must be 128x384 px");

            const auto* sheetMeta = reg.GetMetadata(sheetKey);
            TEST_ASSERT(sheetMeta != nullptr, ("Spritesheet missing in AMS: " + sheetKey).c_str());
            TEST_ASSERT(sheetMeta->width == 32 && sheetMeta->height == 48, "Spritesheet frame size must be 32x48 px");
        }
        std::cout << "  -> All 50 designs verified on disk and in AMS registry (100 assets). [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 2: Category Distribution
    // ========================================================================
    {
        std::cout << "[TEST] 2. Category distribution..." << std::endl;
        size_t clothingCount = 0, armorCount = 0, pantsCount = 0, shoesCount = 0;
        size_t headCount = 0, faceCount = 0, bagCount = 0, varCount = 0;

        for (const auto& item : PHASE_17_COSMETICS) {
            if (item.category == "clothing") clothingCount++;
            else if (item.category == "armor") armorCount++;
            else if (item.category == "pants") pantsCount++;
            else if (item.category == "footwear") shoesCount++;
            else if (item.category == "headwear") headCount++;
            else if (item.category == "face_accessories") faceCount++;
            else if (item.category == "backpacks") bagCount++;
            else if (item.category == "character_variations") varCount++;
        }

        TEST_ASSERT(clothingCount == 8, "Must contain 8 clothing designs");
        TEST_ASSERT(armorCount == 8, "Must contain 8 armor designs");
        TEST_ASSERT(pantsCount == 6, "Must contain 6 pants designs");
        TEST_ASSERT(shoesCount == 5, "Must contain 5 footwear designs");
        TEST_ASSERT(headCount == 6, "Must contain 6 headwear designs");
        TEST_ASSERT(faceCount == 5, "Must contain 5 face accessories");
        TEST_ASSERT(bagCount == 5, "Must contain 5 backpacks");
        TEST_ASSERT(varCount == 7, "Must contain 7 character variations");

        std::cout << "  -> 8 Clothing, 8 Armor, 6 Pants, 5 Shoes, 6 Headwear, 5 Face, 5 Backpacks, 7 Variations validated. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 3: Master Catalog Integrity (`costume_catalog.json`)
    // ========================================================================
    {
        std::cout << "[TEST] 3. Master costume catalog integrity..." << std::endl;
        std::string catalogPath = "data/cosmetics/costume_catalog.json";
        TEST_ASSERT(fs::exists(catalogPath), "Master costume catalog must exist");

        std::ifstream file(catalogPath);
        nlohmann::json catJson;
        file >> catJson;

        TEST_ASSERT(catJson.contains("total_cosmetics"), "Catalog must have total_cosmetics");
        TEST_ASSERT(catJson["total_cosmetics"].get<size_t>() == 50, "Total cosmetics must be 50");
        TEST_ASSERT(catJson["cosmetics"].size() == 50, "Cosmetics array must contain 50 entries");
        std::cout << "  -> Master catalog data verified. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 4: Modular Character Compositing & Independent Equipping
    // ========================================================================
    {
        std::cout << "[TEST] 4. Modular character compositing & independent equipping..." << std::endl;
        ModularCharacter character;
        auto& equip = character.GetEquipment();
        equip.Clear();

        // Equip shirt + armor simultaneously
        CosmeticItem shirt{ "clo_forest_tunic_001", "Forest Tunic", CosmeticCategory::Shirt, "sheet_clo_forest_tunic_001" };
        CosmeticItem armor{ "arm_forest_guardian_001", "Guardian Armor", CosmeticCategory::Armor, "sheet_arm_forest_guardian_001" };
        CosmeticItem pants{ "clo_p_traveler_pants_001", "Traveler Pants", CosmeticCategory::Pants, "sheet_clo_p_traveler_pants_001" };
        CosmeticItem shoes{ "sho_leather_boots_001", "Leather Boots", CosmeticCategory::Shoes, "sheet_sho_leather_boots_001" };
        CosmeticItem hat{ "hat_forest_ranger_hood_001", "Ranger Hood", CosmeticCategory::Headwear, "sheet_hat_forest_ranger_hood_001" };
        CosmeticItem warpaint{ "var_face_warpaint_001", "Azure Warpaint", CosmeticCategory::Face, "sheet_var_face_warpaint_001" };
        CosmeticItem bag{ "bag_explorer_backpack_001", "Explorer Pack", CosmeticCategory::Back, "sheet_bag_explorer_backpack_001" };

        TEST_ASSERT(equip.Equip(shirt), "Should equip shirt");
        TEST_ASSERT(equip.Equip(armor), "Should equip armor");
        TEST_ASSERT(equip.Equip(pants), "Should equip pants");
        TEST_ASSERT(equip.Equip(shoes), "Should equip shoes");
        TEST_ASSERT(equip.Equip(hat), "Should equip hat");
        TEST_ASSERT(equip.Equip(warpaint), "Should equip face warpaint");
        TEST_ASSERT(equip.Equip(bag), "Should equip backpack");

        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Shirt), "Must have Shirt equipped");
        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Armor), "Must have Armor equipped");
        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Pants), "Must have Pants equipped");
        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Shoes), "Must have Shoes equipped");
        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Headwear), "Must have Headwear equipped");
        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Face), "Must have Face equipped");
        TEST_ASSERT(equip.HasEquipped(CosmeticCategory::Back), "Must have Back equipped");

        // Swap out armor only
        CosmeticItem steelArmor{ "arm_steel_armor_001", "Steel Armor", CosmeticCategory::Armor, "sheet_arm_steel_armor_001" };
        TEST_ASSERT(equip.Equip(steelArmor), "Should swap armor");
        TEST_ASSERT(equip.GetEquipped(CosmeticCategory::Armor)->id == "arm_steel_armor_001", "Armor must now be steel armor");
        TEST_ASSERT(equip.GetEquipped(CosmeticCategory::Shirt)->id == "clo_forest_tunic_001", "Shirt must remain untouched");

        std::cout << "  -> Modular equipping and slot isolation verified. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 5: Directional Depth Sorting & Occlusion Matrix
    // ========================================================================
    {
        std::cout << "[TEST] 5. Directional depth sorting & occlusion matrix..." << std::endl;
        ModularCharacter character;
        auto& equip = character.GetEquipment();
        equip.Clear();

        equip.Equip(CosmeticItem{ "clo_desert_robe_001", "Desert Robe", CosmeticCategory::Shirt, "sheet_clo_desert_robe_001" });
        equip.Equip(CosmeticItem{ "arm_desert_armor_001", "Desert Armor", CosmeticCategory::Armor, "sheet_arm_desert_armor_001" });
        equip.Equip(CosmeticItem{ "bag_leather_backpack_001", "Leather Pack", CosmeticCategory::Back, "sheet_bag_leather_backpack_001" });
        equip.Equip(CosmeticItem{ "fac_adventurer_goggles_001", "Goggles", CosmeticCategory::Goggles, "sheet_fac_adventurer_goggles_001" });

        // Facing Down (South)
        auto stackSouth = character.BuildLayerStack(Direction::Down);
        int bagZSouth = -1, shirtZSouth = -1, armorZSouth = -1, gogglesZSouth = -1;
        bool gogglesVisibleSouth = false;

        for (const auto& layer : stackSouth) {
            if (layer.category == CosmeticCategory::Back) bagZSouth = layer.zIndex;
            if (layer.category == CosmeticCategory::Shirt) shirtZSouth = layer.zIndex;
            if (layer.category == CosmeticCategory::Armor) armorZSouth = layer.zIndex;
            if (layer.category == CosmeticCategory::Goggles) {
                gogglesZSouth = layer.zIndex;
                gogglesVisibleSouth = layer.visible;
            }
        }

        TEST_ASSERT(bagZSouth == 10, "Facing Down: backpack must be behind body (Z=10)");
        TEST_ASSERT(shirtZSouth == 50, "Facing Down: shirt must be at Z=50");
        TEST_ASSERT(armorZSouth == 52, "Facing Down: armor must be layered over shirt (Z=52 > Z=50)");
        TEST_ASSERT(gogglesVisibleSouth == true, "Facing Down: goggles must be visible");

        // Facing Up (North)
        auto stackNorth = character.BuildLayerStack(Direction::Up);
        int bagZNorth = -1;
        bool gogglesVisibleNorth = true;

        for (const auto& layer : stackNorth) {
            if (layer.category == CosmeticCategory::Back) bagZNorth = layer.zIndex;
            if (layer.category == CosmeticCategory::Goggles) gogglesVisibleNorth = layer.visible;
        }

        TEST_ASSERT(bagZNorth == 85, "Facing Up: backpack must be in foreground over torso (Z=85)");
        TEST_ASSERT(gogglesVisibleNorth == false, "Facing Up: face accessories must be hidden from behind");

        std::cout << "  -> Directional Z-order and front/back occlusion validated. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 6: Occlusion Flags (Helmets Hide Hair, Masks Hide Goggles)
    // ========================================================================
    {
        std::cout << "[TEST] 6. Occlusion flags (hair_hide_all & mask_hide_goggles)..." << std::endl;
        ModularCharacter character;
        auto& equip = character.GetEquipment();
        equip.Clear();

        // Equip hair + open hat
        equip.Equip(CosmeticItem{ "var_hair_braided_red_001", "Braided Red", CosmeticCategory::Hair, "sheet_var_hair_braided_red_001" });
        equip.Equip(CosmeticItem{ "hat_adventurer_hat_001", "Adventurer Hat", CosmeticCategory::Headwear, "sheet_hat_adventurer_hat_001" });

        auto stackWithHair = character.BuildLayerStack(Direction::Down);
        bool hasHairLayer = false;
        for (const auto& l : stackWithHair) {
            if (l.category == CosmeticCategory::Hair) hasHairLayer = true;
        }
        TEST_ASSERT(hasHairLayer, "Hair must be present when wearing open adventurer hat");

        // Equip full iron helmet with hair_hide_all
        CosmeticItem helm{ "hat_iron_helmet_001", "Iron Helmet", CosmeticCategory::Headwear, "sheet_hat_iron_helmet_001" };
        helm.occlusionFlags = { "hair_hide_all" };
        equip.Equip(helm);

        auto stackWithHelm = character.BuildLayerStack(Direction::Down);
        bool hasHairUnderHelm = false;
        for (const auto& l : stackWithHelm) {
            if (l.category == CosmeticCategory::Hair) hasHairUnderHelm = true;
        }
        TEST_ASSERT(!hasHairUnderHelm, "Hair must be completely hidden under full iron helmet");

        // Equip mystic mask with mask_hide_goggles
        equip.Equip(CosmeticItem{ "fac_adventurer_goggles_001", "Goggles", CosmeticCategory::Goggles, "sheet_fac_adventurer_goggles_001" });
        CosmeticItem mysticMask{ "fac_mystic_mask_001", "Mystic Mask", CosmeticCategory::Mask, "sheet_fac_mystic_mask_001" };
        mysticMask.occlusionFlags = { "mask_hide_goggles" };
        equip.Equip(mysticMask);

        auto stackWithMask = character.BuildLayerStack(Direction::Down);
        bool hasGogglesUnderMask = false;
        for (const auto& l : stackWithMask) {
            if (l.category == CosmeticCategory::Goggles) hasGogglesUnderMask = true;
        }
        TEST_ASSERT(!hasGogglesUnderMask, "Goggles must be suppressed under porcelain mystic mask");

        std::cout << "  -> Occlusion flags hair_hide_all and mask_hide_goggles verified. [PASSED]" << std::endl;
    }

    CloseWindow();
    std::cout << "\n>>> ALL PHASE 17 COSTUMES & ARMOR COLLECTION TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
