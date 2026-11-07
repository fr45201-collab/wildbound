#pragma once

#include "NetworkConnection.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace wildbound::network {

class ConnectionManager {
public:
    ConnectionManager() = default;
    ~ConnectionManager() = default;

    ClientId AddConnection(void* peerHandle, const std::string& address, uint16_t port);
    bool RemoveConnection(ClientId id);

    [[nodiscard]] NetworkConnection* FindConnection(ClientId id) noexcept;
    [[nodiscard]] const NetworkConnection* FindConnection(ClientId id) const noexcept;
    [[nodiscard]] NetworkConnection* FindConnectionByPeer(void* peerHandle) noexcept;

    [[nodiscard]] ConnectionState GetConnectionState(ClientId id) const noexcept;
    void SetConnectionState(ClientId id, ConnectionState state);

    [[nodiscard]] size_t GetConnectionCount() const noexcept { return m_connections.size(); }
    [[nodiscard]] std::vector<ClientId> GetActiveConnectionIds() const;
    [[nodiscard]] const std::unordered_map<ClientId, NetworkConnection>& GetConnections() const noexcept { return m_connections; }
    [[nodiscard]] std::unordered_map<ClientId, NetworkConnection>& GetConnections() noexcept { return m_connections; }

    void ForEachConnection(const std::function<void(NetworkConnection&)>& func);
    void Update(float dt);
    void Reset();

private:
    std::unordered_map<ClientId, NetworkConnection> m_connections;
    std::unordered_map<void*, ClientId> m_peerToId;
    ClientId m_nextClientId{ 1 };
};

} // namespace wildbound::network

