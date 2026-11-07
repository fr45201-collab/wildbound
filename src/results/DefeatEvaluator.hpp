#pragma once

#include "MatchOutcome.hpp"
#include "PlayerPerformance.hpp"
#include <string>

namespace wildbound::match {
struct MatchParticipant;
}

namespace wildbound::results {

/**
 * Authoritative Defeat Evaluator for individual competitors.
 */
class DefeatEvaluator {
public:
    DefeatEvaluator() = default;
    ~DefeatEvaluator() = default;

    /**
     * Evaluates a participant's state and determines their personal outcome.
     */
    [[nodiscard]] PlayerOutcome EvaluateParticipantOutcome(const match::MatchParticipant& participant,
                                                           MatchOutcome overallMatchOutcome,
                                                           uint32_t winnerParticipantId) const;

    /**
     * Formats a human-readable defeat summary description.
     */
    [[nodiscard]] std::string FormatDefeatReason(const match::MatchParticipant& participant) const;
};

} // namespace wildbound::results

