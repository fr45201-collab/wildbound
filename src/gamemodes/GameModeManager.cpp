#include "GameModeManager.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "combat/CombatRules.hpp"
#include "core/Logger.hpp"

namespace wildbound::gamemodes {

GameModeManager::GameModeManager() {
    m_activeMode = GameModeDefinition::CreateSoloDefault();
    m_rules.SetFromDefinition(m_activeMode);
}

bool GameModeManager::Initialize(const std::string& modeDir, const std::string& botDifficultyPath) {
    bool modesLoaded = m_registry.LoadFromDirectory(modeDir);
    if (!modesLoaded) {
        // Fallback to checking assets path
        m_registry.LoadFromDirectory("assets/" + modeDir);
    }

    bool botLoaded = m_botConfig.LoadDifficultyProfiles(botDifficultyPath);
    if (!botLoaded) {
        m_botConfig.LoadDifficultyProfiles("assets/" + botDifficultyPath);
    }

    // Default to Solo
    SelectMode("solo");

    core::Logger::Info("GAMEMODE", "GameModeManager initialized successfully. Active mode: [" + m_activeMode.displayName + "]");
    return true;
}

bool GameModeManager::SelectMode(const std::string& modeId) {
    const auto* modeDef = m_registry.GetMode(modeId);
    if (!modeDef) {
        core::Logger::Warn("GAMEMODE", "Unknown game mode ID: " + modeId);
        return false;
    }

    m_activeMode = *modeDef;
    m_rules.SetFromDefinition(m_activeMode);
    m_teamManager.SetMaxTeamSize(m_activeMode.teamSize);

    if (modeId == "practice") {
        m_practiceMode.Initialize();
    } else {
        m_practiceMode.Reset();
    }

    m_botConfig.SetBotCount(m_activeMode.defaultBotCount);
    m_botConfig.SetDifficulty(StringToBotDifficulty(m_activeMode.botDifficulty));

    core::Logger::Info("GAMEMODE", "Selected game mode: [" + m_activeMode.displayName + "] (Teams: " +
                 (m_activeMode.teamsEnabled ? "Enabled" : "Disabled") + ", TeamSize: " +
                 std::to_string(m_activeMode.teamSize) + ")");
    return true;
}

match::MatchConfiguration GameModeManager::GenerateMatchConfiguration(const std::string& matchId) const {
    match::MatchConfiguration config;
    config.matchId = matchId;
    config.minimumParticipants = m_activeMode.minimumParticipants;
    config.maximumParticipants = m_activeMode.maximumParticipants;
    config.allowAiParticipants = m_activeMode.botsAllowed;
    config.worldId = m_activeMode.environment;
    config.stormConfigId = m_activeMode.stormConfigId;
    config.respawnEnabled = m_activeMode.respawnEnabled;
    config.victoryCondition = m_activeMode.victoryCondition;
    config.showResultsScreen = m_activeMode.competitiveResults;

    if (m_activeMode.id == "practice") {
        config.countdownSeconds = 0.0f;
        config.allowDevSingleParticipant = true;
    } else {
        config.countdownSeconds = 5.0f;
        config.allowDevSingleParticipant = false;
    }

    return config;
}

bool GameModeManager::CanDamage(uint32_t attackerParticipantId, uint32_t targetParticipantId) const {
    if (attackerParticipantId == 0 || targetParticipantId == 0) return true;
    if (attackerParticipantId == targetParticipantId) return false;

    if (!m_activeMode.teamsEnabled) {
        // In Solo or free-for-all, all competitors can damage each other
        return true;
    }

    bool isTeammate = m_teamManager.IsTeammate(attackerParticipantId, targetParticipantId);
    return m_rules.ShouldAllowDamage(isTeammate);
}

void GameModeManager::ApplyRulesToCombat(combat::CombatRules& combatRules) const {
    combatRules.friendlyFire = m_activeMode.friendlyFireEnabled;
    combatRules.allowPvP = true; // BR modes allow PvP
    combatRules.allowPvE = true;
}

void GameModeManager::SetupTeamsForParticipants(const match::MatchParticipantRegistry& registry) {
    m_teamManager.ClearTeams(const_cast<match::MatchParticipantRegistry*>(&registry));

    const auto& participants = registry.GetAllParticipants();

    if (!m_activeMode.teamsEnabled) {
        // In Solo mode, create a 1-person team for each participant so team-based queries work uniformly
        m_teamManager.SetMaxTeamSize(1);
        for (const auto& p : participants) {
            std::string teamName = p.displayName.empty() ? ("Participant " + std::to_string(p.participantId)) : p.displayName;
            TeamId tid = m_teamManager.CreateTeam(teamName);
            m_teamManager.AddParticipantToTeam(tid, p.participantId, const_cast<match::MatchParticipantRegistry*>(&registry));
        }
        return;
    }

    // In Duos / Squads mode: distribute participants evenly into teams of teamSize
    m_teamManager.SetMaxTeamSize(m_activeMode.teamSize);

    TeamId currentTeamId = 0;
    uint32_t membersInCurrentTeam = 0;

    for (const auto& p : participants) {
        uint32_t pid = p.participantId;

        if (membersInCurrentTeam == 0 || membersInCurrentTeam >= m_activeMode.teamSize) {
            uint32_t teamNum = static_cast<uint32_t>(m_teamManager.GetTotalTeamCount() + 1);
            currentTeamId = m_teamManager.CreateTeam("Squad " + std::to_string(teamNum));
            membersInCurrentTeam = 0;
        }

        m_teamManager.AddParticipantToTeam(currentTeamId, pid, const_cast<match::MatchParticipantRegistry*>(&registry));
        ++membersInCurrentTeam;
    }

    core::Logger::Info("GAMEMODE", "Configured " + std::to_string(m_teamManager.GetTotalTeamCount()) +
                 " teams for " + std::to_string(participants.size()) + " participants.");
}

bool GameModeManager::EvaluateVictory(const match::MatchParticipantRegistry& registry,
                                      bool isMatchActive,
                                      results::MatchOutcome& outOutcome,
                                      uint32_t& outWinnerId,
                                      std::string& outWinnerName) {
    outWinnerId = 0;
    outWinnerName = "No Winner (Draw)";

    if (!isMatchActive) {
        outOutcome = results::MatchOutcome::InProgress;
        return false;
    }

    // 1. Practice Mode: Never declares competitive victory
    if (m_activeMode.victoryCondition == "practice_freeplay") {
        outOutcome = results::MatchOutcome::InProgress;
        return false;
    }

    // 2. Team-Based Battle Royale (Duos, Squads)
    if (m_activeMode.teamsEnabled) {
        m_teamManager.UpdateTeamEliminationStatus(registry);

        TeamId winningTeamId = 0;
        bool decided = m_teamVictoryEvaluator.Evaluate(m_teamManager, registry, isMatchActive,
                                                      outOutcome, winningTeamId, outWinnerName);
        if (decided) {
            outWinnerId = winningTeamId;
        }
        return decided;
    }

    // 3. Solo Battle Royale / Offline Bots
    size_t totalCount = registry.GetTotalCount();
    if (totalCount == 0) {
        outOutcome = results::MatchOutcome::Failed;
        return true;
    }

    auto aliveIds = registry.GetAliveParticipantIds();
    size_t aliveCount = aliveIds.size();

    if (aliveCount == 1) {
        outOutcome = results::MatchOutcome::Victory;
        outWinnerId = aliveIds[0];
        const auto* winner = registry.FindParticipant(outWinnerId);
        if (winner) {
            outWinnerName = winner->displayName;
        } else {
            outWinnerName = "Unknown Champion";
        }
        return true;
    }

    if (aliveCount == 0) {
        outOutcome = results::MatchOutcome::Draw;
        outWinnerId = 0;
        outWinnerName = "No Winner (Draw)";
        return true;
    }

    outOutcome = results::MatchOutcome::InProgress;
    return false;
}

void GameModeManager::Reset() {
    m_teamManager.Reset();
    m_practiceMode.Reset();
    m_teamManager.SetMaxTeamSize(m_activeMode.teamSize);
}

} // namespace wildbound::gamemodes
