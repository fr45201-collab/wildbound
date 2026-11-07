#include "GameModeDefinition.hpp"
#include "core/Logger.hpp"
#include <fstream>

using wildbound::core::Logger;

namespace wildbound::gamemodes {

GameModeDefinition GameModeDefinition::CreateSoloDefault() {
    GameModeDefinition def;
    def.id = "solo";
    def.displayName = "Solo Battle Royale";
    def.description = "Every participant fights independently. Be the last survivor.";
    def.minimumParticipants = 2;
    def.maximumParticipants = 100;
    def.teamSize = 1;
    def.teamsEnabled = false;
    def.friendlyFireEnabled = false;
    def.botsAllowed = true;
    def.defaultBotCount = 15;
    def.botDifficulty = "normal";
    def.respawnEnabled = false;
    def.stormEnabled = true;
    def.stormConfigId = "default_corruption_storm";
    def.competitiveResults = true;
    def.victoryCondition = "last_survivor";
    def.environment = "battle_royale_island";
    def.isAvailable = true;
    def.requiresNetworking = false;
    return def;
}

GameModeDefinition GameModeDefinition::FromJson(const nlohmann::json& j) {
    GameModeDefinition def;
    def.id = j.value("id", "solo");
    def.displayName = j.value("displayName", "Solo Battle Royale");
    def.description = j.value("description", "");

    def.minimumParticipants = j.value("minimumParticipants", 2u);
    def.maximumParticipants = j.value("maximumParticipants", 100u);
    def.teamSize = j.value("teamSize", 1u);

    def.teamsEnabled = j.value("teamsEnabled", false);
    def.friendlyFireEnabled = j.value("friendlyFireEnabled", false);
    def.botsAllowed = j.value("botsAllowed", true);
    def.defaultBotCount = j.value("defaultBotCount", 15u);
    def.botDifficulty = j.value("botDifficulty", "normal");

    def.respawnEnabled = j.value("respawnEnabled", false);
    def.stormEnabled = j.value("stormEnabled", true);
    def.stormConfigId = j.value("stormConfigId", "default_corruption_storm");
    def.competitiveResults = j.value("competitiveResults", true);
    def.victoryCondition = j.value("victoryCondition", "last_survivor");
    def.environment = j.value("environment", "battle_royale_island");

    def.isAvailable = j.value("isAvailable", true);
    def.requiresNetworking = j.value("requiresNetworking", false);
    return def;
}

nlohmann::json GameModeDefinition::ToJson() const {
    return nlohmann::json{
        { "id", id },
        { "displayName", displayName },
        { "description", description },
        { "minimumParticipants", minimumParticipants },
        { "maximumParticipants", maximumParticipants },
        { "teamSize", teamSize },
        { "teamsEnabled", teamsEnabled },
        { "friendlyFireEnabled", friendlyFireEnabled },
        { "botsAllowed", botsAllowed },
        { "defaultBotCount", defaultBotCount },
        { "botDifficulty", botDifficulty },
        { "respawnEnabled", respawnEnabled },
        { "stormEnabled", stormEnabled },
        { "stormConfigId", stormConfigId },
        { "competitiveResults", competitiveResults },
        { "victoryCondition", victoryCondition },
        { "environment", environment },
        { "isAvailable", isAvailable },
        { "requiresNetworking", requiresNetworking }
    };
}

bool GameModeDefinition::LoadFromFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::Error("GAMEMODE", "Failed to open game mode config file: " + path);
            return false;
        }
        nlohmann::json j;
        file >> j;
        *this = FromJson(j);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("GAMEMODE", "JSON parse error in " + path + ": " + e.what());
        return false;
    }
}

bool GameModeDefinition::SaveToFile(const std::string& path) const {
    try {
        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }
        file << ToJson().dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

bool GameModeDefinition::Validate(std::vector<std::string>& outErrors) const {
    bool valid = true;
    if (id.empty()) {
        outErrors.push_back("Game mode ID cannot be empty.");
        valid = false;
    }
    if (minimumParticipants == 0) {
        outErrors.push_back("minimumParticipants must be at least 1.");
        valid = false;
    }
    if (maximumParticipants < minimumParticipants) {
        outErrors.push_back("maximumParticipants cannot be less than minimumParticipants.");
        valid = false;
    }
    if (teamsEnabled && teamSize < 2) {
        outErrors.push_back("teamSize must be at least 2 when teams are enabled.");
        valid = false;
    }
    if (!teamsEnabled && teamSize != 1) {
        outErrors.push_back("teamSize must be 1 when teams are disabled.");
        valid = false;
    }
    if (victoryCondition != "last_survivor" &&
        victoryCondition != "last_team" &&
        victoryCondition != "practice_freeplay") {
        outErrors.push_back("Unsupported victoryCondition: " + victoryCondition);
        valid = false;
    }
    return valid;
}

} // namespace wildbound::gamemodes
