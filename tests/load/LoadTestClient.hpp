#pragma once

#include "LoadTestMetrics.hpp"
#include "../../src/network/ENetClient.hpp"
#include "../../src/network/NetworkMessage.hpp"
#include "../../src/network/NetworkProtocol.hpp"
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <random>

namespace wildbound::testing {

class LoadTestClient {
public:
    LoadTestClient(uint32_t simulatedIndex = 0);
    ~LoadTestClient();

    bool Connect(const std::string& host, uint16_t port, const std::string& playerName);
    void Disconnect();
    void Update(float dt);

    // Gameplay simulation actions
    void SendInput(float dirX, float dirY, float facingAngle, uint8_t actionFlags = 0);
    void SendAttack(uint8_t weaponSlot = 1, float aimAngle = 0.0f);
    void SendPickup(uint32_t itemId);

    // Adverse network conditions
    void SetArtificialLatencyMs(uint32_t latencyMs) { m_artificialLatencyMs = latencyMs; }
    void SetArtificialPacketLoss(double lossPercent) { m_artificialLossPercent = lossPercent; }

    // Status queries
    [[nodiscard]] bool IsConnected() const noexcept { return m_client.IsConnected(); }
    [[nodiscard]] bool IsAccepted() const noexcept { return m_isAccepted; }
    [[nodiscard]] uint16_t GetAssignedClientId() const noexcept { return m_assignedClientId; }
    [[nodiscard]] uint32_t GetAssignedParticipantId() const noexcept { return m_assignedParticipantId; }
    [[nodiscard]] const std::string& GetPlayerName() const noexcept { return m_playerName; }
    [[nodiscard]] const ClientConnectionMetric& GetMetrics() const noexcept { return m_metrics; }

    // RTT & Diagnostics
    [[nodiscard]] uint32_t GetRttMs() const noexcept { return m_client.GetRoundTripTimeMs(); }

    void SetAutonomousSimulation(bool enabled) { m_autonomousSimulation = enabled; }

private:
    void SendRawPacket(network::ChannelType channel, network::DeliveryMode mode,
                       network::MessageType type, const uint8_t* payload, size_t size);
    void HandlePacket(uint16_t messageType, const uint8_t* data, size_t size);

    uint32_t m_simulatedIndex{ 0 };
    network::ENetClient m_client;
    std::string m_playerName;
    bool m_isAccepted{ false };
    uint16_t m_assignedClientId{ 0 };
    uint32_t m_assignedParticipantId{ 0 };
    std::string m_assignedMatchId;

    ClientConnectionMetric m_metrics;
    std::chrono::steady_clock::time_point m_connectStartTime;

    // Simulation state
    bool m_autonomousSimulation{ true };
    float m_inputTimer{ 0.0f };
    float m_attackTimer{ 0.0f };
    uint32_t m_inputSequence{ 0 };
    float m_simulatedAngle{ 0.0f };
    float m_simulatedPosX{ 400.0f };
    float m_simulatedPosY{ 400.0f };

    // Adverse network conditions
    uint32_t m_artificialLatencyMs{ 0 };
    double m_artificialLossPercent{ 0.0 };
    std::mt19937 m_rng;
};

} // namespace wildbound::testing

