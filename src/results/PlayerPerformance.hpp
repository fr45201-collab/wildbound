#pragma once

#include "MatchOutcome.hpp"
#include "match/MatchParticipant.hpp"
#include <cstdint>
#include <string>

namespace wildbound::results {

/**
 * Detailed match-scoped telemetry and performance breakdown for a single participant.
 */
struct PlayerPerformance {
    uint32_t participantId = 0;
    std::string displayName;
    match::ParticipantType type = match::ParticipantType::LocalHuman;
    PlayerOutcome outcome = PlayerOutcome::Undecided;
    uint32_t placement = 0;
    float survivalDuration = 0.0f;
    uint32_t kills = 0;
    uint32_t deaths = 0;
    float damageDealt = 0.0f;
    float combatDamageReceived = 0.0f;
    float stormDamageReceived = 0.0f;
    float totalDamageReceived = 0.0f;
    match::EliminationCause eliminationCause = match::EliminationCause::Unknown;
    uint32_t killerParticipantId = 0;
    std::string killerDisplayName;
    bool isWinner = false;
};

} // namespace wildbound::results

