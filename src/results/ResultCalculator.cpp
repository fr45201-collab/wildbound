#include "ResultCalculator.hpp"
#include "VictoryEvaluator.hpp"
#include "DefeatEvaluator.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "match/MatchStatistics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace wildbound::results {

std::string ResultCalculator::FormatDuration(float seconds) noexcept {
    if (seconds < 0.0f) {
        seconds = 0.0f;
    }

    uint32_t totalSecs = static_cast<uint32_t>(seconds);
    uint32_t hours = totalSecs / 3600;
    uint32_t mins = (totalSecs % 3600) / 60;
    uint32_t secs = totalSecs % 60;

    char buf[32];
    if (hours > 0) {
        std::snprintf(buf, sizeof(buf), "%02u:%02u:%02u", hours, mins, secs);
    } else {
        std::snprintf(buf, sizeof(buf), "%02u:%02u", mins, secs);
    }
    return std::string(buf);
}

void ResultCalculator::CalculatePlacements(match::MatchParticipantRegistry& registry,
                                         uint32_t winnerId,
                                         MatchOutcome outcome) const {
    (void)outcome;
    auto& allParticipants = registry.GetAllParticipants();

    // Reset placements
    for (auto& p : allParticipants) {
        p.isWinner = (winnerId != 0 && p.participantId == winnerId);
        p.placement = 0;
    }

    if (winnerId != 0) {
        if (auto* winner = registry.FindParticipant(winnerId)) {
            winner->placement = 1;
            winner->isWinner = true;
        }
    }

    // Collect non-winners for placement ranking
    std::vector<match::MatchParticipant*> eliminatedList;
    eliminatedList.reserve(allParticipants.size());

    for (auto& p : allParticipants) {
        if (!p.isWinner) {
            eliminatedList.push_back(&p);
        }
    }

    // Sort descending by elimination timestamp (survived longer = better rank)
    std::sort(eliminatedList.begin(), eliminatedList.end(),
              [](const match::MatchParticipant* a, const match::MatchParticipant* b) {
                  return a->eliminationInfo.timestamp > b->eliminationInfo.timestamp;
              });

    // Standard tournament tie resolution
    uint32_t currentRank = (winnerId != 0) ? 2 : 1;
    for (size_t i = 0; i < eliminatedList.size(); ) {
        size_t j = i + 1;
        while (j < eliminatedList.size() &&
               std::abs(eliminatedList[j]->eliminationInfo.timestamp - eliminatedList[i]->eliminationInfo.timestamp) < 0.001f) {
            ++j;
        }

        // Assign current rank to all tied participants
        for (size_t k = i; k < j; ++k) {
            eliminatedList[k]->placement = currentRank;
        }

        // Advance current rank by the size of the tied group
        currentRank += static_cast<uint32_t>(j - i);
        i = j;
    }
}

MatchResultModel ResultCalculator::CalculateFinalResults(
    const std::string& matchId,
    float matchDuration,
    uint32_t localParticipantId,
    match::MatchParticipantRegistry& registry,
    const match::MatchStatistics& statistics,
    const std::vector<EliminationRecord>& eliminationRecords,
    const VictoryEvaluator& victoryEvaluator,
    const DefeatEvaluator& defeatEvaluator
) const {
    MatchResultModel model;
    model.matchId = matchId;
    model.matchDuration = matchDuration;
    model.totalParticipants = static_cast<uint32_t>(registry.GetTotalCount());
    model.totalEliminations = statistics.eliminatedParticipants;
    model.survivorsCount = static_cast<uint32_t>(registry.GetAliveCount());
    model.eliminationRecords = eliminationRecords;

    // Evaluate decisive outcome
    MatchOutcome outcome = MatchOutcome::InProgress;
    uint32_t winnerId = 0;
    std::string winnerName;
    victoryEvaluator.Evaluate(registry, true, outcome, winnerId, winnerName);

    model.matchOutcome = outcome;
    model.winnerParticipantId = winnerId;
    model.winnerDisplayName = winnerName;

    // Calculate placements on registry
    CalculatePlacements(registry, winnerId, outcome);

    // Build participant performance list
    const auto& participants = registry.GetAllParticipants();
    model.participantPerformances.reserve(participants.size());

    for (const auto& p : participants) {
        PlayerPerformance perf;
        perf.participantId = p.participantId;
        perf.displayName = p.displayName;
        perf.type = p.type;
        perf.outcome = defeatEvaluator.EvaluateParticipantOutcome(p, outcome, winnerId);
        perf.placement = p.placement;
        perf.survivalDuration = p.survivalDuration;
        perf.kills = p.kills;
        perf.deaths = (p.status == match::ParticipantStatus::Eliminated) ? 1 : 0;
        perf.damageDealt = p.damageDealt;
        perf.combatDamageReceived = p.combatDamageReceived;
        perf.stormDamageReceived = p.stormDamageReceived;
        perf.totalDamageReceived = p.damageReceived;
        perf.eliminationCause = p.eliminationInfo.cause;
        perf.killerParticipantId = p.eliminationInfo.killerParticipantId;
        perf.killerDisplayName = p.eliminationInfo.killerName;
        perf.isWinner = p.isWinner;

        if (p.participantId == localParticipantId) {
            model.localPlayerPerformance = perf;
            model.localPlayerOutcome = perf.outcome;
        }

        model.participantPerformances.push_back(perf);
    }

    // Sort leaderboard strictly by placement ascending (Rank 1, 2, 3...)
    std::sort(model.participantPerformances.begin(), model.participantPerformances.end(),
              [](const PlayerPerformance& a, const PlayerPerformance& b) {
                  return a.placement < b.placement;
              });

    if (outcome == MatchOutcome::Victory) {
        model.completionReason = "Sole survivor emerged victorious.";
    } else if (outcome == MatchOutcome::Draw) {
        model.completionReason = "All competitors were eliminated simultaneously.";
    } else {
        model.completionReason = "Match completed.";
    }

    model.isFinalized = true;
    return model;
}

} // namespace wildbound::results
