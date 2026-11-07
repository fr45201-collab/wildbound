#pragma once

#include "CoordinatorProtocol.hpp"
#include <vector>
#include <string>
#include <algorithm>

namespace wildbound::coordinator {

struct LobbyMember {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string displayName;
    bool isReady{ false };
};

struct Lobby {
    LobbyId lobbyId{ INVALID_LOBBY_ID };
    SessionId hostSessionId{ INVALID_SESSION_ID };
    std::string lobbyName;
    std::string gameMode{ "solo" };
    std::string mapName{ "maps/world/battle_royale_island.json" };
    uint32_t maxCapacity{ 32 };
    bool isPrivate{ false };
    std::string joinCode;
    std::vector<LobbyMember> members;

    [[nodiscard]] size_t GetMemberCount() const noexcept { return members.size(); }
    [[nodiscard]] bool IsFull() const noexcept { return members.size() >= maxCapacity; }

    [[nodiscard]] bool HasMember(SessionId sid) const noexcept {
        return std::any_of(members.begin(), members.end(), [sid](const LobbyMember& m) {
            return m.sessionId == sid;
        });
    }

    [[nodiscard]] bool IsHost(SessionId sid) const noexcept {
        return hostSessionId == sid;
    }
};

} // namespace wildbound::coordinator

