#include "LobbyManager.hpp"
#include <algorithm>

namespace wildbound::coordinator {

LobbyId LobbyManager::GenerateLobbyId() noexcept {
    return m_nextLobbyId++;
}

LobbyId LobbyManager::CreateLobby(SessionId hostId, const std::string& hostName,
                                  const std::string& lobbyName, uint32_t maxCapacity,
                                  bool isPrivate, const std::string& joinCode) {
    if (hostId == INVALID_SESSION_ID) return INVALID_LOBBY_ID;

    // Remove from existing lobby if any
    auto it = m_sessionToLobby.find(hostId);
    if (it != m_sessionToLobby.end()) {
        bool disbanded = false;
        SessionId newHost = INVALID_SESSION_ID;
        RemoveMember(it->second, hostId, disbanded, newHost);
    }

    LobbyId newId = GenerateLobbyId();
    Lobby lobby{
        .lobbyId = newId,
        .hostSessionId = hostId,
        .lobbyName = lobbyName.empty() ? ("Lobby #" + std::to_string(newId)) : lobbyName,
        .gameMode = "solo",
        .mapName = "maps/world/battle_royale_island.json",
        .maxCapacity = (maxCapacity > 0) ? maxCapacity : 32,
        .isPrivate = isPrivate,
        .joinCode = joinCode,
        .members = { LobbyMember{ hostId, hostName, true } }
    };

    m_lobbies[newId] = std::move(lobby);
    m_sessionToLobby[hostId] = newId;
    return newId;
}

Lobby* LobbyManager::GetLobby(LobbyId id) noexcept {
    auto it = m_lobbies.find(id);
    return (it != m_lobbies.end()) ? &it->second : nullptr;
}

const Lobby* LobbyManager::GetLobby(LobbyId id) const noexcept {
    auto it = m_lobbies.find(id);
    return (it != m_lobbies.end()) ? &it->second : nullptr;
}

Lobby* LobbyManager::GetLobbyBySession(SessionId sid) noexcept {
    auto it = m_sessionToLobby.find(sid);
    if (it == m_sessionToLobby.end()) return nullptr;
    return GetLobby(it->second);
}

const Lobby* LobbyManager::GetLobbyBySession(SessionId sid) const noexcept {
    auto it = m_sessionToLobby.find(sid);
    if (it == m_sessionToLobby.end()) return nullptr;
    return GetLobby(it->second);
}

bool LobbyManager::AddMember(LobbyId lobbyId, SessionId sid, const std::string& displayName, const std::string& joinCode) {
    auto* lobby = GetLobby(lobbyId);
    if (!lobby) return false;
    if (lobby->IsFull()) return false;
    if (lobby->HasMember(sid)) return false;

    if (lobby->isPrivate && lobby->joinCode != joinCode) {
        return false;
    }

    // Leave existing lobby if any
    auto it = m_sessionToLobby.find(sid);
    if (it != m_sessionToLobby.end()) {
        bool disbanded = false;
        SessionId newHost = INVALID_SESSION_ID;
        RemoveMember(it->second, sid, disbanded, newHost);
    }

    lobby->members.push_back(LobbyMember{
        .sessionId = sid,
        .displayName = displayName,
        .isReady = false
    });

    m_sessionToLobby[sid] = lobbyId;
    return true;
}

bool LobbyManager::RemoveMember(LobbyId lobbyId, SessionId sid, bool& outDisbanded, SessionId& outNewHost) {
    outDisbanded = false;
    outNewHost = INVALID_SESSION_ID;

    auto* lobby = GetLobby(lobbyId);
    if (!lobby) return false;

    auto it = std::find_if(lobby->members.begin(), lobby->members.end(), [sid](const LobbyMember& m) {
        return m.sessionId == sid;
    });

    if (it == lobby->members.end()) return false;

    lobby->members.erase(it);
    m_sessionToLobby.erase(sid);

    if (lobby->members.empty()) {
        m_lobbies.erase(lobbyId);
        outDisbanded = true;
        return true;
    }

    if (lobby->hostSessionId == sid) {
        lobby->hostSessionId = lobby->members.front().sessionId;
        lobby->members.front().isReady = true;
        outNewHost = lobby->hostSessionId;
    }

    return true;
}

bool LobbyManager::SetMemberReady(LobbyId lobbyId, SessionId sid, bool isReady) {
    auto* lobby = GetLobby(lobbyId);
    if (!lobby) return false;

    for (auto& m : lobby->members) {
        if (m.sessionId == sid) {
            m.isReady = isReady;
            return true;
        }
    }
    return false;
}

bool LobbyManager::DisbandLobby(LobbyId lobbyId) {
    auto it = m_lobbies.find(lobbyId);
    if (it == m_lobbies.end()) return false;

    for (const auto& m : it->second.members) {
        m_sessionToLobby.erase(m.sessionId);
    }
    m_lobbies.erase(it);
    return true;
}

std::vector<const Lobby*> LobbyManager::GetPublicLobbies() const {
    std::vector<const Lobby*> result;
    for (const auto& [id, lobby] : m_lobbies) {
        if (!lobby.isPrivate) {
            result.push_back(&lobby);
        }
    }
    return result;
}

void LobbyManager::Clear() {
    m_lobbies.clear();
    m_sessionToLobby.clear();
}

} // namespace wildbound::coordinator

