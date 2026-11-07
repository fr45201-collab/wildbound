#include "MatchState.hpp"

namespace wildbound::match {

const char* MatchStateToString(MatchState state) noexcept {
    switch (state) {
        case MatchState::Uninitialized:     return "Uninitialized";
        case MatchState::Initializing:      return "Initializing";
        case MatchState::WaitingForPlayers:  return "WaitingForPlayers";
        case MatchState::Countdown:         return "Countdown";
        case MatchState::Starting:          return "Starting";
        case MatchState::Active:            return "Active";
        case MatchState::Finishing:         return "Finishing";
        case MatchState::Completed:         return "Completed";
        case MatchState::Cleanup:           return "Cleanup";
        case MatchState::Failed:            return "Failed";
        default:                            return "Unknown";
    }
}

bool IsValidStateTransition(MatchState from, MatchState to) noexcept {
    if (from == to) {
        return false; // No self-transitions
    }

    switch (from) {
        case MatchState::Uninitialized:
            return (to == MatchState::Initializing);

        case MatchState::Initializing:
            return (to == MatchState::WaitingForPlayers || to == MatchState::Failed);

        case MatchState::WaitingForPlayers:
            return (to == MatchState::Countdown || to == MatchState::Cleanup || to == MatchState::Failed);

        case MatchState::Countdown:
            return (to == MatchState::Starting || to == MatchState::Cleanup || to == MatchState::Failed);

        case MatchState::Starting:
            return (to == MatchState::Active || to == MatchState::Cleanup || to == MatchState::Failed);

        case MatchState::Active:
            return (to == MatchState::Finishing || to == MatchState::Cleanup || to == MatchState::Failed);

        case MatchState::Finishing:
            return (to == MatchState::Completed || to == MatchState::Cleanup || to == MatchState::Failed);

        case MatchState::Completed:
            return (to == MatchState::Cleanup);

        case MatchState::Cleanup:
            return (to == MatchState::Uninitialized);

        case MatchState::Failed:
            return (to == MatchState::Cleanup || to == MatchState::Uninitialized);

        default:
            return false;
    }
}

} // namespace wildbound::match

