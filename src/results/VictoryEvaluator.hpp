#pragma once

#include "MatchOutcome.hpp"
#include <string>
#include <cstdint>

namespace wildbound::match {
class MatchParticipantRegistry;
}

namespace wildbound::results {

/**
 * Authoritative Victory Condition Evaluator.
 * Evaluates Last Survivor Standing, mutual elimination draws, and incomplete match states.
 */
class VictoryEvaluator {
public:
    VictoryEvaluator() = default;
    ~VictoryEvaluator() = default;

    /**
     * Evaluates the current participant registry for victory or draw conditions.
     * @param registry Authoritative participant registry.
     * @param isMatchActive Whether the match is currently in an active state.
     * @param outOutcome Outputs MatchOutcome (Victory, Draw, InProgress, etc.).
     * @param outWinnerId Outputs winner participant ID (0 if draw/none).
     * @param outWinnerName Outputs winner display name.
     * @return True if a decisive end-of-match condition (Victory or Draw) was reached.
     */
    bool Evaluate(const match::MatchParticipantRegistry& registry,
                  bool isMatchActive,
                  MatchOutcome& outOutcome,
                  uint32_t& outWinnerId,
                  std::string& outWinnerName) const;
};

} // namespace wildbound::results

