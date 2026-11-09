#pragma once

#include <string>
#include <cstdint>

namespace wildbound::network::matchmaking {

/**
 * Matchmaking and coordinator states representing the client-side lifecycle.
 */
enum class MatchmakingState {
    Idle = 0,
    Connecting,
    RegisteringSession,
    InLobby,
    RequestingMatch,
    Queued,
    MatchFound,
    Cancelling,
    Cancelled,
    Failed
};

[[nodiscard]] inline const char* MatchmakingStateToString(MatchmakingState state) noexcept {
    switch (state) {
        case MatchmakingState::Idle: return "IDLE";
        case MatchmakingState::Connecting: return "CONNECTING";
        case MatchmakingState::RegisteringSession: return "REGISTERING_SESSION";
        case MatchmakingState::InLobby: return "IN_LOBBY";
        case MatchmakingState::RequestingMatch: return "REQUESTING_MATCH";
        case MatchmakingState::Queued: return "QUEUED";
        case MatchmakingState::MatchFound: return "MATCH_FOUND";
        case MatchmakingState::Cancelling: return "CANCELLING";
        case MatchmakingState::Cancelled: return "CANCELLED";
        case MatchmakingState::Failed: return "FAILED";
        default: return "UNKNOWN";
    }
}

/**
 * Authoritative match assignment payload received from the Matchmaking Coordinator.
 * Safely validated before being made available for Stage 6C gameplay join.
 */
struct MatchAssignmentInfo {
    std::string matchId;
    std::string serverHost;
    uint16_t serverPort{ 0 };
    std::string playerAuthToken;
    std::string gameMode{ "solo" };
    std::string mapName;
    uint64_t assignedTimestamp{ 0 };

    [[nodiscard]] bool IsValid() const noexcept {
        return !matchId.empty() &&
               !serverHost.empty() &&
               serverPort > 0 &&
               !playerAuthToken.empty() &&
               (gameMode == "solo" || gameMode == "duos" || gameMode == "squads") &&
               !mapName.empty();
    }
};

} // namespace wildbound::network::matchmaking

