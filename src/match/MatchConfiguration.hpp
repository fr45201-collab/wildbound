#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace wildbound::match {

/**
 * Data-Driven Battle Royale Match Configuration.
 */
struct MatchConfiguration {
    std::string matchId = "wildbound_default";
    float countdownSeconds = 10.0f;
    uint32_t minimumParticipants = 2;
    uint32_t maximumParticipants = 100;
    bool allowAiParticipants = false;
    std::string worldId = "battle_royale_island";
    std::string stormConfigId = "default_corruption_storm";
    bool respawnEnabled = false;
    std::string victoryCondition = "last_survivor";
    bool showResultsScreen = true;
    bool allowDevSingleParticipant = false; // Bypass for developer solo testing

    static MatchConfiguration CreateDefault();
    static MatchConfiguration FromJson(const nlohmann::json& j);
    [[nodiscard]] nlohmann::json ToJson() const;

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;
    [[nodiscard]] bool Validate(std::vector<std::string>& outErrors) const;
};

} // namespace wildbound::match

