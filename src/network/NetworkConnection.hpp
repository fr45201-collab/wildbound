#pragma once

#include "NetworkTypes.hpp"
#include "ConnectionState.hpp"
#include "NetworkStatistics.hpp"
#include <string>
#include <chrono>

namespace wildbound::network {

class NetworkConnection {
public:
    NetworkConnection() = default;
    NetworkConnection(ClientId id, void* peerHandle, std::string address, uint16_t port);

    [[nodiscard]] ClientId GetId() const noexcept { return m_id; }
    [[nodiscard]] void* GetPeerHandle() const noexcept { return m_peerHandle; }
    void SetPeerHandle(void* handle) noexcept { m_peerHandle = handle; }

    [[nodiscard]] const std::string& GetAddress() const noexcept { return m_address; }
    [[nodiscard]] uint16_t GetPort() const noexcept { return m_port; }

    [[nodiscard]] ConnectionState GetState() const noexcept { return m_state; }
    void SetState(ConnectionState state) noexcept { m_state = state; }

    [[nodiscard]] const std::string& GetDisconnectReason() const noexcept { return m_disconnectReason; }
    void SetDisconnectReason(std::string reason) { m_disconnectReason = std::move(reason); }

    [[nodiscard]] NetworkStatistics& GetStats() noexcept { return m_stats; }
    [[nodiscard]] const NetworkStatistics& GetStats() const noexcept { return m_stats; }

    [[nodiscard]] uint64_t GetLastActivityTimeMs() const noexcept { return m_lastActivityTimeMs; }
    void Touch(uint64_t currentMs) noexcept { m_lastActivityTimeMs = currentMs; }

    [[nodiscard]] bool IsConnected() const noexcept { return m_state == ConnectionState::Connected; }

private:
    ClientId m_id{ 0 };
    void* m_peerHandle{ nullptr }; // Points to ENetPeer* internally
    std::string m_address;
    uint16_t m_port{ 0 };
    ConnectionState m_state{ ConnectionState::Disconnected };
    std::string m_disconnectReason;
    uint64_t m_lastActivityTimeMs{ 0 };
    NetworkStatistics m_stats;
};

} // namespace wildbound::network

