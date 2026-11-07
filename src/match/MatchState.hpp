#pragma once

#include <string>

namespace wildbound::match {

/**
 * Formal Battle Royale Match State Machine Enumeration.
 */
enum class MatchState {
    Uninitialized,       // No match exists / idle in lobby/menu
    Initializing,        // Loading world, validating dependencies, setting up match systems
    WaitingForPlayers,   // Participants registered and validated, waiting for countdown trigger
    Countdown,           // Pre-match countdown timer active; gameplay actions restricted
    Starting,            // Final operations before match start; preparing storm & controls
    Active,              // Active match in progress; combat, storm progression, eliminations active
    Finishing,           // Final survivor detected or end condition met; computing results
    Completed,           // Results screen displayed; match ended
    Cleanup,             // Releasing match resources, resetting state
    Failed               // Unrecoverable initialization or runtime failure
};

[[nodiscard]] const char* MatchStateToString(MatchState state) noexcept;
[[nodiscard]] bool IsValidStateTransition(MatchState from, MatchState to) noexcept;

} // namespace wildbound::match

