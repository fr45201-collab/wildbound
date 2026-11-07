#include "MatchParticipant.hpp"

namespace wildbound::match {

const char* ParticipantTypeToString(ParticipantType type) noexcept {
    switch (type) {
        case ParticipantType::LocalHuman:   return "LocalHuman";
        case ParticipantType::RemoteHuman:  return "RemoteHuman";
        case ParticipantType::AI:           return "AI";
        default:                            return "Unknown";
    }
}

const char* ParticipantStatusToString(ParticipantStatus status) noexcept {
    switch (status) {
        case ParticipantStatus::Registered:    return "Registered";
        case ParticipantStatus::Alive:         return "Alive";
        case ParticipantStatus::Eliminated:    return "Eliminated";
        case ParticipantStatus::Disconnected:  return "Disconnected";
        default:                               return "Unknown";
    }
}

const char* EliminationCauseToString(EliminationCause cause) noexcept {
    switch (cause) {
        case EliminationCause::Combat:           return "Combat";
        case EliminationCause::CorruptionStorm:  return "CorruptionStorm";
        case EliminationCause::Environment:      return "Environment";
        case EliminationCause::Disconnect:       return "Disconnect";
        case EliminationCause::Unknown:          return "Unknown";
        default:                                 return "Unknown";
    }
}

} // namespace wildbound::match

