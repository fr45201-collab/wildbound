#pragma once

#include "MatchResultModel.hpp"
#include "VictoryEvaluator.hpp"
#include "DefeatEvaluator.hpp"
#include "ResultCalculator.hpp"
#include "ResultSerializer.hpp"

#include <vector>
#include <string>

namespace wildbound::match {
class MatchParticipantRegistry;
class MatchStatistics;
}

namespace wildbound::results {

/**
 * Authoritative Victory & Match Results Manager.
 * Extends Phase 32 Match Lifecycle with result calculation,
 * elimination logging, statistics aggregation, and local persistence.
 */
class MatchResultManager {
public:
    MatchResultManager();
    ~MatchResultManager() = default;

    /**
     * Resets all result tracking data for a new match context.
     */
    void Reset();

    /**
     * Records an authoritative elimination event into the chronological log.
     */
    void RecordEliminationEvent(const match::MatchParticipant& victim,
                                const match::EliminationInfo& info,
                                float matchTime);

    /**
     * Finalizes the match result into an immutable snapshot.
     * Guaranteed to execute at most once per match lifecycle.
     */
    bool FinalizeMatchResult(const std::string& matchId,
                             float matchDuration,
                             uint32_t localParticipantId,
                             match::MatchParticipantRegistry& registry,
                             const match::MatchStatistics& statistics,
                             bool autoSave = true);

    [[nodiscard]] bool IsFinalized() const noexcept { return m_finalResult.isFinalized; }
    [[nodiscard]] const MatchResultModel& GetFinalResult() const noexcept { return m_finalResult; }
    [[nodiscard]] const std::vector<EliminationRecord>& GetEliminationRecords() const noexcept { return m_eliminationRecords; }

    [[nodiscard]] const VictoryEvaluator& GetVictoryEvaluator() const noexcept { return m_victoryEvaluator; }
    [[nodiscard]] const DefeatEvaluator& GetDefeatEvaluator() const noexcept { return m_defeatEvaluator; }
    [[nodiscard]] const ResultCalculator& GetResultCalculator() const noexcept { return m_resultCalculator; }
    [[nodiscard]] const ResultSerializer& GetResultSerializer() const noexcept { return m_resultSerializer; }

private:
    VictoryEvaluator m_victoryEvaluator;
    DefeatEvaluator m_defeatEvaluator;
    ResultCalculator m_resultCalculator;
    ResultSerializer m_resultSerializer;

    std::vector<EliminationRecord> m_eliminationRecords;
    MatchResultModel m_finalResult;
    uint32_t m_nextEliminationEventId = 1;
};

} // namespace wildbound::results

