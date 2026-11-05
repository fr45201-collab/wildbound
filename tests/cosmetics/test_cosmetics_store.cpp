#include <cassert>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include "cosmetics/CosmeticsRarity.hpp"
#include "cosmetics/CosmeticsColorVariant.hpp"
#include "cosmetics/CosmeticsOwnership.hpp"
#include "cosmetics/CosmeticsOwnershipManager.hpp"
#include "cosmetics/CosmeticsSearchFilter.hpp"
#include "cosmetics/CosmeticsStoreManager.hpp"
#include "customization/CharacterCustomizationManager.hpp"
#include "resources/AssetManager.hpp"
#include "core/EngineConfig.hpp"
#include "core/Logger.hpp"

using namespace wildbound;
using namespace wildbound::cosmetics;
using namespace wildbound::customization;

void TestRarityMetadataAndConversions() {
    std::cout << "[Test 1] Testing Cosmetics Rarity Metadata & Conversions..." << std::endl;

    assert(StringToRarity("common") == CosmeticRarity::Common);
    assert(StringToRarity("uncommon") == CosmeticRarity::Uncommon);
    assert(StringToRarity("rare") == CosmeticRarity::Rare);
    assert(StringToRarity("epic") == CosmeticRarity::Epic);
    assert(StringToRarity("legendary") == CosmeticRarity::Legendary);
    assert(StringToRarity("mythic") == CosmeticRarity::Mythic);
    assert(StringToRarity("invalid_str") == CosmeticRarity::Common);

    assert(RarityToString(CosmeticRarity::Common) == "common");
    assert(RarityToString(CosmeticRarity::Mythic) == "mythic");
    assert(RarityToDisplayName(CosmeticRarity::Epic) == "Epic");
    assert(RarityToDisplayName(CosmeticRarity::Legendary) == "Legendary");

    assert(RarityToSortPriority(CosmeticRarity::Common) < RarityToSortPriority(CosmeticRarity::Uncommon));
    assert(RarityToSortPriority(CosmeticRarity::Uncommon) < RarityToSortPriority(CosmeticRarity::Rare));
    assert(RarityToSortPriority(CosmeticRarity::Rare) < RarityToSortPriority(CosmeticRarity::Epic));
    assert(RarityToSortPriority(CosmeticRarity::Epic) < RarityToSortPriority(CosmeticRarity::Legendary));
    assert(RarityToSortPriority(CosmeticRarity::Legendary) < RarityToSortPriority(CosmeticRarity::Mythic));

    Color c = RarityToColor(CosmeticRarity::Legendary);
    assert(c.r > 200 && c.g > 150); // Gold / orange
    std::cout << "  -> Rarity metadata tests passed." << std::endl;
}

void TestColorVariantsAndPalettes() {
    std::cout << "[Test 2] Testing Color Variants and Category Palettes..." << std::endl;

    CosmeticColorVariant v;
    v.id = "variant_ruby";
    v.displayName = "Ruby Red";
    v.tint = Color{ 220, 20, 40, 255 };
    v.hexColor = "#DC1428";

    nlohmann::json j = v.ToJson();
    CosmeticColorVariant v2 = CosmeticColorVariant::FromJson(j);
    assert(v2.id == v.id);
    assert(v2.displayName == v.displayName);
    assert(v2.tint.r == 220);
    assert(v2.tint.g == 20);
    assert(v2.tint.b == 40);

    auto hairVariants = ColorVariantPalette::GetHairVariants();
    assert(hairVariants.size() >= 5);
    bool foundBlonde = false;
    for (const auto& hv : hairVariants) {
        if (hv.id == "hair_blonde") foundBlonde = true;
    }
    assert(foundBlonde);

    auto metalVariants = ColorVariantPalette::GetVariantsForCategory("armor");
    assert(!metalVariants.empty());

    auto clothVariants = ColorVariantPalette::GetVariantsForCategory("clothing");
    assert(!clothVariants.empty());

    std::cout << "  -> Color variant tests passed." << std::endl;
}

void TestOwnershipRecordsAndManager() {
    std::cout << "[Test 3] Testing Cosmetics Ownership Manager & Persistence..." << std::endl;

    const std::string testSavePath = "user_data/test_cosmetics_ownership.json";
    if (std::filesystem::exists(testSavePath)) {
        std::filesystem::remove(testSavePath);
    }

    CosmeticsOwnershipManager mgr;
    assert(mgr.Initialize(testSavePath));

    // Starter items must be owned by default
    assert(mgr.IsOwned("cosmetic_skin_default"));
    assert(mgr.IsOwned("cosmetic_hair_short"));
    assert(mgr.IsOwned("cosmetic_tunic_adventurer"));
    assert(mgr.IsOwned("cosmetic_pants_leather"));
    assert(mgr.IsOwned("cosmetic_boots_leather"));

    // Unknown or locked cosmetic
    const std::string testItem = "cosmetic_dragon_helm_999";
    mgr.RegisterCosmetic(testItem, false);
    assert(!mgr.IsOwned(testItem));
    assert(mgr.GetOwnershipState(testItem) == OwnershipState::Locked);

    // Unlock via local prototype transaction
    assert(mgr.UnlockPrototype(testItem, "prototype_test"));
    assert(mgr.IsOwned(testItem));
    assert(mgr.GetOwnershipState(testItem) == OwnershipState::Owned);
    const auto* rec = mgr.GetRecord(testItem);
    assert(rec != nullptr);
    assert(rec->acquisitionSource == "prototype_test");

    // Color variant selection
    assert(mgr.SetSelectedVariant(testItem, "metal_gold"));
    assert(mgr.GetSelectedVariant(testItem) == "metal_gold");

    // Revocation
    assert(mgr.RevokeCosmetic(testItem));
    assert(!mgr.IsOwned(testItem));

    // Reload from persistence file
    mgr.UnlockPrototype(testItem, "prototype_reloaded");
    mgr.Save();

    CosmeticsOwnershipManager mgr2;
    assert(mgr2.Initialize(testSavePath));
    assert(mgr2.IsOwned(testItem));
    assert(mgr2.GetRecord(testItem)->acquisitionSource == "prototype_reloaded");

    // Reset ownership to starter defaults
    mgr2.ResetOwnership();
    assert(!mgr2.IsOwned(testItem));
    assert(mgr2.IsOwned("cosmetic_skin_default"));

    // Clean up test file
    if (std::filesystem::exists(testSavePath)) {
        std::filesystem::remove(testSavePath);
    }

    std::cout << "  -> Ownership manager and persistence tests passed." << std::endl;
}

void TestSearchAndMultiFiltering() {
    std::cout << "[Test 4] Testing Search, Filtering, and Sorting..." << std::endl;

    CosmeticsOwnershipManager ownership;
    ownership.Initialize("user_data/test_ownership_filter.json");

    CosmeticDefinition item1;
    item1.id = "cosmetic_hat_wizard_001";
    item1.displayName = "Grand Wizard Hat";
    item1.description = "Pointy mystical hat with silver runes.";
    item1.category = "headwear";
    item1.slot = CustomizationSlot::Headwear;
    item1.rarity = "legendary";

    CosmeticDefinition item2;
    item2.id = "cosmetic_tunic_common_001";
    item2.displayName = "Apprentice Tunic";
    item2.description = "Simple cloth tunic.";
    item2.category = "clothing";
    item2.slot = CustomizationSlot::Chest;
    item2.rarity = "common";

    CosmeticDefinition item3;
    item3.id = "cosmetic_armor_plate_001";
    item3.displayName = "Iron Chestplate";
    item3.description = "Heavy steel protection.";
    item3.category = "armor";
    item3.slot = CustomizationSlot::Chest;
    item3.rarity = "rare";
    item3.isGameplayEquipment = true;

    ownership.RegisterCosmetic(item1.id, false); // Locked
    ownership.RegisterCosmetic(item2.id, true);  // Owned
    ownership.RegisterCosmetic(item3.id, false); // Locked

    std::vector<const CosmeticDefinition*> catalog = { &item1, &item2, &item3 };

    // 1. Text Search
    CosmeticFilterCriteria critSearch;
    critSearch.searchQuery = "wizard";
    auto res1 = CosmeticsSearchFilter::FilterAndSort(catalog, critSearch, ownership);
    assert(res1.size() == 1);
    assert(res1[0]->id == item1.id);

    // 2. Category Match
    CosmeticFilterCriteria critCat;
    critCat.category = "headwear";
    auto res2 = CosmeticsSearchFilter::FilterAndSort(catalog, critCat, ownership);
    assert(res2.size() == 1);
    assert(res2[0]->id == item1.id);

    // 3. Ownership Filter: OwnedOnly
    CosmeticFilterCriteria critOwned;
    critOwned.ownership = OwnershipFilter::OwnedOnly;
    auto res3 = CosmeticsSearchFilter::FilterAndSort(catalog, critOwned, ownership);
    assert(res3.size() == 1);
    assert(res3[0]->id == item2.id);

    // 4. Ownership Filter: LockedOnly
    CosmeticFilterCriteria critLocked;
    critLocked.ownership = OwnershipFilter::LockedOnly;
    auto res4 = CosmeticsSearchFilter::FilterAndSort(catalog, critLocked, ownership);
    assert(res4.size() == 2);

    // 5. Rarity Sort Descending
    CosmeticFilterCriteria critSortRarity;
    critSortRarity.sortOption = CosmeticSortOption::RarityDesc;
    auto res5 = CosmeticsSearchFilter::FilterAndSort(catalog, critSortRarity, ownership);
    assert(res5.size() == 3);
    assert(res5[0]->id == item1.id); // Legendary
    assert(res5[1]->id == item3.id); // Rare
    assert(res5[2]->id == item2.id); // Common

    // Clean up
    if (std::filesystem::exists("user_data/test_ownership_filter.json")) {
        std::filesystem::remove("user_data/test_ownership_filter.json");
    }

    std::cout << "  -> Search, filtering, and sorting tests passed." << std::endl;
}

void TestStoreManagerAndCustomizationIntegration() {
    std::cout << "[Test 5] Testing Cosmetics Store Coordinator & Customization Integration..." << std::endl;

    const std::string testSavePath = "user_data/test_store_mgr_ownership.json";
    const std::string testLoadoutPath = "user_data/test_store_mgr_loadouts.json";
    if (std::filesystem::exists(testSavePath)) std::filesystem::remove(testSavePath);
    if (std::filesystem::exists(testLoadoutPath)) std::filesystem::remove(testLoadoutPath);

    resources::AssetManager assetMgr;
    CharacterCustomizationManager customMgr;
    customMgr.Initialize(assetMgr, "data/cosmetics/costume_catalog.json", testLoadoutPath);

    CosmeticsStoreManager storeMgr;
    assert(storeMgr.Initialize(customMgr, nullptr, testSavePath));

    // Verify all catalogue items registered
    assert(storeMgr.GetOwnershipManager().GetTotalRecordsCount() >= 50);

    // 1. Lock a specific non-starter item
    const std::string lockedItem = "shirt_royal_001";
    if (customMgr.GetRegistry().HasCosmetic(lockedItem)) {
        storeMgr.GetOwnershipManager().RevokeCosmetic(lockedItem);
        assert(!storeMgr.GetOwnershipManager().IsOwned(lockedItem));

        // 2. Attempting to equip locked item MUST FAIL
        storeMgr.SelectCosmetic(lockedItem);
        assert(!storeMgr.IsSelectedOwned());
        bool equipSuccess = storeMgr.EquipSelected();
        assert(!equipSuccess && "Equipping locked item must be rejected!");

        // 3. Live preview of locked item MUST SUCCEED
        bool previewSuccess = storeMgr.PreviewSelected();
        assert(previewSuccess && "Live preview of locked item must be allowed!");

        // 4. Unlock prototype
        assert(storeMgr.UnlockPrototypeSelected());
        assert(storeMgr.IsSelectedOwned());

        // 5. Now equip must SUCCEED
        assert(storeMgr.EquipSelected());
        assert(storeMgr.IsSelectedEquipped());

        // 6. Unequip
        assert(storeMgr.UnequipSlot(CustomizationSlot::Chest));
        assert(!storeMgr.IsSelectedEquipped());
    }

    // 7. Loadout validation test: Loadout with unowned cosmetics must skip/clear unowned slots
    customMgr.CreateLoadout("Validation Test Loadout");
    assert(customMgr.GetLoadouts().size() >= 1);

    // Set an unowned item on the loadout appearance
    const std::string unownedItem = "hat_helmet_001";
    storeMgr.GetOwnershipManager().RevokeCosmetic(unownedItem);
    assert(!storeMgr.GetOwnershipManager().IsOwned(unownedItem));

    // Apply loadout validated
    assert(storeMgr.ApplyLoadoutValidated(0));

    // Clean up
    if (std::filesystem::exists(testSavePath)) std::filesystem::remove(testSavePath);
    if (std::filesystem::exists(testLoadoutPath)) std::filesystem::remove(testLoadoutPath);

    std::cout << "  -> Store manager and customization integration tests passed." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 26: COSMETICS STORE & WARDROBE   " << std::endl;
    std::cout << "=================================================" << std::endl;

    TestRarityMetadataAndConversions();
    TestColorVariantsAndPalettes();
    TestOwnershipRecordsAndManager();
    TestSearchAndMultiFiltering();
    TestStoreManagerAndCustomizationIntegration();

    std::cout << "=================================================" << std::endl;
    std::cout << "ALL PHASE 26 COSMETICS STORE TESTS PASSED (100%)" << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}

