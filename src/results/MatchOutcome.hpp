#pragma once

namespace wildbound::results {

/**
 * High-level authoritative match completion outcome.
 */
enum class MatchOutcome {
    InProgress,
    Victory,
    Draw,
    Cancelled,
    Failed
};

/**
 * Individual participant match outcome.
 */
enum class PlayerOutcome {
    Undecided,
    Victory,
    Defeat,
    Draw
};

[[nodiscard]] const char* MatchOutcomeToString(MatchOutcome outcome) noexcept;
[[nodiscard]] const char* PlayerOutcomeToString(PlayerOutcome outcome) noexcept;

} // namespace wildbound::results

