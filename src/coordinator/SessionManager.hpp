#pragma once

#include "PlayerSession.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

namespace wildbound::coordinator {

class SessionManager {
public:
    SessionManager() = default;
    ~SessionManager() = default;

    SessionId CreateSession(network::ClientId connectionId, const std::string& displayName, float currentTime = 0.0f);
    [[nodiscard]] PlayerSession* GetSession(SessionId id) noexcept;
    [[nodiscard]] const PlayerSession* GetSession(SessionId id) const noexcept;
    [[nodiscard]] PlayerSession* GetSessionByConnection(network::ClientId connectionId) noexcept;
    [[nodiscard]] const PlayerSession* GetSessionByConnection(network::ClientId connectionId) const noexcept;
    [[nodiscard]] PlayerSession* GetSessionByName(const std::string& name) noexcept;

    bool UpdateHeartbeat(SessionId id, float currentTime);
    bool SetSessionState(SessionId id, SessionState state);
    bool SetSessionParty(SessionId id, PartyId partyId);
    bool SetSessionLobby(SessionId id, LobbyId lobbyId);
    bool SetSessionMatch(SessionId id, const std::string& matchId);

    bool RemoveSession(SessionId id);
    bool RemoveSessionByConnection(network::ClientId connectionId);

    void CheckTimeouts(float currentTime, float timeoutSeconds, std::vector<SessionId>& outTimedOut);

    [[nodiscard]] size_t GetActiveSessionCount() const noexcept;
    [[nodiscard]] const std::unordered_map<SessionId, PlayerSession>& GetAllSessions() const noexcept { return m_sessions; }

    void Clear();

private:
    SessionId GenerateSessionId() noexcept;

    std::unordered_map<SessionId, PlayerSession> m_sessions;
    std::unordered_map<network::ClientId, SessionId> m_connectionToSession;
    SessionId m_nextSessionId{ 10001 };
};

} // namespace wildbound::coordinator
