#pragma once

#include "match/MatchParticipant.hpp"
#include <cstdint>
#include <string>

namespace wildbound::results {

/**
 * Immutable audit record of an authoritative participant elimination event.
 */
struct EliminationRecord {
    uint32_t eventId = 0;
    uint32_t victimParticipantId = 0;
    std::string victimName;
    uint32_t killerParticipantId = 0;
    std::string killerName;
    match::EliminationCause cause = match::EliminationCause::Unknown;
    float matchElapsedTime = 0.0f;
    uint32_t placement = 0;
};

} // namespace wildbound::results

