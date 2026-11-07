#pragma once

#include "MatchOutcome.hpp"
#include "PlayerPerformance.hpp"
#include "EliminationRecord.hpp"
#include <string>
#include <vector>

namespace wildbound::results {

/**
 * Immutable snapshot of finalized match results, telemetry, and leaderboard.
 */
struct MatchResultModel {
    std::string matchId;
    MatchOutcome matchOutcome = MatchOutcome::InProgress;
    PlayerOutcome localPlayerOutcome = PlayerOutcome::Undecided;
    uint32_t winnerParticipantId = 0;
    std::string winnerDisplayName = "No Winner (Draw)";
    float matchDuration = 0.0f;
    uint32_t totalParticipants = 0;
    uint32_t totalEliminations = 0;
    uint32_t survivorsCount = 0;
    std::string completionReason;

    // Local participant performance breakdown
    PlayerPerformance localPlayerPerformance;

    // Authoritative sorted leaderboard (Ranks 1..N)
    std::vector<PlayerPerformance> participantPerformances;

    // Chronological event log of all eliminations
    std::vector<EliminationRecord> eliminationRecords;

    bool isFinalized = false;

    void Reset() {
        matchId.clear();
        matchOutcome = MatchOutcome::InProgress;
        localPlayerOutcome = PlayerOutcome::Undecided;
        winnerParticipantId = 0;
        winnerDisplayName = "No Winner (Draw)";
        matchDuration = 0.0f;
        totalParticipants = 0;
        totalEliminations = 0;
        survivorsCount = 0;
        completionReason.clear();
        localPlayerPerformance = PlayerPerformance{};
        participantPerformances.clear();
        eliminationRecords.clear();
        isFinalized = false;
    }
};

} // namespace wildbound::results

