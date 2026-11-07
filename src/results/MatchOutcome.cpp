#include "MatchOutcome.hpp"

namespace wildbound::results {

const char* MatchOutcomeToString(MatchOutcome outcome) noexcept {
    switch (outcome) {
        case MatchOutcome::InProgress: return "InProgress";
        case MatchOutcome::Victory:    return "Victory";
        case MatchOutcome::Draw:       return "Draw";
        case MatchOutcome::Cancelled:  return "Cancelled";
        case MatchOutcome::Failed:     return "Failed";
        default:                       return "Unknown";
    }
}

const char* PlayerOutcomeToString(PlayerOutcome outcome) noexcept {
    switch (outcome) {
        case PlayerOutcome::Undecided: return "Undecided";
        case PlayerOutcome::Victory:   return "Victory";
        case PlayerOutcome::Defeat:    return "Defeat";
        case PlayerOutcome::Draw:      return "Draw";
        default:                       return "Unknown";
    }
}

} // namespace wildbound::results

