#pragma once

#include "AppearanceLoadout.hpp"
#include <vector>
#include <string>

namespace wildbound::customization {

class AppearanceSerializer {
public:
    static constexpr const char* DEFAULT_LOADOUT_FILE_PATH = "user_data/character/appearance_loadouts.json";
    static constexpr uint32_t CURRENT_SCHEMA_VERSION = 1;

    // Loadouts persistence
    static bool SaveLoadouts(const std::string& filePath,
                             const std::vector<AppearanceLoadout>& loadouts,
                             const CharacterAppearance& activeAppearance);

    static bool LoadLoadouts(const std::string& filePath,
                             std::vector<AppearanceLoadout>& outLoadouts,
                             CharacterAppearance& outActiveAppearance);

    // Single appearance persistence
    static bool SaveAppearance(const std::string& filePath, const CharacterAppearance& appearance);
    static bool LoadAppearance(const std::string& filePath, CharacterAppearance& outAppearance);

    // Default factory
    static std::vector<AppearanceLoadout> CreateDefaultLoadouts();
};

} // namespace wildbound::customization

