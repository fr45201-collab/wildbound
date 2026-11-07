#pragma once

#include "ENetTransport.hpp"
#include "ConnectionManager.hpp"
#include "ConnectionEvents.hpp"
#include "PacketValidator.hpp"
#include "NetworkStatistics.hpp"
#include <functional>
#include <vector>

namespace wildbound::network {

class ENetServer {
public:
    using EventCallback = std::function<void(const ConnectionEvent&)>;

    ENetServer();
    ~ENetServer();

    bool Start(uint16_t port, size_t maxClients = 32);
    void Stop();

    void Update(float dt);

    bool Send(ClientId clientId, ChannelType channel, DeliveryMode mode,
              const uint8_t* data, size_t size);
    void Broadcast(ChannelType channel, DeliveryMode mode,
                   const uint8_t* data, size_t size, ClientId excludeClientId = 0);
    void DisconnectClient(ClientId clientId, uint32_t reason = 0);

    void SetEventCallback(EventCallback callback) { m_eventCallback = std::move(callback); }

    [[nodiscard]] bool IsListening() const noexcept { return m_isRunning; }
    [[nodiscard]] uint16_t GetPort() const noexcept { return m_port; }
    [[nodiscard]] size_t GetConnectedCount() const noexcept { return m_connectionManager.GetConnectionCount(); }

    [[nodiscard]] ConnectionManager& GetConnectionManager() noexcept { return m_connectionManager; }
    [[nodiscard]] const ConnectionManager& GetConnectionManager() const noexcept { return m_connectionManager; }

    [[nodiscard]] NetworkStatistics& GetGlobalStats() noexcept { return m_globalStats; }
    [[nodiscard]] const NetworkStatistics& GetGlobalStats() const noexcept { return m_globalStats; }

private:
    ENetTransport m_transport;
    ConnectionManager m_connectionManager;
    NetworkStatistics m_globalStats;
    EventCallback m_eventCallback;
    uint16_t m_port{ 0 };
    bool m_isRunning{ false };
};

} // namespace wildbound::network

