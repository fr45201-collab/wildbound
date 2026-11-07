#include "TeamVictoryEvaluator.hpp"
#include "TeamManager.hpp"
#include "match/MatchParticipantRegistry.hpp"

namespace wildbound::gamemodes {

bool TeamVictoryEvaluator::Evaluate(const TeamManager& teamManager,
                                    const match::MatchParticipantRegistry& registry,
                                    bool isMatchActive,
                                    results::MatchOutcome& outOutcome,
                                    TeamId& outWinningTeamId,
                                    std::string& outWinningTeamName) const {
    outWinningTeamId = 0;
    outWinningTeamName = "No Winner (Draw)";

    if (!isMatchActive) {
        outOutcome = results::MatchOutcome::InProgress;
        return false;
    }

    if (teamManager.GetTotalTeamCount() == 0) {
        outOutcome = results::MatchOutcome::Failed;
        return true;
    }

    auto aliveTeamIds = teamManager.GetAliveTeamIds(registry);
    size_t aliveCount = aliveTeamIds.size();

    if (aliveCount == 1) {
        outOutcome = results::MatchOutcome::Victory;
        outWinningTeamId = aliveTeamIds[0];
        const auto* team = teamManager.FindTeam(outWinningTeamId);
        if (team) {
            outWinningTeamName = team->name;
        } else {
            outWinningTeamName = "Unknown Winning Team";
        }
        return true;
    }

    if (aliveCount == 0) {
        outOutcome = results::MatchOutcome::Draw;
        outWinningTeamId = 0;
        outWinningTeamName = "No Winner (Draw)";
        return true;
    }

    outOutcome = results::MatchOutcome::InProgress;
    return false;
}

} // namespace wildbound::gamemodes

