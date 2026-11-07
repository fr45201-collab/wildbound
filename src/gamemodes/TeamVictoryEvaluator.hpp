#pragma once

#include "results/MatchOutcome.hpp"
#include "Team.hpp"
#include <string>
#include <cstdint>

namespace wildbound::match {
class MatchParticipantRegistry;
}

namespace wildbound::gamemodes {
class TeamManager;

/**
 * Team-aware victory evaluator for Duos and Squads Battle Royale.
 * Evaluates whether only one surviving team remains standing.
 */
class TeamVictoryEvaluator {
public:
    TeamVictoryEvaluator() = default;
    ~TeamVictoryEvaluator() = default;

    /**
     * Evaluates the current state of teams and participant registry.
     * @return True if a definitive end-of-match condition (Victory or Draw) was reached.
     */
    bool Evaluate(const TeamManager& teamManager,
                  const match::MatchParticipantRegistry& registry,
                  bool isMatchActive,
                  results::MatchOutcome& outOutcome,
                  TeamId& outWinningTeamId,
                  std::string& outWinningTeamName) const;
};

} // namespace wildbound::gamemodes

