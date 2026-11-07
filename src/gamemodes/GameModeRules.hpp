#pragma once

#include "GameModeDefinition.hpp"

namespace wildbound::gamemodes {

/**
 * Runtime enforcement rules derived from GameModeDefinition.
 */
class GameModeRules {
public:
    GameModeRules() = default;
    explicit GameModeRules(const GameModeDefinition& def);

    void SetFromDefinition(const GameModeDefinition& def);

    [[nodiscard]] bool AreTeamsEnabled() const noexcept { return m_teamsEnabled; }
    [[nodiscard]] uint32_t GetTeamSize() const noexcept { return m_teamSize; }
    [[nodiscard]] bool IsFriendlyFireEnabled() const noexcept { return m_friendlyFireEnabled; }
    [[nodiscard]] bool AreBotsAllowed() const noexcept { return m_botsAllowed; }
    [[nodiscard]] bool IsRespawnEnabled() const noexcept { return m_respawnEnabled; }
    [[nodiscard]] bool IsStormEnabled() const noexcept { return m_stormEnabled; }
    [[nodiscard]] bool AreCompetitiveResultsEnabled() const noexcept { return m_competitiveResults; }
    [[nodiscard]] const std::string& GetVictoryCondition() const noexcept { return m_victoryCondition; }

    [[nodiscard]] bool ShouldAllowDamage(bool isTeammate) const noexcept {
        if (isTeammate) {
            return m_friendlyFireEnabled;
        }
        return true;
    }

private:
    bool m_teamsEnabled = false;
    uint32_t m_teamSize = 1;
    bool m_friendlyFireEnabled = false;
    bool m_botsAllowed = true;
    bool m_respawnEnabled = false;
    bool m_stormEnabled = true;
    bool m_competitiveResults = true;
    std::string m_victoryCondition = "last_survivor";
};

} // namespace wildbound::gamemodes

