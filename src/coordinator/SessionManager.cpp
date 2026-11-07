#include "SessionManager.hpp"
#include <algorithm>

namespace wildbound::coordinator {

SessionId SessionManager::GenerateSessionId() noexcept {
    return m_nextSessionId++;
}

SessionId SessionManager::CreateSession(network::ClientId connectionId, const std::string& displayName, float currentTime) {
    // If connection already has a session, remove old one first
    auto it = m_connectionToSession.find(connectionId);
    if (it != m_connectionToSession.end()) {
        m_sessions.erase(it->second);
        m_connectionToSession.erase(it);
    }

    SessionId newId = GenerateSessionId();
    PlayerSession session{
        .sessionId = newId,
        .connectionId = connectionId,
        .displayName = displayName.empty() ? ("Player_" + std::to_string(newId)) : displayName,
        .state = SessionState::Connected,
        .partyId = INVALID_PARTY_ID,
        .lobbyId = INVALID_LOBBY_ID,
        .currentMatchId = "",
        .lastHeartbeatTime = currentTime,
        .sessionStartTime = currentTime
    };

    m_sessions[newId] = std::move(session);
    m_connectionToSession[connectionId] = newId;
    return newId;
}

PlayerSession* SessionManager::GetSession(SessionId id) noexcept {
    auto it = m_sessions.find(id);
    return (it != m_sessions.end()) ? &it->second : nullptr;
}

const PlayerSession* SessionManager::GetSession(SessionId id) const noexcept {
    auto it = m_sessions.find(id);
    return (it != m_sessions.end()) ? &it->second : nullptr;
}

PlayerSession* SessionManager::GetSessionByConnection(network::ClientId connectionId) noexcept {
    auto it = m_connectionToSession.find(connectionId);
    if (it == m_connectionToSession.end()) return nullptr;
    return GetSession(it->second);
}

const PlayerSession* SessionManager::GetSessionByConnection(network::ClientId connectionId) const noexcept {
    auto it = m_connectionToSession.find(connectionId);
    if (it == m_connectionToSession.end()) return nullptr;
    return GetSession(it->second);
}

PlayerSession* SessionManager::GetSessionByName(const std::string& name) noexcept {
    for (auto& [id, session] : m_sessions) {
        if (session.displayName == name && session.IsActive()) {
            return &session;
        }
    }
    return nullptr;
}

bool SessionManager::UpdateHeartbeat(SessionId id, float currentTime) {
    auto* s = GetSession(id);
    if (!s) return false;
    s->lastHeartbeatTime = currentTime;
    return true;
}

bool SessionManager::SetSessionState(SessionId id, SessionState state) {
    auto* s = GetSession(id);
    if (!s) return false;
    s->state = state;
    return true;
}

bool SessionManager::SetSessionParty(SessionId id, PartyId partyId) {
    auto* s = GetSession(id);
    if (!s) return false;
    s->partyId = partyId;
    if (partyId != INVALID_PARTY_ID && s->state == SessionState::Connected) {
        s->state = SessionState::InParty;
    } else if (partyId == INVALID_PARTY_ID && s->state == SessionState::InParty) {
        s->state = SessionState::Connected;
    }
    return true;
}

bool SessionManager::SetSessionLobby(SessionId id, LobbyId lobbyId) {
    auto* s = GetSession(id);
    if (!s) return false;
    s->lobbyId = lobbyId;
    return true;
}

bool SessionManager::SetSessionMatch(SessionId id, const std::string& matchId) {
    auto* s = GetSession(id);
    if (!s) return false;
    s->currentMatchId = matchId;
    if (!matchId.empty()) {
        s->state = SessionState::InMatch;
    }
    return true;
}

bool SessionManager::RemoveSession(SessionId id) {
    auto it = m_sessions.find(id);
    if (it == m_sessions.end()) return false;
    m_connectionToSession.erase(it->second.connectionId);
    m_sessions.erase(it);
    return true;
}

bool SessionManager::RemoveSessionByConnection(network::ClientId connectionId) {
    auto it = m_connectionToSession.find(connectionId);
    if (it == m_connectionToSession.end()) return false;
    SessionId sid = it->second;
    m_connectionToSession.erase(it);
    m_sessions.erase(sid);
    return true;
}

void SessionManager::CheckTimeouts(float currentTime, float timeoutSeconds, std::vector<SessionId>& outTimedOut) {
    outTimedOut.clear();
    for (auto& [id, s] : m_sessions) {
        if (s.state != SessionState::Disconnected) {
            if ((currentTime - s.lastHeartbeatTime) > timeoutSeconds) {
                outTimedOut.push_back(id);
            }
        }
    }
    for (SessionId sid : outTimedOut) {
        RemoveSession(sid);
    }
}

size_t SessionManager::GetActiveSessionCount() const noexcept {
    size_t count = 0;
    for (const auto& [id, s] : m_sessions) {
        if (s.IsActive()) ++count;
    }
    return count;
}

void SessionManager::Clear() {
    m_sessions.clear();
    m_connectionToSession.clear();
}

} // namespace wildbound::coordinator

