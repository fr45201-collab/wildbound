#include "DefeatEvaluator.hpp"
#include "match/MatchParticipant.hpp"

namespace wildbound::results {

PlayerOutcome DefeatEvaluator::EvaluateParticipantOutcome(const match::MatchParticipant& participant,
                                                         MatchOutcome overallMatchOutcome,
                                                         uint32_t winnerParticipantId) const {
    if (overallMatchOutcome == MatchOutcome::Draw) {
        return PlayerOutcome::Draw;
    }

    if (overallMatchOutcome == MatchOutcome::Victory && participant.participantId == winnerParticipantId) {
        return PlayerOutcome::Victory;
    }

    if (participant.status == match::ParticipantStatus::Eliminated ||
        participant.status == match::ParticipantStatus::Disconnected) {
        return PlayerOutcome::Defeat;
    }

    return PlayerOutcome::Undecided;
}

std::string DefeatEvaluator::FormatDefeatReason(const match::MatchParticipant& participant) const {
    if (participant.status != match::ParticipantStatus::Eliminated) {
        return "Active";
    }

    switch (participant.eliminationInfo.cause) {
        case match::EliminationCause::Combat:
            if (!participant.eliminationInfo.killerName.empty()) {
                return "Eliminated by " + participant.eliminationInfo.killerName;
            }
            return "Eliminated in combat";

        case match::EliminationCause::CorruptionStorm:
            return "Consumed by Corruption Storm";

        case match::EliminationCause::Environment:
            return "Fell to environmental hazards";

        case match::EliminationCause::Disconnect:
            return "Disconnected from battle";

        case match::EliminationCause::Unknown:
        default:
            return "Eliminated in the wild";
    }
}

} // namespace wildbound::results

