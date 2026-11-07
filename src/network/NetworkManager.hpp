#pragma once

#include "NetworkConfiguration.hpp"
#include "NetworkRole.hpp"
#include "ConnectionState.hpp"
#include "NetworkStatistics.hpp"
#include "NetworkConnection.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace wildbound::network {

class ENetServer;
class ENetClient;
class ReplicationManager;

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool Initialize(const std::string& configPath = "assets/data/networking/network_config.json");
    void Shutdown();

    // Replication Access
    [[nodiscard]] ReplicationManager* GetReplicationManager() noexcept;
    [[nodiscard]] const ReplicationManager* GetReplicationManager() const noexcept;

    // Host / Server Operations
    bool StartServer(uint16_t port = 0, size_t maxClients = 0);
    void StopServer();
    [[nodiscard]] bool IsServerRunning() const noexcept;
    [[nodiscard]] uint16_t GetServerPort() const noexcept;
    [[nodiscard]] size_t GetServerConnectedCount() const noexcept;
    [[nodiscard]] const std::unordered_map<ClientId, NetworkConnection>& GetServerConnections() const noexcept;
    [[nodiscard]] const NetworkStatistics& GetServerGlobalStats() const noexcept;
    void DisconnectServerClient(ClientId clientId, uint32_t reason = 0);

    // Client Operations
    bool ConnectClient(const std::string& address = "", uint16_t port = 0, uint32_t timeoutMs = 5000);
    void DisconnectClient();
    [[nodiscard]] bool IsClientConnected() const noexcept;
    [[nodiscard]] ConnectionState GetClientState() const noexcept;
    [[nodiscard]] uint32_t GetClientRoundTripTimeMs() const noexcept;
    [[nodiscard]] const std::string& GetClientServerAddress() const noexcept;
    [[nodiscard]] uint16_t GetClientServerPort() const noexcept;
    [[nodiscard]] const NetworkStatistics& GetClientStats() const noexcept;

    void Update(float dt);

    [[nodiscard]] NetworkRole GetRole() const noexcept { return m_role; }
    [[nodiscard]] const NetworkConfiguration& GetConfig() const noexcept { return m_config; }

    // Test packet sending helper
    bool SendTestMessage(bool reliable, const std::string& text = "TestPing");

    [[nodiscard]] ENetServer* GetServer() noexcept { return m_server.get(); }
    [[nodiscard]] const ENetServer* GetServer() const noexcept { return m_server.get(); }
    [[nodiscard]] ENetClient* GetClient() noexcept { return m_client.get(); }
    [[nodiscard]] const ENetClient* GetClient() const noexcept { return m_client.get(); }

private:
    NetworkConfiguration m_config;
    std::unique_ptr<ENetServer> m_server;
    std::unique_ptr<ENetClient> m_client;
    std::unique_ptr<ReplicationManager> m_replication;
    NetworkRole m_role{ NetworkRole::None };
    bool m_isInitialized{ false };
};

} // namespace wildbound::network

