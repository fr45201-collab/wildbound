#include "CharacterConfig.hpp"
#include "../core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::entities {

CharacterConfig CharacterConfig::CreateDefault() {
    CharacterConfig cfg;
    cfg.id = "player";
    cfg.name = "Explorer";
    cfg.spritesheet = "player_spritesheet";
    cfg.stats.walkSpeed = 100.0f;
    cfg.stats.runSpeed = 165.0f;
    cfg.stats.normalizeDiagonal = true;

    cfg.collision.enabled = true;
    cfg.collision.width = 16.0f;
    cfg.collision.height = 12.0f;
    cfg.collision.offsetX = 0.0f;
    cfg.collision.offsetY = 0.0f;

    cfg.animationClips = {
        { "idle_down", "player_idle_down" },
        { "idle_up",   "player_idle_up" },
        { "idle_left", "player_idle_left" },
        { "idle_right","player_idle_right" },
        { "walk_down", "player_walk_down" },
        { "walk_up",   "player_walk_up" },
        { "walk_left", "player_walk_left" },
        { "walk_right","player_walk_right" },
        { "run_down",  "player_run_down" },
        { "run_up",    "player_run_up" },
        { "run_left",  "player_run_left" },
        { "run_right", "player_run_right" }
    };

    return cfg;
}

CharacterConfig CharacterConfig::LoadFromFile(const std::string& jsonPath) {
    if (!fs::exists(jsonPath)) {
        Logger::Warn("CONFIG", "Character config not found at '" + jsonPath + "', using defaults.");
        return CreateDefault();
    }

    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            Logger::Warn("CONFIG", "Could not open '" + jsonPath + "', using defaults.");
            return CreateDefault();
        }

        nlohmann::json j;
        file >> j;

        CharacterConfig cfg = CreateDefault();
        cfg.id = j.value("id", cfg.id);
        cfg.name = j.value("name", cfg.name);
        cfg.spritesheet = j.value("spritesheet", cfg.spritesheet);

        if (j.contains("movement")) {
            const auto& mov = j["movement"];
            cfg.stats.walkSpeed = mov.value("walk_speed", cfg.stats.walkSpeed);
            cfg.stats.runSpeed = mov.value("run_speed", cfg.stats.runSpeed);
            cfg.stats.normalizeDiagonal = mov.value("diagonal_normalization", cfg.stats.normalizeDiagonal);
        }

        if (j.contains("collision")) {
            const auto& col = j["collision"];
            cfg.collision.enabled = col.value("enabled", cfg.collision.enabled);
            cfg.collision.width = col.value("width", cfg.collision.width);
            cfg.collision.height = col.value("height", cfg.collision.height);
            cfg.collision.offsetX = col.value("offset_x", cfg.collision.offsetX);
            cfg.collision.offsetY = col.value("offset_y", cfg.collision.offsetY);
        }

        if (j.contains("animations") && j["animations"].is_object()) {
            for (auto& [key, val] : j["animations"].items()) {
                if (val.is_string()) {
                    cfg.animationClips[key] = val.get<std::string>();
                }
            }
        }

        Logger::Info("CONFIG", "Loaded character configuration for [" + cfg.id + "] from " + jsonPath);
        return cfg;
    }
    catch (const std::exception& ex) {
        Logger::Error("CONFIG", "Error parsing character config '" + jsonPath + "': " + ex.what());
        return CreateDefault();
    }
}

} // namespace wildbound::entities

