#include "MatchEliminationSystem.hpp"
#include "MatchParticipantRegistry.hpp"
#include "MatchStatistics.hpp"

namespace wildbound::match {

bool MatchEliminationSystem::ProcessElimination(uint32_t victimParticipantId,
                                                EliminationCause cause,
                                                uint32_t killerParticipantId,
                                                float matchTime,
                                                MatchParticipantRegistry& registry,
                                                MatchStatistics& stats) {
    if (victimParticipantId == 0) {
        return false;
    }

    MatchParticipant* victim = registry.FindParticipant(victimParticipantId);
    if (!victim) {
        return false;
    }

    // Idempotent: Ignore if already eliminated
    if (victim->status == ParticipantStatus::Eliminated) {
        return false;
    }

    victim->status = ParticipantStatus::Eliminated;
    victim->survivalDuration = matchTime;
    victim->eliminationInfo.timestamp = matchTime;
    victim->eliminationInfo.cause = cause;
    victim->eliminationInfo.killerParticipantId = killerParticipantId;

    if (killerParticipantId != 0) {
        if (const auto* killer = registry.FindParticipant(killerParticipantId)) {
            victim->eliminationInfo.killerName = killer->displayName;
        }
    } else if (cause == EliminationCause::CorruptionStorm) {
        victim->eliminationInfo.killerName = "Corruption Storm";
    }

    // Update match statistics
    stats.RecordElimination(victimParticipantId, killerParticipantId, registry);

    // Notify listeners
    for (const auto& cb : m_callbacks) {
        if (cb) {
            cb(*victim, victim->eliminationInfo);
        }
    }

    return true;
}

} // namespace wildbound::match

