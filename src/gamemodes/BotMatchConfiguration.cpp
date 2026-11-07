#include "BotMatchConfiguration.hpp"
#include "core/Logger.hpp"
#include <fstream>

using wildbound::core::Logger;

namespace wildbound::gamemodes {

const char* BotDifficultyToString(BotDifficulty diff) noexcept {
    switch (diff) {
        case BotDifficulty::Easy:   return "easy";
        case BotDifficulty::Normal: return "normal";
        case BotDifficulty::Hard:   return "hard";
        default:                    return "normal";
    }
}

BotDifficulty StringToBotDifficulty(const std::string& str) noexcept {
    if (str == "easy" || str == "Easy") return BotDifficulty::Easy;
    if (str == "hard" || str == "Hard") return BotDifficulty::Hard;
    return BotDifficulty::Normal;
}

BotMatchConfiguration::BotMatchConfiguration() {
    InitializeDefaults();
}

void BotMatchConfiguration::InitializeDefaults() {
    BotDifficultyParams easy;
    easy.perceptionRange = 180.0f;
    easy.reactionDelay = 0.8f;
    easy.attackCooldownMultiplier = 1.6f;
    easy.movementSpeedMultiplier = 0.85f;
    easy.stormEvacuateThreshold = 12.0f;
    easy.retreatHealthPercentage = 0.15f;
    easy.aggressionWeight = 0.35f;
    m_profiles["easy"] = easy;

    BotDifficultyParams normal;
    normal.perceptionRange = 280.0f;
    normal.reactionDelay = 0.35f;
    normal.attackCooldownMultiplier = 1.0f;
    normal.movementSpeedMultiplier = 1.0f;
    normal.stormEvacuateThreshold = 20.0f;
    normal.retreatHealthPercentage = 0.25f;
    normal.aggressionWeight = 0.65f;
    m_profiles["normal"] = normal;

    BotDifficultyParams hard;
    hard.perceptionRange = 400.0f;
    hard.reactionDelay = 0.12f;
    hard.attackCooldownMultiplier = 0.75f;
    hard.movementSpeedMultiplier = 1.15f;
    hard.stormEvacuateThreshold = 30.0f;
    hard.retreatHealthPercentage = 0.35f;
    hard.aggressionWeight = 0.90f;
    m_profiles["hard"] = hard;
}

bool BotMatchConfiguration::LoadDifficultyProfiles(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::Warn("BOT_CONFIG", "Could not open difficulty config: " + path + ". Using defaults.");
            return false;
        }

        nlohmann::json j;
        file >> j;

        if (j.contains("difficulties")) {
            for (auto& [diffKey, val] : j["difficulties"].items()) {
                BotDifficultyParams params;
                params.perceptionRange = val.value("perceptionRange", 280.0f);
                params.reactionDelay = val.value("reactionDelay", 0.35f);
                params.attackCooldownMultiplier = val.value("attackCooldownMultiplier", 1.0f);
                params.movementSpeedMultiplier = val.value("movementSpeedMultiplier", 1.0f);
                params.stormEvacuateThreshold = val.value("stormEvacuateThreshold", 20.0f);
                params.retreatHealthPercentage = val.value("retreatHealthPercentage", 0.25f);
                params.aggressionWeight = val.value("aggressionWeight", 0.65f);

                m_profiles[diffKey] = params;
            }
        }
        return true;
    } catch (const std::exception& e) {
        Logger::Error("BOT_CONFIG", "Error parsing bot difficulty file: " + std::string(e.what()));
        return false;
    }
}

BotDifficultyParams BotMatchConfiguration::GetParams(BotDifficulty diff) const {
    std::string key = BotDifficultyToString(diff);
    auto it = m_profiles.find(key);
    if (it != m_profiles.end()) {
        return it->second;
    }
    return BotDifficultyParams{};
}

} // namespace wildbound::gamemodes
