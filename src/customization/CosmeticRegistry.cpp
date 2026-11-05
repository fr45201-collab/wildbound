#include "CosmeticRegistry.hpp"
#include "../core/Logger.hpp"
#include "../resources/AssetRegistry.hpp"
#include <fstream>
#include <algorithm>

namespace wildbound::customization {

bool CosmeticRegistry::RegisterCosmetic(const CosmeticDefinition& def) {
    if (!def.IsValid()) {
        core::Logger::Warn("COSMETIC_REG", "Failed to register cosmetic: definition is invalid (missing ID or spritesheet).");
        return false;
    }

    if (m_definitions.find(def.id) != m_definitions.end()) {
        core::Logger::Warn("COSMETIC_REG", "Overwriting existing cosmetic ID: " + def.id);
        // Remove from slot index first
        auto& list = m_slotIndex[m_definitions[def.id].slot];
        list.erase(std::remove(list.begin(), list.end(), def.id), list.end());
    }

    m_definitions[def.id] = def;
    m_slotIndex[def.slot].push_back(def.id);
    return true;
}

bool CosmeticRegistry::UnregisterCosmetic(const std::string& id) {
    auto it = m_definitions.find(id);
    if (it == m_definitions.end()) return false;

    CustomizationSlot slot = it->second.slot;
    m_definitions.erase(it);

    auto& list = m_slotIndex[slot];
    list.erase(std::remove(list.begin(), list.end(), id), list.end());
    return true;
}

void CosmeticRegistry::Clear() {
    m_definitions.clear();
    m_slotIndex.clear();
}

const CosmeticDefinition* CosmeticRegistry::GetCosmetic(const std::string& id) const {
    auto it = m_definitions.find(id);
    if (it != m_definitions.end()) {
        return &it->second;
    }
    return nullptr;
}

bool CosmeticRegistry::HasCosmetic(const std::string& id) const {
    return m_definitions.find(id) != m_definitions.end();
}

std::vector<const CosmeticDefinition*> CosmeticRegistry::GetCosmeticsBySlot(CustomizationSlot slot) const {
    std::vector<const CosmeticDefinition*> results;
    auto it = m_slotIndex.find(slot);
    if (it != m_slotIndex.end()) {
        results.reserve(it->second.size());
        for (const auto& id : it->second) {
            auto defIt = m_definitions.find(id);
            if (defIt != m_definitions.end()) {
                results.push_back(&defIt->second);
            }
        }
    }
    return results;
}

std::vector<const CosmeticDefinition*> CosmeticRegistry::GetAllCosmetics() const {
    std::vector<const CosmeticDefinition*> results;
    results.reserve(m_definitions.size());
    for (const auto& [_, def] : m_definitions) {
        results.push_back(&def);
    }
    return results;
}

bool CosmeticRegistry::LoadFromCatalog(const std::string& catalogPath, const resources::AssetRegistry* assetReg) {
    std::ifstream file(catalogPath);
    if (!file.is_open()) {
        core::Logger::Warn("COSMETIC_REG", "Could not open costume catalog JSON: " + catalogPath);
        return false;
    }

    try {
        nlohmann::json root = nlohmann::json::parse(file);
        if (!root.contains("cosmetics") || !root["cosmetics"].is_array()) {
            core::Logger::Error("COSMETIC_REG", "Malformed catalog JSON: missing 'cosmetics' array in " + catalogPath);
            return false;
        }

        size_t loaded = 0;
        for (const auto& item : root["cosmetics"]) {
            CosmeticDefinition def = CosmeticDefinition::FromJson(item);
            if (def.IsValid()) {
                if (assetReg && !def.spritesheetId.empty()) {
                    // Optional validation against AMS
                    if (!assetReg->HasAsset(def.spritesheetId)) {
                        core::Logger::Warn("COSMETIC_REG", "Asset not found in manifest for cosmetic: " + def.id);
                    }
                }
                RegisterCosmetic(def);
                ++loaded;
            }
        }

        core::Logger::Info("COSMETIC_REG", "Loaded " + std::to_string(loaded) + " cosmetics from " + catalogPath);
        return loaded > 0;
    } catch (const std::exception& e) {
        core::Logger::Error("COSMETIC_REG", "JSON parse error in " + catalogPath + ": " + e.what());
        return false;
    }
}

void CosmeticRegistry::RegisterBuiltinCosmetics(const resources::AssetRegistry* assetReg) {
    (void)assetReg;

    // 1. Core Base Skins & Appearance (Phase 7 Defaults)
    {
        CosmeticDefinition skinDefault;
        skinDefault.id = "skin_default";
        skinDefault.displayName = "Default Skin";
        skinDefault.slot = CustomizationSlot::Skin;
        skinDefault.category = "skin";
        skinDefault.spritesheetId = "sheet_skin_default";
        skinDefault.textureId = "tex_skin_default";
        skinDefault.rarity = "common";
        RegisterCosmetic(skinDefault);

        CosmeticDefinition skinTan;
        skinTan.id = "skin_tan";
        skinTan.displayName = "Tan Skin";
        skinTan.slot = CustomizationSlot::Skin;
        skinTan.category = "skin";
        skinTan.spritesheetId = "sheet_skin_tan";
        skinTan.textureId = "tex_skin_tan";
        skinTan.rarity = "common";
        RegisterCosmetic(skinTan);

        CosmeticDefinition hairShortBrown;
        hairShortBrown.id = "hair_short_brown";
        hairShortBrown.displayName = "Short Brown Hair";
        hairShortBrown.slot = CustomizationSlot::Hair;
        hairShortBrown.category = "hair";
        hairShortBrown.spritesheetId = "sheet_hair_short_brown";
        hairShortBrown.textureId = "tex_hair_short_brown";
        RegisterCosmetic(hairShortBrown);

        CosmeticDefinition hairPonytailBlonde;
        hairPonytailBlonde.id = "hair_ponytail_blonde";
        hairPonytailBlonde.displayName = "Blonde Ponytail";
        hairPonytailBlonde.slot = CustomizationSlot::Hair;
        hairPonytailBlonde.category = "hair";
        hairPonytailBlonde.spritesheetId = "sheet_hair_ponytail_blonde";
        hairPonytailBlonde.textureId = "tex_hair_ponytail_blonde";
        RegisterCosmetic(hairPonytailBlonde);

        CosmeticDefinition shirtForest;
        shirtForest.id = "shirt_forest_tunic";
        shirtForest.displayName = "Forest Tunic";
        shirtForest.slot = CustomizationSlot::Chest;
        shirtForest.category = "shirt";
        shirtForest.spritesheetId = "sheet_shirt_forest_tunic";
        shirtForest.textureId = "tex_shirt_forest_tunic";
        RegisterCosmetic(shirtForest);

        CosmeticDefinition shirtAdventurer;
        shirtAdventurer.id = "shirt_adventurer_vest";
        shirtAdventurer.displayName = "Adventurer Vest";
        shirtAdventurer.slot = CustomizationSlot::Chest;
        shirtAdventurer.category = "shirt";
        shirtAdventurer.spritesheetId = "sheet_shirt_adventurer_vest";
        shirtAdventurer.textureId = "tex_shirt_adventurer_vest";
        RegisterCosmetic(shirtAdventurer);

        CosmeticDefinition pantsTraveler;
        pantsTraveler.id = "pants_traveler_trousers";
        pantsTraveler.displayName = "Traveler Trousers";
        pantsTraveler.slot = CustomizationSlot::Legs;
        pantsTraveler.category = "pants";
        pantsTraveler.spritesheetId = "sheet_pants_traveler_trousers";
        pantsTraveler.textureId = "tex_pants_traveler_trousers";
        RegisterCosmetic(pantsTraveler);

        CosmeticDefinition pantsScout;
        pantsScout.id = "pants_scout_shorts";
        pantsScout.displayName = "Scout Shorts";
        pantsScout.slot = CustomizationSlot::Legs;
        pantsScout.category = "pants";
        pantsScout.spritesheetId = "sheet_pants_scout_shorts";
        pantsScout.textureId = "tex_pants_scout_shorts";
        RegisterCosmetic(pantsScout);

        CosmeticDefinition shoesBoots;
        shoesBoots.id = "shoes_leather_boots";
        shoesBoots.displayName = "Leather Boots";
        shoesBoots.slot = CustomizationSlot::Feet;
        shoesBoots.category = "shoes";
        shoesBoots.spritesheetId = "sheet_shoes_leather_boots";
        shoesBoots.textureId = "tex_shoes_leather_boots";
        RegisterCosmetic(shoesBoots);

        CosmeticDefinition shoesIron;
        shoesIron.id = "shoes_iron_greaves";
        shoesIron.displayName = "Iron Greaves";
        shoesIron.slot = CustomizationSlot::Feet;
        shoesIron.category = "shoes";
        shoesIron.spritesheetId = "sheet_shoes_iron_greaves";
        shoesIron.textureId = "tex_shoes_iron_greaves";
        RegisterCosmetic(shoesIron);

        CosmeticDefinition hatRanger;
        hatRanger.id = "hat_ranger_hood";
        hatRanger.displayName = "Ranger Hood";
        hatRanger.slot = CustomizationSlot::Headwear;
        hatRanger.category = "headwear";
        hatRanger.spritesheetId = "sheet_hat_ranger_hood";
        hatRanger.textureId = "tex_hat_ranger_hood";
        hatRanger.occlusionFlags = { "hair_hide_top" };
        RegisterCosmetic(hatRanger);

        CosmeticDefinition hatIron;
        hatIron.id = "hat_iron_helm";
        hatIron.displayName = "Iron Helmet";
        hatIron.slot = CustomizationSlot::Helmet;
        hatIron.category = "headwear";
        hatIron.spritesheetId = "sheet_hat_iron_helm";
        hatIron.textureId = "tex_hat_iron_helm";
        hatIron.occlusionFlags = { "hair_hide_all" };
        RegisterCosmetic(hatIron);

        CosmeticDefinition faceEyepatch;
        faceEyepatch.id = "face_eyepatch";
        faceEyepatch.displayName = "Eyepatch";
        faceEyepatch.slot = CustomizationSlot::Face;
        faceEyepatch.category = "face";
        faceEyepatch.spritesheetId = "sheet_face_eyepatch";
        faceEyepatch.textureId = "tex_face_eyepatch";
        RegisterCosmetic(faceEyepatch);

        CosmeticDefinition faceGoggles;
        faceGoggles.id = "face_explorer_goggles";
        faceGoggles.displayName = "Explorer Goggles";
        faceGoggles.slot = CustomizationSlot::Face;
        faceGoggles.category = "goggles";
        faceGoggles.spritesheetId = "sheet_face_explorer_goggles";
        faceGoggles.textureId = "tex_face_explorer_goggles";
        RegisterCosmetic(faceGoggles);

        CosmeticDefinition backTraveler;
        backTraveler.id = "back_traveler_backpack";
        backTraveler.displayName = "Traveler Backpack";
        backTraveler.slot = CustomizationSlot::Back;
        backTraveler.category = "back";
        backTraveler.spritesheetId = "sheet_back_traveler_backpack";
        backTraveler.textureId = "tex_back_traveler_backpack";
        RegisterCosmetic(backTraveler);
    }

    // 2. Phase 8 Production Cosmetics (32 Items)
    // Hairstyles (5)
    struct ItemMeta { const char* id; const char* name; CustomizationSlot slot; const char* cat; const char* flag; };
    const ItemMeta p8Items[] = {
        { "hair_short_001", "Short Crop", CustomizationSlot::Hair, "hair", nullptr },
        { "hair_long_001", "Long Wanderer", CustomizationSlot::Hair, "hair", nullptr },
        { "hair_messy_001", "Messy Explorer", CustomizationSlot::Hair, "hair", nullptr },
        { "hair_ponytail_001", "High Ponytail", CustomizationSlot::Hair, "hair", nullptr },
        { "hair_curly_001", "Curly Locks", CustomizationSlot::Hair, "hair", nullptr },

        // Shirts (5)
        { "shirt_tunic_001", "Simple Tunic", CustomizationSlot::Chest, "clothing", nullptr },
        { "shirt_leather_001", "Leather Vest", CustomizationSlot::Chest, "clothing", nullptr },
        { "shirt_armor_001", "Light Armor", CustomizationSlot::Chest, "armor", nullptr },
        { "shirt_traveler_001", "Traveler Cloak", CustomizationSlot::Chest, "clothing", nullptr },
        { "shirt_mage_001", "Mystic Robe", CustomizationSlot::Chest, "clothing", nullptr },

        // Pants (5)
        { "pants_traveler_001", "Traveler Pants", CustomizationSlot::Legs, "pants", nullptr },
        { "pants_leather_001", "Leather Trousers", CustomizationSlot::Legs, "pants", nullptr },
        { "pants_armored_001", "Armored Greaves", CustomizationSlot::Legs, "pants", nullptr },
        { "pants_short_001", "Breeches", CustomizationSlot::Legs, "pants", nullptr },
        { "pants_mage_001", "Mystic Skirt", CustomizationSlot::Legs, "pants", nullptr },

        // Shoes (3)
        { "shoes_boots_001", "Heavy Boots", CustomizationSlot::Feet, "footwear", nullptr },
        { "shoes_armored_001", "Plated Sabatons", CustomizationSlot::Feet, "footwear", nullptr },
        { "shoes_light_001", "Light Sandals", CustomizationSlot::Feet, "footwear", nullptr },

        // Hats (5)
        { "hat_adventurer_001", "Adventurer Hat", CustomizationSlot::Headwear, "headwear", nullptr },
        { "hat_hood_001", "Cowl Hood", CustomizationSlot::Headwear, "headwear", "hair_hide_top" },
        { "hat_helmet_001", "Iron Greathelm", CustomizationSlot::Helmet, "headwear", "hair_hide_all" },
        { "hat_wizard_001", "Pointed Wizard Hat", CustomizationSlot::Headwear, "headwear", nullptr },
        { "hat_cap_001", "Wool Cap", CustomizationSlot::Headwear, "headwear", "hair_hide_top" },

        // Goggles (3)
        { "goggles_adventure_001", "Adventure Goggles", CustomizationSlot::Face, "face_accessories", nullptr },
        { "goggles_steampunk_001", "Brass Goggles", CustomizationSlot::Face, "face_accessories", nullptr },
        { "goggles_snow_001", "Glacier Visor", CustomizationSlot::Face, "face_accessories", nullptr },

        // Masks (3)
        { "mask_cloth_001", "Cloth Bandana", CustomizationSlot::Face, "face_accessories", nullptr },
        { "mask_leather_001", "Leather Half-Mask", CustomizationSlot::Face, "face_accessories", nullptr },
        { "mask_mystic_001", "Mystic Veil", CustomizationSlot::Face, "face_accessories", "mask_hide_goggles" },

        // Backpacks (3)
        { "backpack_traveler_001", "Rucksack", CustomizationSlot::Back, "backpacks", nullptr },
        { "backpack_leather_001", "Leather Knapsack", CustomizationSlot::Back, "backpacks", nullptr },
        { "backpack_adventure_001", "Expedition Pack", CustomizationSlot::Back, "backpacks", nullptr }
    };

    for (const auto& m : p8Items) {
        CosmeticDefinition def;
        def.id = m.id;
        def.displayName = m.name;
        def.slot = m.slot;
        def.category = m.cat;
        def.spritesheetId = "sheet_" + std::string(m.id);
        def.textureId = "tex_" + std::string(m.id);
        if (m.flag) {
            def.occlusionFlags.push_back(m.flag);
        }
        RegisterCosmetic(def);
    }
}

bool CosmeticRegistry::ValidateCosmetic(const std::string& id, const resources::AssetRegistry& assetReg) const {
    const auto* def = GetCosmetic(id);
    if (!def) return false;
    return assetReg.HasAsset(def->spritesheetId);
}

} // namespace wildbound::customization

