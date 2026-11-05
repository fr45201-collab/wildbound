#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "entities/ModularCharacter.hpp"
#include "graphics/animation/Direction.hpp"
#include "core/Logger.hpp"

using namespace wildbound::entities;
using namespace wildbound::graphics::animation;

void TestCategoryExclusivity() {
    std::cout << "[TEST] Running Category Exclusivity Test..." << std::endl;

    CharacterEquipment equip;
    assert(!equip.HasEquipped(CosmeticCategory::Shirt));
    assert(equip.GetEquipped(CosmeticCategory::Shirt) == nullptr);

    CosmeticItem tunic;
    tunic.id = "shirt_forest_tunic";
    tunic.name = "Forest Tunic";
    tunic.category = CosmeticCategory::Shirt;
    tunic.spritesheetId = "sheet_shirt_forest_tunic";

    assert(equip.Equip(tunic));
    assert(equip.HasEquipped(CosmeticCategory::Shirt));
    assert(equip.GetEquipped(CosmeticCategory::Shirt)->id == "shirt_forest_tunic");

    // Equip another shirt into the same category - should replace
    CosmeticItem vest;
    vest.id = "shirt_adventurer_vest";
    vest.name = "Adventurer Vest";
    vest.category = CosmeticCategory::Shirt;
    vest.spritesheetId = "sheet_shirt_adventurer_vest";

    assert(equip.Equip(vest));
    assert(equip.HasEquipped(CosmeticCategory::Shirt));
    assert(equip.GetEquipped(CosmeticCategory::Shirt)->id == "shirt_adventurer_vest");
    assert(equip.GetAllEquipped().size() == 1);

    // Unequip
    assert(equip.Unequip(CosmeticCategory::Shirt));
    assert(!equip.HasEquipped(CosmeticCategory::Shirt));
    assert(equip.GetAllEquipped().empty());

    std::cout << "  -> PASSED" << std::endl;
}

void TestOcclusionFlags() {
    std::cout << "[TEST] Running Occlusion Flags Test..." << std::endl;

    CharacterEquipment equip;

    CosmeticItem hood;
    hood.id = "hat_ranger_hood";
    hood.name = "Ranger Hood";
    hood.category = CosmeticCategory::Headwear;
    hood.spritesheetId = "sheet_hat_ranger_hood";
    equip.Equip(hood);

    assert(!equip.HasOcclusionFlag("hair_hide_all"));

    CosmeticItem helm;
    helm.id = "hat_iron_helm";
    helm.name = "Iron Helmet";
    helm.category = CosmeticCategory::Headwear;
    helm.spritesheetId = "sheet_hat_iron_helm";
    helm.occlusionFlags.push_back("hair_hide_all");
    equip.Equip(helm);

    assert(equip.HasOcclusionFlag("hair_hide_all"));
    assert(!equip.HasOcclusionFlag("robe_hide_pants"));

    std::cout << "  -> PASSED" << std::endl;
}

void TestDirectionalLayerSorting() {
    std::cout << "[TEST] Running Directional Layer Sorting & Occlusion Test..." << std::endl;

    ModularCharacter character;

    // Equip full set
    CosmeticItem hair;
    hair.id = "hair_short_brown";
    hair.category = CosmeticCategory::Hair;
    hair.spritesheetId = "sheet_hair_short_brown";
    character.GetEquipment().Equip(hair);

    CosmeticItem shirt;
    shirt.id = "shirt_forest_tunic";
    shirt.category = CosmeticCategory::Shirt;
    shirt.spritesheetId = "sheet_shirt_forest_tunic";
    character.GetEquipment().Equip(shirt);

    CosmeticItem face;
    face.id = "face_eyepatch";
    face.category = CosmeticCategory::Face;
    face.spritesheetId = "sheet_face_eyepatch";
    character.GetEquipment().Equip(face);

    CosmeticItem back;
    back.id = "back_traveler_backpack";
    back.category = CosmeticCategory::Back;
    back.spritesheetId = "sheet_back_traveler_backpack";
    character.GetEquipment().Equip(back);

    // Test Facing Down (South)
    auto southStack = character.BuildLayerStack(Direction::Down);
    assert(!southStack.empty());

    // Back accessory must be behind body (Z < 20)
    auto backIt = std::find_if(southStack.begin(), southStack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Back;
    });
    assert(backIt != southStack.end());
    assert(backIt->zIndex < 20); // Z=10

    // Face accessory must be visible and in front
    auto faceIt = std::find_if(southStack.begin(), southStack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Face;
    });
    assert(faceIt != southStack.end());
    assert(faceIt->visible == true);
    assert(faceIt->zIndex > 60);

    // Test Facing Up (North)
    auto northStack = character.BuildLayerStack(Direction::Up);
    auto northBackIt = std::find_if(northStack.begin(), northStack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Back;
    });
    assert(northBackIt != northStack.end());
    // Back accessory must be in front of torso (Z > 50)
    assert(northBackIt->zIndex > 50); // Z=85

    auto northFaceIt = std::find_if(northStack.begin(), northStack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Face;
    });
    assert(northFaceIt != northStack.end());
    // Face accessory must be hidden when facing North
    assert(northFaceIt->visible == false);

    // Test Helmet Occlusion: hair must be suppressed if hair_hide_all is active
    CosmeticItem helm;
    helm.id = "hat_iron_helm";
    helm.category = CosmeticCategory::Headwear;
    helm.spritesheetId = "sheet_hat_iron_helm";
    helm.occlusionFlags.push_back("hair_hide_all");
    character.GetEquipment().Equip(helm);

    auto occludedStack = character.BuildLayerStack(Direction::Down);
    auto hairIt = std::find_if(occludedStack.begin(), occludedStack.end(), [](const VisualLayer& l) {
        return l.category == CosmeticCategory::Hair;
    });
    assert(hairIt == occludedStack.end()); // Hair completely omitted from render stack

    std::cout << "  -> PASSED" << std::endl;
}

void TestJsonSerializationAndPersistence() {
    std::cout << "[TEST] Running JSON Serialization & Persistence Test..." << std::endl;

    ModularCharacter original;
    original.ApplyDefaultAppearance();

    nlohmann::json j = original.ToJson();
    assert(j.contains("base_body"));
    assert(j.contains("equipment"));
    assert(j["equipment"].contains("hair"));
    assert(j["equipment"].contains("shirt"));

    // Save to temp file
    std::string tempPath = "scratch_test_appearance.json";
    assert(original.SaveAppearance(tempPath));
    assert(std::filesystem::exists(tempPath));

    // Deserialization via FromJson
    ModularCharacter loaded;
    // Dummy asset manager not required for pure data loading
    loaded.FromJson(j, *(wildbound::resources::AssetManager*)nullptr);
    assert(loaded.GetEquipment().HasEquipped(CosmeticCategory::Hair));
    assert(loaded.GetEquipment().HasEquipped(CosmeticCategory::Shirt));
    assert(loaded.GetEquipment().GetEquipped(CosmeticCategory::Hair)->id == "hair_short_brown");
    assert(loaded.GetEquipment().GetEquipped(CosmeticCategory::Shirt)->id == "shirt_forest_tunic");

    // Clean up temp file
    std::filesystem::remove(tempPath);

    std::cout << "  -> PASSED" << std::endl;
}

int main() {
    wildbound::core::Logger::SetMinLogLevel(wildbound::core::LogLevel::Warning);

    std::cout << "========================================" << std::endl;
    std::cout << "  WILDBOUND MODULAR CHARACTER TEST SUITE " << std::endl;
    std::cout << "========================================" << std::endl;

    TestCategoryExclusivity();
    TestOcclusionFlags();
    TestDirectionalLayerSorting();
    TestJsonSerializationAndPersistence();

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL MODULAR CHARACTER TESTS PASSED!   " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

