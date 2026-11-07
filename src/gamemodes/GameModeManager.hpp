#pragma once

#include "GameModeDefinition.hpp"
#include "GameModeRules.hpp"
#include "GameModeRegistry.hpp"
#include "TeamManager.hpp"
#include "TeamVictoryEvaluator.hpp"
#include "BotMatchConfiguration.hpp"
#include "PracticeMode.hpp"
#include "match/MatchConfiguration.hpp"
#include "results/MatchOutcome.hpp"
#include <string>
#include <memory>

namespace wildbound::match {
class MatchManager;
class MatchParticipantRegistry;
}

namespace wildbound::combat {
class CombatRules;
}

namespace wildbound::gamemodes {

/**
 * Top-level Game Modes coordinator for Wildbound.
 * Centralizes mode definitions, team lifecycle, bot configurations, and match rules.
 */
class GameModeManager {
public:
    GameModeManager();
    ~GameModeManager() = default;

    bool Initialize(const std::string& modeDir = "data/gamemodes",
                    const std::string& botDifficultyPath = "data/gamemodes/bot_difficulty.json");

    // Mode Selection & Activation
    bool SelectMode(const std::string& modeId);
    [[nodiscard]] const GameModeDefinition& GetActiveMode() const noexcept { return m_activeMode; }
    [[nodiscard]] const GameModeRules& GetActiveRules() const noexcept { return m_rules; }

    // Subsystems
    [[nodiscard]] TeamManager& GetTeamManager() noexcept { return m_teamManager; }
    [[nodiscard]] const TeamManager& GetTeamManager() const noexcept { return m_teamManager; }

    [[nodiscard]] PracticeMode& GetPracticeMode() noexcept { return m_practiceMode; }
    [[nodiscard]] const PracticeMode& GetPracticeMode() const noexcept { return m_practiceMode; }

    [[nodiscard]] BotMatchConfiguration& GetBotConfig() noexcept { return m_botConfig; }
    [[nodiscard]] const BotMatchConfiguration& GetBotConfig() const noexcept { return m_botConfig; }

    [[nodiscard]] GameModeRegistry& GetRegistry() noexcept { return m_registry; }
    [[nodiscard]] const GameModeRegistry& GetRegistry() const noexcept { return m_registry; }

    // Match Configuration Generation
    [[nodiscard]] match::MatchConfiguration GenerateMatchConfiguration(const std::string& matchId = "wildbound_match") const;

    // Combat & Friendly-Fire Integration
    [[nodiscard]] bool CanDamage(uint32_t attackerParticipantId, uint32_t targetParticipantId) const;
    void ApplyRulesToCombat(combat::CombatRules& combatRules) const;

    // Team Assignment
    void SetupTeamsForParticipants(const match::MatchParticipantRegistry& registry);

    // Victory Evaluation Integration
    bool EvaluateVictory(const match::MatchParticipantRegistry& registry,
                         bool isMatchActive,
                         results::MatchOutcome& outOutcome,
                         uint32_t& outWinnerId,
                         std::string& outWinnerName);

    // Reset runtime state between matches
    void Reset();

private:
    GameModeRegistry m_registry;
    GameModeDefinition m_activeMode;
    GameModeRules m_rules;
    TeamManager m_teamManager;
    TeamVictoryEvaluator m_teamVictoryEvaluator;
    BotMatchConfiguration m_botConfig;
    PracticeMode m_practiceMode;
};

} // namespace wildbound::gamemodes

