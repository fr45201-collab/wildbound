#include "AppearanceSerializer.hpp"
#include "../core/Logger.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace wildbound::customization {

bool AppearanceSerializer::SaveLoadouts(const std::string& filePath,
                                       const std::vector<AppearanceLoadout>& loadouts,
                                       const CharacterAppearance& activeAppearance) {
    try {
        fs::path path(filePath);
        if (path.has_parent_path()) {
            fs::create_directories(path.parent_path());
        }

        nlohmann::json root;
        root["schema_version"] = CURRENT_SCHEMA_VERSION;
        root["active_appearance"] = activeAppearance.ToJson();

        nlohmann::json loadoutsArray = nlohmann::json::array();
        for (const auto& l : loadouts) {
            loadoutsArray.push_back(l.ToJson());
        }
        root["loadouts"] = loadoutsArray;

        std::ofstream out(filePath, std::ios::trunc);
        if (!out.is_open()) {
            core::Logger::Error("APPEARANCE_SERIALIZER", "Failed to open appearance loadouts file for writing: " + filePath);
            return false;
        }

        out << root.dump(4);
        core::Logger::Info("APPEARANCE_SERIALIZER", "Saved " + std::to_string(loadouts.size()) + " loadouts to " + filePath);
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("APPEARANCE_SERIALIZER", "Exception saving appearance loadouts: " + std::string(e.what()));
        return false;
    }
}

bool AppearanceSerializer::LoadLoadouts(const std::string& filePath,
                                       std::vector<AppearanceLoadout>& outLoadouts,
                                       CharacterAppearance& outActiveAppearance) {
    if (!fs::exists(filePath)) {
        core::Logger::Info("APPEARANCE_SERIALIZER", "Loadout file does not exist. Initializing with defaults: " + filePath);
        outLoadouts = CreateDefaultLoadouts();
        outActiveAppearance.ResetToDefault();
        SaveLoadouts(filePath, outLoadouts, outActiveAppearance);
        return true;
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            core::Logger::Error("APPEARANCE_SERIALIZER", "Failed to open loadouts file: " + filePath);
            return false;
        }

        nlohmann::json root = nlohmann::json::parse(file);
        if (!root.is_object()) {
            core::Logger::Warn("APPEARANCE_SERIALIZER", "Malformed JSON root in loadouts file. Rebuilding defaults.");
            outLoadouts = CreateDefaultLoadouts();
            outActiveAppearance.ResetToDefault();
            return false;
        }

        // Schema version check
        uint32_t version = root.value("schema_version", 1u);
        if (version > CURRENT_SCHEMA_VERSION) {
            core::Logger::Warn("APPEARANCE_SERIALIZER", "Unsupported schema version: " + std::to_string(version));
        }

        if (root.contains("active_appearance") && root["active_appearance"].is_object()) {
            outActiveAppearance.FromJson(root["active_appearance"]);
        }

        outLoadouts.clear();
        if (root.contains("loadouts") && root["loadouts"].is_array()) {
            for (const auto& item : root["loadouts"]) {
                AppearanceLoadout l = AppearanceLoadout::FromJson(item);
                if (l.IsValid()) {
                    outLoadouts.push_back(l);
                }
            }
        }

        if (outLoadouts.empty()) {
            outLoadouts = CreateDefaultLoadouts();
        }

        core::Logger::Info("APPEARANCE_SERIALIZER", "Loaded " + std::to_string(outLoadouts.size()) + " loadouts from " + filePath);
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("APPEARANCE_SERIALIZER", "Error parsing loadouts JSON: " + std::string(e.what()) + ". Using defaults.");
        outLoadouts = CreateDefaultLoadouts();
        outActiveAppearance.ResetToDefault();
        return false;
    }
}

bool AppearanceSerializer::SaveAppearance(const std::string& filePath, const CharacterAppearance& appearance) {
    try {
        fs::path path(filePath);
        if (path.has_parent_path()) {
            fs::create_directories(path.parent_path());
        }

        std::ofstream out(filePath, std::ios::trunc);
        if (!out.is_open()) return false;
        out << appearance.ToJson().dump(4);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool AppearanceSerializer::LoadAppearance(const std::string& filePath, CharacterAppearance& outAppearance) {
    if (!fs::exists(filePath)) return false;

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;
        nlohmann::json j = nlohmann::json::parse(file);
        return outAppearance.FromJson(j);
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<AppearanceLoadout> AppearanceSerializer::CreateDefaultLoadouts() {
    std::vector<AppearanceLoadout> loadouts;

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // 1. Default Explorer
    {
        AppearanceLoadout l;
        l.id = "loadout_explorer";
        l.name = "Forest Explorer";
        l.timestamp = now;
        l.appearance.ResetToDefault();
        loadouts.push_back(l);
    }

    // 2. Armored Knight
    {
        AppearanceLoadout l;
        l.id = "loadout_knight";
        l.name = "Armored Knight";
        l.timestamp = now;
        l.appearance.SetCosmetic(CustomizationSlot::Skin, "skin_default");
        l.appearance.SetCosmetic(CustomizationSlot::Helmet, "hat_iron_helm");
        l.appearance.SetCosmetic(CustomizationSlot::Chest, "shirt_armor_001");
        l.appearance.SetCosmetic(CustomizationSlot::Legs, "pants_armored_001");
        l.appearance.SetCosmetic(CustomizationSlot::Feet, "shoes_armored_001");
        l.appearance.SetCosmetic(CustomizationSlot::Back, "backpack_adventure_001");
        loadouts.push_back(l);
    }

    // 3. Mystic Wanderer
    {
        AppearanceLoadout l;
        l.id = "loadout_mystic";
        l.name = "Mystic Wanderer";
        l.timestamp = now;
        l.appearance.SetCosmetic(CustomizationSlot::Skin, "skin_tan");
        l.appearance.SetCosmetic(CustomizationSlot::Hair, "hair_long_001");
        l.appearance.SetCosmetic(CustomizationSlot::Headwear, "hat_wizard_001");
        l.appearance.SetCosmetic(CustomizationSlot::Chest, "shirt_mage_001");
        l.appearance.SetCosmetic(CustomizationSlot::Legs, "pants_mage_001");
        l.appearance.SetCosmetic(CustomizationSlot::Feet, "shoes_light_001");
        l.appearance.SetCosmetic(CustomizationSlot::Face, "mask_mystic_001");
        loadouts.push_back(l);
    }

    return loadouts;
}

} // namespace wildbound::customization

