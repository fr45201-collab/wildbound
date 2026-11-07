#include "GameModeRegistry.hpp"
#include "core/Logger.hpp"
#include <filesystem>

using wildbound::core::Logger;

namespace wildbound::gamemodes {

GameModeRegistry::GameModeRegistry() {
    RegisterBuiltinDefaults();
}

void GameModeRegistry::RegisterBuiltinDefaults() {
    // 1. Solo
    RegisterMode(GameModeDefinition::CreateSoloDefault());

    // 2. Duos
    GameModeDefinition duos;
    duos.id = "duos";
    duos.displayName = "Duos Battle Royale";
    duos.description = "Team up with a partner in 2-player squads. Be the last surviving duo.";
    duos.minimumParticipants = 2;
    duos.maximumParticipants = 100;
    duos.teamSize = 2;
    duos.teamsEnabled = true;
    duos.friendlyFireEnabled = false;
    duos.botsAllowed = true;
    duos.defaultBotCount = 14;
    duos.botDifficulty = "normal";
    duos.respawnEnabled = false;
    duos.stormEnabled = true;
    duos.stormConfigId = "default_corruption_storm";
    duos.competitiveResults = true;
    duos.victoryCondition = "last_team";
    duos.environment = "battle_royale_island";
    duos.isAvailable = true;
    duos.requiresNetworking = false;
    RegisterMode(duos);

    // 3. Squads
    GameModeDefinition squads;
    squads.id = "squads";
    squads.displayName = "Squads Battle Royale";
    squads.description = "Form a 4-player team to dominate the island. Outlast opposing squads.";
    squads.minimumParticipants = 4;
    squads.maximumParticipants = 100;
    squads.teamSize = 4;
    squads.teamsEnabled = true;
    squads.friendlyFireEnabled = false;
    squads.botsAllowed = true;
    squads.defaultBotCount = 16;
    squads.botDifficulty = "normal";
    squads.respawnEnabled = false;
    squads.stormEnabled = true;
    squads.stormConfigId = "default_corruption_storm";
    squads.competitiveResults = true;
    squads.victoryCondition = "last_team";
    squads.environment = "battle_royale_island";
    squads.isAvailable = true;
    squads.requiresNetworking = false;
    RegisterMode(squads);

    // 4. Practice
    GameModeDefinition practice;
    practice.id = "practice";
    practice.displayName = "Practice Arena";
    practice.description = "Explore, test weapons, and improve your gameplay skills without competitive stress.";
    practice.minimumParticipants = 1;
    practice.maximumParticipants = 1;
    practice.teamSize = 1;
    practice.teamsEnabled = false;
    practice.friendlyFireEnabled = false;
    practice.botsAllowed = false;
    practice.defaultBotCount = 0;
    practice.botDifficulty = "normal";
    practice.respawnEnabled = true;
    practice.stormEnabled = false;
    practice.stormConfigId = "none";
    practice.competitiveResults = false;
    practice.victoryCondition = "practice_freeplay";
    practice.environment = "forest_01";
    practice.isAvailable = true;
    practice.requiresNetworking = false;
    RegisterMode(practice);

    // 5. Offline Bots
    GameModeDefinition offlineBots;
    offlineBots.id = "offline_bots";
    offlineBots.displayName = "Offline Bot Matches";
    offlineBots.description = "Fight AI-controlled competitors in a full Battle Royale match without an internet connection.";
    offlineBots.minimumParticipants = 2;
    offlineBots.maximumParticipants = 32;
    offlineBots.teamSize = 1;
    offlineBots.teamsEnabled = false;
    offlineBots.friendlyFireEnabled = false;
    offlineBots.botsAllowed = true;
    offlineBots.defaultBotCount = 15;
    offlineBots.botDifficulty = "normal";
    offlineBots.respawnEnabled = false;
    offlineBots.stormEnabled = true;
    offlineBots.stormConfigId = "default_corruption_storm";
    offlineBots.competitiveResults = true;
    offlineBots.victoryCondition = "last_survivor";
    offlineBots.environment = "battle_royale_island";
    offlineBots.isAvailable = true;
    offlineBots.requiresNetworking = false;
    RegisterMode(offlineBots);
}

bool GameModeRegistry::LoadFromDirectory(const std::string& directoryPath) {
    namespace fs = std::filesystem;
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        Logger::Warn("GAMEMODE", "Directory does not exist: " + directoryPath + ". Using builtin defaults.");
        return false;
    }

    size_t loadedCount = 0;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            // Avoid loading difficulty profile as a game mode
            if (entry.path().filename() == "bot_difficulty.json") {
                continue;
            }

            GameModeDefinition def;
            if (def.LoadFromFile(entry.path().string())) {
                std::vector<std::string> errors;
                if (def.Validate(errors)) {
                    RegisterMode(def);
                    ++loadedCount;
                } else {
                    for (const auto& err : errors) {
                        Logger::Warn("GAMEMODE", "Invalid mode config in " + entry.path().string() + ": " + err);
                    }
                }
            }
        }
    }

    Logger::Info("GAMEMODE", "Loaded " + std::to_string(loadedCount) + " game modes from " + directoryPath);
    return loadedCount > 0;
}

void GameModeRegistry::RegisterMode(const GameModeDefinition& def) {
    m_modes[def.id] = def;
}

const GameModeDefinition* GameModeRegistry::GetMode(const std::string& id) const {
    auto it = m_modes.find(id);
    return (it != m_modes.end()) ? &it->second : nullptr;
}

bool GameModeRegistry::HasMode(const std::string& id) const {
    return m_modes.find(id) != m_modes.end();
}

std::vector<GameModeDefinition> GameModeRegistry::GetAllModes() const {
    std::vector<GameModeDefinition> result;
    result.reserve(m_modes.size());
    for (const auto& [id, def] : m_modes) {
        result.push_back(def);
    }
    return result;
}

void GameModeRegistry::Clear() {
    m_modes.clear();
}

} // namespace wildbound::gamemodes
