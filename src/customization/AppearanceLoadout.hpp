#pragma once

#include "CharacterAppearance.hpp"
#include <string>
#include <cstdint>
#include <chrono>
#include <nlohmann/json.hpp>

namespace wildbound::customization {

/**
 * Saved appearance loadout preset (Phase 25).
 * Stores a named cosmetic appearance configuration without gameplay stat data.
 */
struct AppearanceLoadout {
    std::string id;
    std::string name = "Custom Loadout";
    CharacterAppearance appearance;
    uint32_t schemaVersion = 1;
    uint64_t timestamp = 0; // Unix timestamp in seconds

    [[nodiscard]] bool IsValid() const noexcept {
        return !id.empty() && !name.empty();
    }

    [[nodiscard]] nlohmann::json ToJson() const {
        nlohmann::json j;
        j["id"] = id;
        j["name"] = name;
        j["schema_version"] = schemaVersion;
        j["timestamp"] = timestamp;
        j["appearance"] = appearance.ToJson();
        return j;
    }

    static AppearanceLoadout FromJson(const nlohmann::json& j) {
        AppearanceLoadout loadout;
        if (j.contains("id") && j["id"].is_string()) {
            loadout.id = j["id"].get<std::string>();
        }
        if (j.contains("name") && j["name"].is_string()) {
            loadout.name = j["name"].get<std::string>();
        }
        if (j.contains("schema_version") && j["schema_version"].is_number_unsigned()) {
            loadout.schemaVersion = j["schema_version"].get<uint32_t>();
        }
        if (j.contains("timestamp") && j["timestamp"].is_number_unsigned()) {
            loadout.timestamp = j["timestamp"].get<uint64_t>();
        }
        if (j.contains("appearance") && j["appearance"].is_object()) {
            loadout.appearance.FromJson(j["appearance"]);
        }
        return loadout;
    }
};

} // namespace wildbound::customization

