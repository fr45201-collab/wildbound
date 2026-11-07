#pragma once

#include "ENetTransport.hpp"
#include "ConnectionState.hpp"
#include "ConnectionEvents.hpp"
#include "PacketValidator.hpp"
#include "NetworkStatistics.hpp"
#include <functional>
#include <vector>

namespace wildbound::network {

class ENetClient {
public:
    using EventCallback = std::function<void(const ConnectionEvent&)>;

    ENetClient();
    ~ENetClient();

    bool Connect(const std::string& address, uint16_t port, uint32_t timeoutMs = 5000);
    void Disconnect(uint32_t reason = 0);

    void Update(float dt);

    bool Send(ChannelType channel, DeliveryMode mode, const uint8_t* data, size_t size);

    void SetEventCallback(EventCallback callback) { m_eventCallback = std::move(callback); }

    [[nodiscard]] ConnectionState GetState() const noexcept { return m_state; }
    [[nodiscard]] bool IsConnected() const noexcept { return m_state == ConnectionState::Connected; }
    [[nodiscard]] uint32_t GetRoundTripTimeMs() const noexcept;
    [[nodiscard]] const std::string& GetServerAddress() const noexcept { return m_serverAddress; }
    [[nodiscard]] uint16_t GetServerPort() const noexcept { return m_serverPort; }

    [[nodiscard]] NetworkStatistics& GetStats() noexcept { return m_stats; }
    [[nodiscard]] const NetworkStatistics& GetStats() const noexcept { return m_stats; }

private:
    ENetTransport m_transport;
    ConnectionState m_state{ ConnectionState::Disconnected };
    NetworkStatistics m_stats;
    EventCallback m_eventCallback;
    std::string m_serverAddress;
    uint16_t m_serverPort{ 0 };
    float m_connectionTimer{ 0.0f };
    float m_timeoutThreshold{ 5.0f };
};

} // namespace wildbound::network

