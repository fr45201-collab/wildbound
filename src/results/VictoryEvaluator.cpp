#include "VictoryEvaluator.hpp"
#include "match/MatchParticipantRegistry.hpp"

namespace wildbound::results {

bool VictoryEvaluator::Evaluate(const match::MatchParticipantRegistry& registry,
                                bool isMatchActive,
                                MatchOutcome& outOutcome,
                                uint32_t& outWinnerId,
                                std::string& outWinnerName) const {
    outWinnerId = 0;
    outWinnerName = "No Winner (Draw)";

    if (!isMatchActive) {
        outOutcome = MatchOutcome::InProgress;
        return false;
    }

    size_t totalCount = registry.GetTotalCount();
    if (totalCount == 0) {
        outOutcome = MatchOutcome::Failed;
        return true;
    }

    auto aliveIds = registry.GetAliveParticipantIds();
    size_t aliveCount = aliveIds.size();

    if (aliveCount == 1) {
        outOutcome = MatchOutcome::Victory;
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
        outOutcome = MatchOutcome::Draw;
        outWinnerId = 0;
        outWinnerName = "No Winner (Draw)";
        return true;
    }

    outOutcome = MatchOutcome::InProgress;
    return false;
}

} // namespace wildbound::results

