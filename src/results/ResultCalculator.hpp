#pragma once

#include "MatchResultModel.hpp"
#include <string>
#include <vector>

namespace wildbound::match {
class MatchParticipantRegistry;
class MatchStatistics;
struct MatchConfiguration;
}

namespace wildbound::results {

class VictoryEvaluator;
class DefeatEvaluator;

/**
 * Authoritative Result, Placement, and Duration Calculator.
 * Produces finalized, immutable post-match telemetry snapshots.
 */
class ResultCalculator {
public:
    ResultCalculator() = default;
    ~ResultCalculator() = default;

    /**
     * Formats duration in seconds to standard MM:SS or HH:MM:SS.
     */
    [[nodiscard]] static std::string FormatDuration(float seconds) noexcept;

    /**
     * Assigns authoritative placements to all participants in the registry.
     * Enforces the tournament tie policy for simultaneous eliminations.
     */
    void CalculatePlacements(match::MatchParticipantRegistry& registry,
                            uint32_t winnerId,
                            MatchOutcome outcome) const;

    /**
     * Constructs a finalized, immutable MatchResultModel snapshot.
     */
    [[nodiscard]] MatchResultModel CalculateFinalResults(
        const std::string& matchId,
        float matchDuration,
        uint32_t localParticipantId,
        match::MatchParticipantRegistry& registry,
        const match::MatchStatistics& statistics,
        const std::vector<EliminationRecord>& eliminationRecords,
        const VictoryEvaluator& victoryEvaluator,
        const DefeatEvaluator& defeatEvaluator
    ) const;
};

} // namespace wildbound::results
