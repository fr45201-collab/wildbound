#include "../../src/items/ItemData.hpp"
#include "../../src/items/ItemManager.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <vector>

using wildbound::core::Logger;
using namespace wildbound::items;
using namespace wildbound::resources;
namespace fs = std::filesystem;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound Item & Collectible Asset Collection Tests (Phase 16) ===" << std::endl;

    // Headless raylib context
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundItemTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");

    ItemManager mgr;
    bool ok = mgr.Initialize("data/items/item_catalog.json");
    TEST_ASSERT(ok, "ItemManager::Initialize should succeed");

    // ========================================================================
    // Test 1: Item Catalog Loading & Metadata Integrity (40 Items)
    // ========================================================================
    {
        std::cout << "[TEST] 1. Item catalog loading & count verification..." << std::endl;
        TEST_ASSERT(mgr.GetItemCount() == 40, "Must contain exactly 40 item assets");

        const std::vector<std::string> expectedCollectibles = {
            "col_gold_coin_001",
            "col_silver_coin_001",
            "col_ruby_001",
            "col_sapphire_001",
            "col_emerald_001",
            "col_ancient_crystal_001",
            "col_ancient_relic_001",
            "col_golden_emblem_001"
        };
        for (const auto& id : expectedCollectibles) {
            TEST_ASSERT(mgr.HasItem(id), ("Catalog must contain collectible: " + id).c_str());
            const ItemConfig* item = mgr.GetItem(id);
            TEST_ASSERT(item != nullptr, "ItemConfig must not be null");
            TEST_ASSERT(item->category == ItemCategory::Collectible, "Category must be Collectible");
            TEST_ASSERT(!item->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(!item->subcategory.empty(), "Subcategory must not be empty");
            TEST_ASSERT(!item->description.empty(), "Description must not be empty");
            TEST_ASSERT(!item->rarityTier.empty(), "Rarity tier must not be empty");
            TEST_ASSERT(item->IsValid(), "ItemConfig::IsValid must return true");
        }

        const std::vector<std::string> expectedConsumables = {
            "con_health_potion_small_001",
            "con_health_potion_large_001",
            "con_mana_potion_001",
            "con_stamina_potion_001",
            "con_antidote_001",
            "con_healing_herb_001",
            "con_bread_001",
            "con_apple_001",
            "con_cooked_meat_001",
            "con_water_flask_001"
        };
        for (const auto& id : expectedConsumables) {
            TEST_ASSERT(mgr.HasItem(id), ("Catalog must contain consumable: " + id).c_str());
            const ItemConfig* item = mgr.GetItem(id);
            TEST_ASSERT(item != nullptr, "ItemConfig must not be null");
            TEST_ASSERT(item->category == ItemCategory::Consumable, "Category must be Consumable");
            TEST_ASSERT(!item->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(item->IsValid(), "ItemConfig::IsValid must return true");
        }

        const std::vector<std::string> expectedMaterials = {
            "mat_wood_001",
            "mat_stone_001",
            "mat_iron_ore_001",
            "mat_gold_ore_001",
            "mat_leather_001",
            "mat_cloth_001",
            "mat_iron_ingot_001",
            "mat_gold_ingot_001"
        };
        for (const auto& id : expectedMaterials) {
            TEST_ASSERT(mgr.HasItem(id), ("Catalog must contain material: " + id).c_str());
            const ItemConfig* item = mgr.GetItem(id);
            TEST_ASSERT(item != nullptr, "ItemConfig must not be null");
            TEST_ASSERT(item->category == ItemCategory::CraftingMaterial, "Category must be CraftingMaterial");
            TEST_ASSERT(!item->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(item->IsValid(), "ItemConfig::IsValid must return true");
        }

        const std::vector<std::string> expectedResources = {
            "res_red_mushroom_001",
            "res_blue_mushroom_001",
            "res_healing_flower_001",
            "res_forest_herb_001",
            "res_wild_berries_001",
            "res_desert_cactus_fruit_001",
            "res_frozen_herb_001"
        };
        for (const auto& id : expectedResources) {
            TEST_ASSERT(mgr.HasItem(id), ("Catalog must contain resource: " + id).c_str());
            const ItemConfig* item = mgr.GetItem(id);
            TEST_ASSERT(item != nullptr, "ItemConfig must not be null");
            TEST_ASSERT(item->category == ItemCategory::NaturalResource, "Category must be NaturalResource");
            TEST_ASSERT(!item->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(!item->biomeTags.empty(), "Resource should have associated biome tags");
            TEST_ASSERT(item->IsValid(), "ItemConfig::IsValid must return true");
        }

        const std::vector<std::string> expectedSpecial = {
            "spc_ancient_key_001",
            "spc_rusty_key_001",
            "spc_ancient_scroll_001",
            "spc_treasure_map_001",
            "spc_rune_stone_001",
            "spc_ancient_amulet_001",
            "spc_mysterious_artifact_001"
        };
        for (const auto& id : expectedSpecial) {
            TEST_ASSERT(mgr.HasItem(id), ("Catalog must contain special item: " + id).c_str());
            const ItemConfig* item = mgr.GetItem(id);
            TEST_ASSERT(item != nullptr, "ItemConfig must not be null");
            TEST_ASSERT(item->category == ItemCategory::SpecialItem, "Category must be SpecialItem");
            TEST_ASSERT(!item->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(item->IsValid(), "ItemConfig::IsValid must return true");
        }

        std::cout << "  -> All 40 items loaded and verified with complete metadata. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 2: Category Filtering & Distribution
    // ========================================================================
    {
        std::cout << "[TEST] 2. Category filtering & distribution..." << std::endl;
        auto collectibles = mgr.GetItemsByCategory(ItemCategory::Collectible);
        TEST_ASSERT(collectibles.size() == 8, "Must retrieve exactly 8 collectibles");

        auto consumables = mgr.GetItemsByCategory(ItemCategory::Consumable);
        TEST_ASSERT(consumables.size() == 10, "Must retrieve exactly 10 consumables");

        auto materials = mgr.GetItemsByCategory(ItemCategory::CraftingMaterial);
        TEST_ASSERT(materials.size() == 8, "Must retrieve exactly 8 crafting materials");

        auto resources = mgr.GetItemsByCategory(ItemCategory::NaturalResource);
        TEST_ASSERT(resources.size() == 7, "Must retrieve exactly 7 natural resources");

        auto specialItems = mgr.GetItemsByCategory(ItemCategory::SpecialItem);
        TEST_ASSERT(specialItems.size() == 7, "Must retrieve exactly 7 special items");

        size_t total = collectibles.size() + consumables.size() + materials.size() + resources.size() + specialItems.size();
        TEST_ASSERT(total == 40, "Sum of category items must be 40");

        std::cout << "  -> Category query filtering validated (8/10/8/7/7). [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 3: AMS Asset Resolution & Disk File Existence (80 Assets)
    // ========================================================================
    {
        std::cout << "[TEST] 3. AMS asset registry resolution & disk verification..." << std::endl;
        const auto& reg = assetMgr.GetRegistry();
        const auto& allItems = mgr.GetAllItems();

        for (const auto& item : allItems) {
            // Inventory Icon (64x64 px)
            const auto* iconMeta = reg.GetMetadata(item.visuals.iconTextureId);
            TEST_ASSERT(iconMeta != nullptr, ("Icon asset missing in AMS: " + item.visuals.iconTextureId).c_str());
            TEST_ASSERT(iconMeta->width == 64 && iconMeta->height == 64, "Icon must be 64x64 px");
            TEST_ASSERT(fs::exists(fs::path("assets") / iconMeta->file), ("Icon file missing on disk: " + iconMeta->file).c_str());

            // World Representation (32x32 px)
            const auto* worldMeta = reg.GetMetadata(item.visuals.worldTextureId);
            TEST_ASSERT(worldMeta != nullptr, ("World sprite missing in AMS: " + item.visuals.worldTextureId).c_str());
            TEST_ASSERT(worldMeta->width == 32 && worldMeta->height == 32, "World sprite must be 32x32 px");
            TEST_ASSERT(fs::exists(fs::path("assets") / worldMeta->file), ("World sprite file missing on disk: " + worldMeta->file).c_str());
        }

        std::cout << "  -> All 80 item assets (40 icons + 40 world sprites) verified in AMS and present on disk. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 4: Biome Tag Queries
    // ========================================================================
    {
        std::cout << "[TEST] 4. Biome tag queries..." << std::endl;

        auto forestItems = mgr.GetItemsByBiome("forest");
        TEST_ASSERT(!forestItems.empty(), "Forest biome must have tagged items");
        bool hasMushroom = false;
        for (const auto* it : forestItems) {
            if (it->id == "res_red_mushroom_001") hasMushroom = true;
        }
        TEST_ASSERT(hasMushroom, "Forest items must include red mushroom");

        auto desertItems = mgr.GetItemsByBiome("desert");
        TEST_ASSERT(!desertItems.empty(), "Desert biome must have tagged items");
        bool hasCactus = false;
        for (const auto* it : desertItems) {
            if (it->id == "res_desert_cactus_fruit_001") hasCactus = true;
        }
        TEST_ASSERT(hasCactus, "Desert items must include desert cactus fruit");

        auto snowItems = mgr.GetItemsByBiome("snow");
        TEST_ASSERT(!snowItems.empty(), "Snow biome must have tagged items");
        bool hasFrozenHerb = false;
        for (const auto* it : snowItems) {
            if (it->id == "res_frozen_herb_001") hasFrozenHerb = true;
        }
        TEST_ASSERT(hasFrozenHerb, "Snow items must include frozen herb");

        std::cout << "  -> Biome tag querying validated across biomes. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 5: Full Validation via ItemManager::ValidateAssets
    // ========================================================================
    {
        std::cout << "[TEST] 5. ItemManager::ValidateAssets integration..." << std::endl;
        std::vector<std::string> errors;
        bool valid = mgr.ValidateAssets(assetMgr, errors);
        TEST_ASSERT(valid, "ItemManager::ValidateAssets should report 100% success");
        TEST_ASSERT(errors.empty(), "Errors list must be completely empty");
        std::cout << "  -> Comprehensive automated item asset validation passed with 0 errors. [PASSED]" << std::endl;
    }

    CloseWindow();
    std::cout << "\n>>> ALL PHASE 16 ITEM & COLLECTIBLE ASSET TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}

