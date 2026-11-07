#include "DedicatedServerRegistry.hpp"

namespace wildbound::coordinator {

ServerInstanceId DedicatedServerRegistry::GenerateServerId() noexcept {
    return m_nextServerId++;
}

ServerInstanceId DedicatedServerRegistry::RegisterServer(network::ClientId connectionId,
                                                         const std::string& serverName,
                                                         const std::string& hostAddress,
                                                         uint16_t port,
                                                         uint32_t maxCapacity,
                                                         const std::string& mapName,
                                                         const std::string& supportedGameMode,
                                                         float currentTime) {
    // If connection already registered, remove old
    auto it = m_connectionToServer.find(connectionId);
    if (it != m_connectionToServer.end()) {
        m_servers.erase(it->second);
        m_connectionToServer.erase(it);
    }

    ServerInstanceId newId = GenerateServerId();
    ServerInstance s{
        .serverId = newId,
        .connectionId = connectionId,
        .serverName = serverName.empty() ? ("DedicatedServer_" + std::to_string(newId)) : serverName,
        .hostAddress = hostAddress.empty() ? "127.0.0.1" : hostAddress,
        .port = (port > 0) ? port : static_cast<uint16_t>(7777),
        .maxCapacity = (maxCapacity > 0) ? maxCapacity : 32,
        .currentPlayers = 0,
        .assignedMatchId = "",
        .supportedGameMode = supportedGameMode.empty() ? "solo" : supportedGameMode,
        .mapName = mapName.empty() ? "maps/world/battle_royale_island.json" : mapName,
        .state = ServerRegistryState::Available,
        .lastHeartbeatTime = currentTime,
        .registeredTime = currentTime
    };

    m_servers[newId] = std::move(s);
    m_connectionToServer[connectionId] = newId;
    return newId;
}

ServerInstance* DedicatedServerRegistry::GetServer(ServerInstanceId id) noexcept {
    auto it = m_servers.find(id);
    return (it != m_servers.end()) ? &it->second : nullptr;
}

const ServerInstance* DedicatedServerRegistry::GetServer(ServerInstanceId id) const noexcept {
    auto it = m_servers.find(id);
    return (it != m_servers.end()) ? &it->second : nullptr;
}

ServerInstance* DedicatedServerRegistry::GetServerByConnection(network::ClientId connectionId) noexcept {
    auto it = m_connectionToServer.find(connectionId);
    if (it == m_connectionToServer.end()) return nullptr;
    return GetServer(it->second);
}

const ServerInstance* DedicatedServerRegistry::GetServerByConnection(network::ClientId connectionId) const noexcept {
    auto it = m_connectionToServer.find(connectionId);
    if (it == m_connectionToServer.end()) return nullptr;
    return GetServer(it->second);
}

bool DedicatedServerRegistry::UpdateHeartbeat(ServerInstanceId id, uint32_t currentPlayers,
                                              ServerRegistryState state, float currentTime) {
    auto* s = GetServer(id);
    if (!s) return false;
    s->currentPlayers = currentPlayers;
    s->state = state;
    s->lastHeartbeatTime = currentTime;
    return true;
}

bool DedicatedServerRegistry::SetServerState(ServerInstanceId id, ServerRegistryState state) {
    auto* s = GetServer(id);
    if (!s) return false;
    s->state = state;
    return true;
}

bool DedicatedServerRegistry::SetAssignedMatch(ServerInstanceId id, const std::string& matchId) {
    auto* s = GetServer(id);
    if (!s) return false;
    s->assignedMatchId = matchId;
    return true;
}

ServerInstance* DedicatedServerRegistry::FindAvailableServer(uint32_t requiredCapacity, const std::string& gameMode) {
    for (auto& [id, server] : m_servers) {
        if (server.IsAvailableFor(requiredCapacity)) {
            if (gameMode.empty() || server.supportedGameMode == gameMode || server.supportedGameMode == "any") {
                return &server;
            }
        }
    }
    return nullptr;
}

void DedicatedServerRegistry::CheckHeartbeatTimeouts(float currentTime, float timeoutSeconds,
                                                     std::vector<ServerInstanceId>& outTimedOut) {
    outTimedOut.clear();
    for (auto& [id, server] : m_servers) {
        if (server.state != ServerRegistryState::Offline) {
            if ((currentTime - server.lastHeartbeatTime) > timeoutSeconds) {
                server.state = ServerRegistryState::Offline;
                outTimedOut.push_back(id);
            }
        }
    }
}

bool DedicatedServerRegistry::DeregisterServer(ServerInstanceId id) {
    auto it = m_servers.find(id);
    if (it == m_servers.end()) return false;
    m_connectionToServer.erase(it->second.connectionId);
    m_servers.erase(it);
    return true;
}

bool DedicatedServerRegistry::DeregisterServerByConnection(network::ClientId connectionId) {
    auto it = m_connectionToServer.find(connectionId);
    if (it == m_connectionToServer.end()) return false;
    ServerInstanceId sid = it->second;
    m_connectionToServer.erase(it);
    m_servers.erase(sid);
    return true;
}

size_t DedicatedServerRegistry::GetAvailableServerCount() const noexcept {
    size_t count = 0;
    for (const auto& [id, s] : m_servers) {
        if (s.state == ServerRegistryState::Available) ++count;
    }
    return count;
}

void DedicatedServerRegistry::Clear() {
    m_servers.clear();
    m_connectionToServer.clear();
}

} // namespace wildbound::coordinator

