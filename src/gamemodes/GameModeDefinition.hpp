#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace wildbound::gamemodes {

/**
 * Data-Driven Game Mode Definition.
 * Fully configurable specification for match rules, team structures, bot settings, and storm parameters.
 */
struct GameModeDefinition {
    std::string id = "solo";
    std::string displayName = "Solo Battle Royale";
    std::string description = "Fight independently to become the last survivor.";

    uint32_t minimumParticipants = 2;
    uint32_t maximumParticipants = 100;
    uint32_t teamSize = 1;

    bool teamsEnabled = false;
    bool friendlyFireEnabled = false;
    bool botsAllowed = true;
    uint32_t defaultBotCount = 15;
    std::string botDifficulty = "normal";

    bool respawnEnabled = false;
    bool stormEnabled = true;
    std::string stormConfigId = "default_corruption_storm";
    bool competitiveResults = true;
    std::string victoryCondition = "last_survivor";
    std::string environment = "battle_royale_island";

    bool isAvailable = true;
    bool requiresNetworking = false;

    static GameModeDefinition CreateSoloDefault();
    static GameModeDefinition FromJson(const nlohmann::json& j);
    [[nodiscard]] nlohmann::json ToJson() const;

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;
    [[nodiscard]] bool Validate(std::vector<std::string>& outErrors) const;
};

} // namespace wildbound::gamemodes

