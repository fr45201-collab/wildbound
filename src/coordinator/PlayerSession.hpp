#pragma once

#include "CoordinatorProtocol.hpp"
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

enum class SessionState : uint8_t {
    Connected,
    InParty,
    InQueue,
    InMatch,
    Disconnected
};

struct PlayerSession {
    SessionId sessionId{ INVALID_SESSION_ID };
    network::ClientId connectionId{ network::INVALID_CLIENT_ID };
    std::string displayName;
    std::string persistentUserId;
    std::string authToken;
    SessionState state{ SessionState::Connected };
    PartyId partyId{ INVALID_PARTY_ID };
    LobbyId lobbyId{ INVALID_LOBBY_ID };
    std::string currentMatchId;
    float lastHeartbeatTime{ 0.0f };
    float sessionStartTime{ 0.0f };

    [[nodiscard]] bool IsActive() const noexcept {
        return sessionId != INVALID_SESSION_ID && state != SessionState::Disconnected;
    }
};

} // namespace wildbound::coordinator

