#include "ConnectionManager.hpp"

namespace wildbound::network {

ClientId ConnectionManager::AddConnection(void* peerHandle, const std::string& address, uint16_t port) {
    ClientId id = m_nextClientId++;
    if (m_nextClientId == 0) m_nextClientId = 1; // Wrap around skipping 0 (server)

    m_connections[id] = NetworkConnection(id, peerHandle, address, port);
    if (peerHandle) {
        m_peerToId[peerHandle] = id;
    }
    return id;
}

bool ConnectionManager::RemoveConnection(ClientId id) {
    auto it = m_connections.find(id);
    if (it == m_connections.end()) {
        return false;
    }

    if (it->second.GetPeerHandle()) {
        m_peerToId.erase(it->second.GetPeerHandle());
    }

    m_connections.erase(it);
    return true;
}

NetworkConnection* ConnectionManager::FindConnection(ClientId id) noexcept {
    auto it = m_connections.find(id);
    return (it != m_connections.end()) ? &it->second : nullptr;
}

const NetworkConnection* ConnectionManager::FindConnection(ClientId id) const noexcept {
    auto it = m_connections.find(id);
    return (it != m_connections.end()) ? &it->second : nullptr;
}

NetworkConnection* ConnectionManager::FindConnectionByPeer(void* peerHandle) noexcept {
    auto it = m_peerToId.find(peerHandle);
    if (it == m_peerToId.end()) return nullptr;
    return FindConnection(it->second);
}

ConnectionState ConnectionManager::GetConnectionState(ClientId id) const noexcept {
    const auto* conn = FindConnection(id);
    return conn ? conn->GetState() : ConnectionState::Disconnected;
}

void ConnectionManager::SetConnectionState(ClientId id, ConnectionState state) {
    auto* conn = FindConnection(id);
    if (conn) {
        conn->SetState(state);
    }
}

std::vector<ClientId> ConnectionManager::GetActiveConnectionIds() const {
    std::vector<ClientId> result;
    result.reserve(m_connections.size());
    for (const auto& [id, conn] : m_connections) {
        if (conn.IsConnected()) {
            result.push_back(id);
        }
    }
    return result;
}

void ConnectionManager::ForEachConnection(const std::function<void(NetworkConnection&)>& func) {
    for (auto& [id, conn] : m_connections) {
        func(conn);
    }
}

void ConnectionManager::Update(float dt) {
    for (auto& [id, conn] : m_connections) {
        conn.GetStats().Update(dt);
    }
}

void ConnectionManager::Reset() {
    m_connections.clear();
    m_peerToId.clear();
    m_nextClientId = 1;
}

} // namespace wildbound::network

