#pragma once

#include "MatchParticipant.hpp"
#include <string>
#include <vector>

namespace wildbound::match {

/**
 * Final Authoritative Match Results and Leaderboard.
 */
struct MatchResult {
    bool hasResult = false;
    bool isVictory = false;
    uint32_t winnerParticipantId = 0;
    std::string winnerDisplayName = "No Winner (Draw)";
    uint32_t localPlayerPlacement = 0;
    uint32_t totalParticipants = 0;
    float matchDuration = 0.0f;
    uint32_t localPlayerKills = 0;
    float localPlayerDamageDealt = 0.0f;
    float localPlayerDamageReceived = 0.0f;
    float localPlayerSurvivalDuration = 0.0f;
    EliminationCause localPlayerEliminationCause = EliminationCause::Unknown;
    std::string killerDisplayName;
    std::vector<MatchParticipant> leaderboard;

    void Reset() {
        hasResult = false;
        isVictory = false;
        winnerParticipantId = 0;
        winnerDisplayName = "No Winner (Draw)";
        localPlayerPlacement = 0;
        totalParticipants = 0;
        matchDuration = 0.0f;
        localPlayerKills = 0;
        localPlayerDamageDealt = 0.0f;
        localPlayerDamageReceived = 0.0f;
        localPlayerSurvivalDuration = 0.0f;
        localPlayerEliminationCause = EliminationCause::Unknown;
        killerDisplayName.clear();
        leaderboard.clear();
    }
};

} // namespace wildbound::match

