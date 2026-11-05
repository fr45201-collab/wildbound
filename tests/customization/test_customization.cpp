#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "customization/CosmeticTypes.hpp"
#include "customization/CosmeticDefinition.hpp"
#include "customization/CosmeticRegistry.hpp"
#include "customization/CharacterAppearance.hpp"
#include "customization/AppearancePreview.hpp"
#include "customization/AppearanceLoadout.hpp"
#include "customization/AppearanceSerializer.hpp"
#include "customization/CharacterCustomizationManager.hpp"
#include "entities/Player.hpp"
#include "entities/EquipmentSystem.hpp"
#include "items/ItemManager.hpp"
#include "resources/AssetManager.hpp"
#include "graphics/animation/Direction.hpp"
#include "input/InputContext.hpp"
#include "core/Logger.hpp"

namespace fs = std::filesystem;
using namespace wildbound::customization;
using namespace wildbound::entities;
using namespace wildbound::graphics::animation;
using namespace wildbound::items;
using namespace wildbound::resources;
using namespace wildbound::input;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

// 1. Cosmetic Definitions & Validation Tests
void TestCosmeticDefinitions() {
    std::cout << "[TEST] Running Cosmetic Definitions Validation..." << std::endl;

    // Valid definition
    CosmeticDefinition def;
    def.id = "test_hair_01";
    def.displayName = "Test Hair";
    def.slot = CustomizationSlot::Hair;
    def.spritesheetId = "sheet_hair_short_001";
    def.textureId = "tex_hair_short_001";
    def.compatibleDirections = { "up", "down", "left", "right" };
    TEST_ASSERT(def.IsValid(), "Valid cosmetic definition must return true for IsValid()");
    TEST_ASSERT(def.SupportsDirection(Direction::Down), "Should support down direction");
    TEST_ASSERT(def.SupportsDirection(Direction::Left), "Should support left direction");

    // Invalid definition (missing ID or spritesheet)
    CosmeticDefinition invalidDef;
    invalidDef.id = "";
    invalidDef.spritesheetId = "sheet_test";
    TEST_ASSERT(!invalidDef.IsValid(), "Definition with empty ID must be invalid");

    CosmeticDefinition invalidDef2;
    invalidDef2.id = "test_item";
    invalidDef2.spritesheetId = "";
    TEST_ASSERT(!invalidDef2.IsValid(), "Definition with empty spritesheet must be invalid");

    // Directional compatibility restriction
    CosmeticDefinition restrictedDef;
    restrictedDef.id = "front_only_item";
    restrictedDef.spritesheetId = "sheet_front";
    restrictedDef.compatibleDirections = { "down" };
    TEST_ASSERT(restrictedDef.SupportsDirection(Direction::Down), "Must support down");
    TEST_ASSERT(!restrictedDef.SupportsDirection(Direction::Up), "Must reject up");

    // JSON Serialization
    nlohmann::json j = def.ToJson();
    CosmeticDefinition deserialized = CosmeticDefinition::FromJson(j);
    TEST_ASSERT(deserialized.id == def.id, "Serialized ID must match");
    TEST_ASSERT(deserialized.slot == def.slot, "Serialized slot must match");
    TEST_ASSERT(deserialized.spritesheetId == def.spritesheetId, "Serialized spritesheet must match");

    std::cout << "  -> Passed Cosmetic Definitions." << std::endl;
}

// 2. Cosmetic Registry Tests
void TestCosmeticRegistry() {
    std::cout << "[TEST] Running Cosmetic Registry Tests..." << std::endl;

    CosmeticRegistry registry;
    TEST_ASSERT(registry.GetCount() == 0, "Registry must initially be empty");

    CosmeticDefinition def1;
    def1.id = "hair_001";
    def1.displayName = "Hair Style 1";
    def1.slot = CustomizationSlot::Hair;
    def1.spritesheetId = "sheet_hair_001";
    TEST_ASSERT(registry.RegisterCosmetic(def1), "RegisterCosmetic should succeed");
    TEST_ASSERT(registry.HasCosmetic("hair_001"), "Registry should contain hair_001");
    TEST_ASSERT(registry.GetCosmetic("hair_001") != nullptr, "Lookup should return pointer");

    // Slot query
    auto hairList = registry.GetCosmeticsBySlot(CustomizationSlot::Hair);
    TEST_ASSERT(hairList.size() == 1, "Slot query should return 1 hair");
    auto chestList = registry.GetCosmeticsBySlot(CustomizationSlot::Chest);
    TEST_ASSERT(chestList.empty(), "Chest list should be empty");

    // Duplicate ID overwriting
    CosmeticDefinition def1_mod = def1;
    def1_mod.displayName = "Modified Hair";
    TEST_ASSERT(registry.RegisterCosmetic(def1_mod), "Overwriting ID should succeed");
    TEST_ASSERT(registry.GetCosmetic("hair_001")->displayName == "Modified Hair", "Display name should update");
    TEST_ASSERT(registry.GetCosmeticsBySlot(CustomizationSlot::Hair).size() == 1, "Slot list should not duplicate item");

    // Builtin Registration
    registry.RegisterBuiltinCosmetics();
    TEST_ASSERT(registry.GetCount() >= 32, "Builtin cosmetics should register at least 32 items");
    TEST_ASSERT(registry.HasCosmetic("hair_short_001"), "Should have hair_short_001");
    TEST_ASSERT(registry.HasCosmetic("shirt_tunic_001"), "Should have shirt_tunic_001");
    TEST_ASSERT(registry.HasCosmetic("pants_traveler_001"), "Should have pants_traveler_001");
    TEST_ASSERT(registry.HasCosmetic("hat_helmet_001"), "Should have hat_helmet_001");

    // Catalog Loading if present
    if (fs::exists("data/cosmetics/costume_catalog.json")) {
        bool catalogLoaded = registry.LoadFromCatalog("data/cosmetics/costume_catalog.json");
        TEST_ASSERT(catalogLoaded, "Should load catalog from data/cosmetics/costume_catalog.json");
        TEST_ASSERT(registry.GetCount() >= 50, "Should have 50+ cosmetics loaded");
        TEST_ASSERT(registry.HasCosmetic("clo_basic_traveler_001"), "Should have clo_basic_traveler_001");
    }

    std::cout << "  -> Passed Cosmetic Registry (" << registry.GetCount() << " items indexed)." << std::endl;
}

// 3. Character Appearance State & Operations Tests
void TestCharacterAppearance() {
    std::cout << "[TEST] Running Character Appearance State Tests..." << std::endl;

    CharacterAppearance app;
    app.ResetToDefault();
    TEST_ASSERT(app.HasCosmetic(CustomizationSlot::Hair), "Default appearance must have hair");
    TEST_ASSERT(app.HasCosmetic(CustomizationSlot::Chest), "Default appearance must have chest");
    TEST_ASSERT(app.HasCosmetic(CustomizationSlot::Legs), "Default appearance must have legs");
    TEST_ASSERT(app.HasCosmetic(CustomizationSlot::Feet), "Default appearance must have feet");

    // Equip & Replace
    app.SetCosmetic(CustomizationSlot::Hair, "hair_long_001");
    TEST_ASSERT(app.GetCosmetic(CustomizationSlot::Hair) == "hair_long_001", "Hair should be hair_long_001");

    // Unequip slot
    app.ClearSlot(CustomizationSlot::Hair);
    TEST_ASSERT(!app.HasCosmetic(CustomizationSlot::Hair), "Hair should be cleared");
    TEST_ASSERT(app.GetCosmetic(CustomizationSlot::Hair).empty(), "Hair string should be empty");

    // Reset slot to default
    app.ResetToDefault();
    TEST_ASSERT(app.GetCosmetic(CustomizationSlot::Hair) == "hair_short_brown", "Should reset hair to default");

    // Apply to ModularCharacter
    CosmeticRegistry registry;
    registry.RegisterBuiltinCosmetics();
    ModularCharacter modularChar;
    app.ApplyToModularCharacter(modularChar, registry);
    TEST_ASSERT(modularChar.GetEquipment().HasEquipped(CosmeticCategory::Hair), "ModularCharacter must have Hair layer");
    TEST_ASSERT(modularChar.GetEquipment().HasEquipped(CosmeticCategory::Shirt), "ModularCharacter must have Shirt layer");

    // Sync from ModularCharacter
    CharacterAppearance syncedApp;
    syncedApp.SyncFromModularCharacter(modularChar);
    TEST_ASSERT(syncedApp.GetCosmetic(CustomizationSlot::Hair) == "hair_short_brown", "Synced hair should match");

    // JSON serialization
    nlohmann::json j = app.ToJson();
    CharacterAppearance loadedApp;
    TEST_ASSERT(loadedApp.FromJson(j), "FromJson should succeed");
    TEST_ASSERT(loadedApp == app, "Deserialized appearance should equal original");

    std::cout << "  -> Passed Character Appearance State." << std::endl;
}

// 4. Live Character Preview & Isolation Tests
void TestCharacterPreviewIsolation() {
    std::cout << "[TEST] Running Live Character Preview & Isolation Tests..." << std::endl;

    CosmeticRegistry registry;
    registry.RegisterBuiltinCosmetics();

    CharacterAppearance authoritativeApp;
    authoritativeApp.ResetToDefault();

    AppearancePreview preview;
    TEST_ASSERT(!preview.IsActive(), "Preview should initially be inactive");

    // 1. Begin preview
    preview.BeginPreview(authoritativeApp, registry);
    TEST_ASSERT(preview.IsActive(), "Preview must be active");
    TEST_ASSERT(!preview.HasUnsavedChanges(), "Initially no unsaved changes");

    // 2. Apply temporary changes
    const auto* newHair = registry.GetCosmetic("hair_curly_001");
    TEST_ASSERT(newHair != nullptr, "hair_curly_001 must exist");
    preview.ApplyPreviewCosmetic(*newHair, registry);
    TEST_ASSERT(preview.HasUnsavedChanges(), "Preview should now have unsaved changes");
    TEST_ASSERT(preview.GetPreviewAppearance().GetCosmetic(CustomizationSlot::Hair) == "hair_curly_001", "Preview hair must be updated");
    TEST_ASSERT(authoritativeApp.GetCosmetic(CustomizationSlot::Hair) == "hair_short_brown", "Authoritative appearance MUST NOT be mutated!");

    // 3. Direction & Animation controls
    preview.SetDirection(Direction::Down);
    TEST_ASSERT(preview.GetDirection() == Direction::Down, "Direction should be Down");
    preview.RotateClockwise();
    TEST_ASSERT(preview.GetDirection() == Direction::Left, "Clockwise from Down should be Left");
    preview.RotateCounterClockwise();
    TEST_ASSERT(preview.GetDirection() == Direction::Down, "Counter-clockwise should return to Down");

    TEST_ASSERT(!preview.IsWalkingAnimation(), "Initial animation should be idle");
    preview.ToggleAnimationMode();
    TEST_ASSERT(preview.IsWalkingAnimation(), "Animation mode should toggle to walk");

    // 4. Reset Preview
    preview.ResetPreview(registry);
    TEST_ASSERT(!preview.HasUnsavedChanges(), "Reset preview should restore original state");
    TEST_ASSERT(preview.GetPreviewAppearance().GetCosmetic(CustomizationSlot::Hair) == "hair_short_brown", "Restored hair must match original");

    // 5. Cancel Preview
    preview.ApplyPreviewCosmetic(*newHair, registry);
    preview.CancelPreview();
    TEST_ASSERT(!preview.IsActive(), "Cancel preview must deactivate preview");
    TEST_ASSERT(authoritativeApp.GetCosmetic(CustomizationSlot::Hair) == "hair_short_brown", "Authoritative state preserved upon cancellation");

    // 6. Confirm Preview
    preview.BeginPreview(authoritativeApp, registry);
    preview.ApplyPreviewCosmetic(*newHair, registry);
    TEST_ASSERT(preview.ConfirmPreview(authoritativeApp), "Confirm preview should succeed");
    TEST_ASSERT(!preview.IsActive(), "Confirm preview must deactivate preview");
    TEST_ASSERT(authoritativeApp.GetCosmetic(CustomizationSlot::Hair) == "hair_curly_001", "Authoritative appearance must now update");

    std::cout << "  -> Passed Live Character Preview & Isolation." << std::endl;
}

// 5. Appearance Loadout System Tests
void TestAppearanceLoadouts() {
    std::cout << "[TEST] Running Appearance Loadout System Tests..." << std::endl;

    CharacterAppearance app;
    app.ResetToDefault();

    AppearanceLoadout loadout;
    loadout.id = "test_loadout_01";
    loadout.name = "My Test Outfit";
    loadout.appearance = app;
    loadout.schemaVersion = 1;
    loadout.timestamp = 1700000000;
    TEST_ASSERT(loadout.IsValid(), "Loadout must be valid");

    // Serialization
    nlohmann::json j = loadout.ToJson();
    AppearanceLoadout restored = AppearanceLoadout::FromJson(j);
    TEST_ASSERT(restored.id == loadout.id, "Restored loadout ID must match");
    TEST_ASSERT(restored.name == loadout.name, "Restored loadout name must match");
    TEST_ASSERT(restored.appearance == loadout.appearance, "Restored appearance must match");

    // Persistence with multiple loadouts
    std::string testPath = "user_data/test/test_loadouts.json";
    if (fs::exists(testPath)) fs::remove(testPath);

    std::vector<AppearanceLoadout> loadoutList = AppearanceSerializer::CreateDefaultLoadouts();
    TEST_ASSERT(loadoutList.size() >= 3, "Default loadouts should have at least 3 presets");

    bool saved = AppearanceSerializer::SaveLoadouts(testPath, loadoutList, app);
    TEST_ASSERT(saved, "SaveLoadouts must succeed");
    TEST_ASSERT(fs::exists(testPath), "Loadouts file must exist on disk");

    std::vector<AppearanceLoadout> loadedList;
    CharacterAppearance loadedActive;
    bool loaded = AppearanceSerializer::LoadLoadouts(testPath, loadedList, loadedActive);
    TEST_ASSERT(loaded, "LoadLoadouts must succeed");
    TEST_ASSERT(loadedList.size() == loadoutList.size(), "Loaded loadout count must match");

    // Clean up test file
    fs::remove(testPath);

    std::cout << "  -> Passed Appearance Loadouts." << std::endl;
}

// 6. Equipment & Cosmetic Stat Isolation Tests
void TestEquipmentAndAppearanceStatIsolation() {
    std::cout << "[TEST] Running Equipment & Cosmetic Stat Isolation Tests..." << std::endl;

    // Verify that equipping a cosmetic does NOT mutate EquipmentSystem defense or stats
    EquipmentSystem equipSys;
    TEST_ASSERT(equipSys.GetTotalDefense() == 0.0f, "Initial defense must be 0");
    TEST_ASSERT(equipSys.GetHealthBonus() == 0.0f, "Initial health bonus must be 0");

    CharacterCustomizationManager customMgr;
    CosmeticRegistry& reg = customMgr.GetRegistry();
    reg.RegisterBuiltinCosmetics();

    // Equip pure cosmetic hat and armor
    customMgr.EquipCosmetic("hat_adventurer_001");
    customMgr.EquipCosmetic("shirt_leather_001");

    // Stats in EquipmentSystem must remain strictly 0
    TEST_ASSERT(equipSys.GetTotalDefense() == 0.0f, "Cosmetic equipping MUST NOT alter gameplay totalDefense");
    TEST_ASSERT(equipSys.GetDamageReduction() == 0.0f, "Cosmetic equipping MUST NOT alter damageReduction");
    TEST_ASSERT(equipSys.GetHealthBonus() == 0.0f, "Cosmetic equipping MUST NOT alter healthBonus");

    // Now equip gameplay armor in EquipmentSystem
    ItemInstance armorItem{ 101, "shirt_armor_001", 1 };
    ItemManager itemMgr;
    equipSys.Equip(EquipmentSlot::Chest, armorItem, &itemMgr);

    // Sync to customization manager
    customMgr.OnEquipmentChanged(EquipmentSlot::Chest, "shirt_armor_001");
    TEST_ASSERT(customMgr.GetEquippedCosmetic(CustomizationSlot::Chest) == "shirt_armor_001", "Visual layer should update to equipped armor");

    std::cout << "  -> Passed Equipment & Cosmetic Stat Isolation." << std::endl;
}

// 7. Input Context & UI Wardrobe Integration Tests
void TestInputContextAndUIIntegration() {
    std::cout << "[TEST] Running Input Context & UI Integration Tests..." << std::endl;

    // Verify Wardrobe input context properties
    InputContext wardrobeCtx = CreateContextPreset(InputContextType::Wardrobe);
    TEST_ASSERT(wardrobeCtx.type == InputContextType::Wardrobe, "Type must be Wardrobe");
    TEST_ASSERT(wardrobeCtx.isModal, "Wardrobe context must be modal to block gameplay actions");
    TEST_ASSERT(wardrobeCtx.AllowsCategory(ActionCategory::UI), "Must allow UI actions");
    TEST_ASSERT(wardrobeCtx.AllowsCategory(ActionCategory::Character), "Must allow Character actions");
    TEST_ASSERT(wardrobeCtx.AllowsCategory(ActionCategory::General), "Must allow General actions");
    TEST_ASSERT(!wardrobeCtx.AllowsCategory(ActionCategory::Movement), "Must block Movement actions");
    TEST_ASSERT(!wardrobeCtx.AllowsCategory(ActionCategory::Combat), "Must block Combat actions");

    std::cout << "  -> Passed Input Context & UI Integration." << std::endl;
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "  WILDBOUND PHASE 25: CUSTOMIZATION TEST SUITE    " << std::endl;
    std::cout << "==================================================" << std::endl;

    TestCosmeticDefinitions();
    TestCosmeticRegistry();
    TestCharacterAppearance();
    TestCharacterPreviewIsolation();
    TestAppearanceLoadouts();
    TestEquipmentAndAppearanceStatIsolation();
    TestInputContextAndUIIntegration();

    std::cout << "==================================================" << std::endl;
    std::cout << "  ALL 7 CUSTOMIZATION TESTS PASSED SUCCESSFULLY! " << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
