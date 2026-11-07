#include "MatchResultManager.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "match/MatchStatistics.hpp"
#include "core/Logger.hpp"

namespace wildbound::results {

MatchResultManager::MatchResultManager() {
    Reset();
}

void MatchResultManager::Reset() {
    m_eliminationRecords.clear();
    m_finalResult.Reset();
    m_nextEliminationEventId = 1;
}

void MatchResultManager::RecordEliminationEvent(const match::MatchParticipant& victim,
                                               const match::EliminationInfo& info,
                                               float matchTime) {
    if (m_finalResult.isFinalized) {
        core::Logger::Warn("RESULTS", "Attempted to record elimination after match result finalization.");
        return;
    }

    EliminationRecord rec;
    rec.eventId = m_nextEliminationEventId++;
    rec.victimParticipantId = victim.participantId;
    rec.victimName = victim.displayName;
    rec.killerParticipantId = info.killerParticipantId;
    rec.killerName = info.killerName;
    rec.cause = info.cause;
    rec.matchElapsedTime = matchTime;
    rec.placement = 0; // Updated upon finalization

    m_eliminationRecords.push_back(rec);

    core::Logger::Info("RESULTS", "Elimination #" + std::to_string(rec.eventId) + ": [" +
                                  rec.victimName + "] eliminated by [" +
                                  (rec.killerName.empty() ? "World/Storm" : rec.killerName) +
                                  "] at " + ResultCalculator::FormatDuration(matchTime));
}

bool MatchResultManager::FinalizeMatchResult(const std::string& matchId,
                                            float matchDuration,
                                            uint32_t localParticipantId,
                                            match::MatchParticipantRegistry& registry,
                                            const match::MatchStatistics& statistics,
                                            bool autoSave) {
    if (m_finalResult.isFinalized) {
        core::Logger::Warn("RESULTS", "Match result has already been finalized. Finalization is immutable.");
        return false;
    }

    core::Logger::Info("RESULTS", "Finalizing match results for match: " + matchId);

    // Compute immutable results model
    m_finalResult = m_resultCalculator.CalculateFinalResults(
        matchId,
        matchDuration,
        localParticipantId,
        registry,
        statistics,
        m_eliminationRecords,
        m_victoryEvaluator,
        m_defeatEvaluator
    );

    // Update placements in elimination records from finalized registry
    for (auto& elim : m_eliminationRecords) {
        if (const auto* p = registry.FindParticipant(elim.victimParticipantId)) {
            elim.placement = p->placement;
        }
    }
    m_finalResult.eliminationRecords = m_eliminationRecords;

    core::Logger::Info("RESULTS", "Match finalized. Outcome: [" +
                                  std::string(MatchOutcomeToString(m_finalResult.matchOutcome)) +
                                  "], Local Player: [" +
                                  std::string(PlayerOutcomeToString(m_finalResult.localPlayerOutcome)) +
                                  "] (Rank #" + std::to_string(m_finalResult.localPlayerPerformance.placement) + ")");

    if (autoSave) {
        m_resultSerializer.SaveResult(m_finalResult);
    }

    return true;
}

} // namespace wildbound::results

