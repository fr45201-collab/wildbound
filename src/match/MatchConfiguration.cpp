#include "MatchConfiguration.hpp"
#include "../core/Logger.hpp"
#include <fstream>

namespace wildbound::match {

MatchConfiguration MatchConfiguration::CreateDefault() {
    MatchConfiguration cfg;
    cfg.matchId = "wildbound_default";
    cfg.countdownSeconds = 10.0f;
    cfg.minimumParticipants = 2;
    cfg.maximumParticipants = 100;
    cfg.allowAiParticipants = false;
    cfg.worldId = "battle_royale_island";
    cfg.stormConfigId = "default_corruption_storm";
    cfg.respawnEnabled = false;
    cfg.victoryCondition = "last_survivor";
    cfg.showResultsScreen = true;
    cfg.allowDevSingleParticipant = false;
    return cfg;
}

MatchConfiguration MatchConfiguration::FromJson(const nlohmann::json& j) {
    MatchConfiguration cfg = CreateDefault();

    if (j.contains("match_id") && j["match_id"].is_string()) {
        cfg.matchId = j["match_id"].get<std::string>();
    }
    if (j.contains("match_countdown_seconds") && j["match_countdown_seconds"].is_number()) {
        cfg.countdownSeconds = j["match_countdown_seconds"].get<float>();
    }
    if (j.contains("minimum_participants") && j["minimum_participants"].is_number_unsigned()) {
        cfg.minimumParticipants = j["minimum_participants"].get<uint32_t>();
    }
    if (j.contains("maximum_participants") && j["maximum_participants"].is_number_unsigned()) {
        cfg.maximumParticipants = j["maximum_participants"].get<uint32_t>();
    }
    if (j.contains("allow_ai_participants") && j["allow_ai_participants"].is_boolean()) {
        cfg.allowAiParticipants = j["allow_ai_participants"].get<bool>();
    }
    if (j.contains("world_id") && j["world_id"].is_string()) {
        cfg.worldId = j["world_id"].get<std::string>();
    }
    if (j.contains("storm_config_id") && j["storm_config_id"].is_string()) {
        cfg.stormConfigId = j["storm_config_id"].get<std::string>();
    }
    if (j.contains("respawn_enabled") && j["respawn_enabled"].is_boolean()) {
        cfg.respawnEnabled = j["respawn_enabled"].get<bool>();
    }
    if (j.contains("victory_condition") && j["victory_condition"].is_string()) {
        cfg.victoryCondition = j["victory_condition"].get<std::string>();
    }
    if (j.contains("show_results_screen") && j["show_results_screen"].is_boolean()) {
        cfg.showResultsScreen = j["show_results_screen"].get<bool>();
    }
    if (j.contains("allow_dev_single_participant") && j["allow_dev_single_participant"].is_boolean()) {
        cfg.allowDevSingleParticipant = j["allow_dev_single_participant"].get<bool>();
    }

    return cfg;
}

nlohmann::json MatchConfiguration::ToJson() const {
    return nlohmann::json{
        { "match_id", matchId },
        { "match_countdown_seconds", countdownSeconds },
        { "minimum_participants", minimumParticipants },
        { "maximum_participants", maximumParticipants },
        { "allow_ai_participants", allowAiParticipants },
        { "world_id", worldId },
        { "storm_config_id", stormConfigId },
        { "respawn_enabled", respawnEnabled },
        { "victory_condition", victoryCondition },
        { "show_results_screen", showResultsScreen },
        { "allow_dev_single_participant", allowDevSingleParticipant }
    };
}

bool MatchConfiguration::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        core::Logger::Warn("MATCH_CONFIG", "Failed to open config file: " + path + ". Using defaults.");
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        *this = FromJson(j);
        core::Logger::Info("MATCH_CONFIG", "Successfully loaded match configuration from: " + path);
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("MATCH_CONFIG", "JSON parse error in " + path + ": " + e.what());
        return false;
    }
}

bool MatchConfiguration::SaveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        core::Logger::Error("MATCH_CONFIG", "Failed to write config file: " + path);
        return false;
    }
    file << ToJson().dump(2);
    return true;
}

bool MatchConfiguration::Validate(std::vector<std::string>& outErrors) const {
    bool valid = true;
    if (matchId.empty()) {
        outErrors.push_back("matchId cannot be empty.");
        valid = false;
    }
    if (countdownSeconds < 0.0f) {
        outErrors.push_back("countdownSeconds cannot be negative.");
        valid = false;
    }
    if (minimumParticipants < 1) {
        outErrors.push_back("minimumParticipants must be at least 1.");
        valid = false;
    }
    if (maximumParticipants < minimumParticipants) {
        outErrors.push_back("maximumParticipants cannot be less than minimumParticipants.");
        valid = false;
    }
    if (worldId.empty()) {
        outErrors.push_back("worldId cannot be empty.");
        valid = false;
    }
    return valid;
}

} // namespace wildbound::match
