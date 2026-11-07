#pragma once

#include "Lobby.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

class LobbyManager {
public:
    LobbyManager() = default;
    ~LobbyManager() = default;

    LobbyId CreateLobby(SessionId hostId, const std::string& hostName,
                        const std::string& lobbyName, uint32_t maxCapacity = 32,
                        bool isPrivate = false, const std::string& joinCode = "");

    [[nodiscard]] Lobby* GetLobby(LobbyId id) noexcept;
    [[nodiscard]] const Lobby* GetLobby(LobbyId id) const noexcept;
    [[nodiscard]] Lobby* GetLobbyBySession(SessionId sid) noexcept;
    [[nodiscard]] const Lobby* GetLobbyBySession(SessionId sid) const noexcept;

    bool AddMember(LobbyId lobbyId, SessionId sid, const std::string& displayName, const std::string& joinCode = "");
    bool RemoveMember(LobbyId lobbyId, SessionId sid, bool& outDisbanded, SessionId& outNewHost);
    bool SetMemberReady(LobbyId lobbyId, SessionId sid, bool isReady);
    bool DisbandLobby(LobbyId lobbyId);

    [[nodiscard]] std::vector<const Lobby*> GetPublicLobbies() const;
    [[nodiscard]] size_t GetLobbyCount() const noexcept { return m_lobbies.size(); }

    void Clear();

private:
    LobbyId GenerateLobbyId() noexcept;

    std::unordered_map<LobbyId, Lobby> m_lobbies;
    std::unordered_map<SessionId, LobbyId> m_sessionToLobby;
    LobbyId m_nextLobbyId{ 30001 };
};

} // namespace wildbound::coordinator

