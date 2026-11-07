#pragma once

#include "ServerLifecycle.hpp"
#include <cstdint>
#include <cstddef>
#include <string>

namespace wildbound::server {

struct ServerMetrics {
    ServerLifecycleState state{ ServerLifecycleState::Uninitialized };
    float uptimeSeconds{ 0.0f };
    uint64_t currentTick{ 0 };
    uint32_t configuredTickRate{ 30 };
    float actualTicksPerSecond{ 0.0f };
    float averageTickDurationMs{ 0.0f };
    float maxTickDurationMs{ 0.0f };
    uint64_t simulationOverloadCount{ 0 };

    size_t connectedClients{ 0 };
    size_t registeredParticipants{ 0 };
    size_t activeEntities{ 0 };
    size_t activeEnemies{ 0 };
    size_t activeLoot{ 0 };

    uint64_t packetsReceived{ 0 };
    uint64_t packetsSent{ 0 };
    uint64_t bytesReceived{ 0 };
    uint64_t bytesSent{ 0 };

    uint64_t invalidRequests{ 0 };
    uint64_t rejectedConnections{ 0 };
    uint64_t disconnects{ 0 };
    uint64_t timeouts{ 0 };
};

class ServerDiagnostics {
public:
    ServerDiagnostics() = default;
    ~ServerDiagnostics() = default;

    void Reset();
    void Update(float dt, ServerLifecycleState currentState);
    void RecordTickDuration(float durationMs, bool wasOverload);

    void RecordPacketReceived(size_t bytes);
    void RecordPacketSent(size_t bytes);
    void RecordInvalidRequest();
    void RecordRejectedConnection();
    void RecordDisconnect();
    void RecordTimeout();

    void SetEntityCounts(size_t total, size_t enemies, size_t loot);
    void SetClientCounts(size_t connected, size_t registered);

    [[nodiscard]] const ServerMetrics& GetMetrics() const noexcept { return m_metrics; }
    [[nodiscard]] std::string FormatSummary() const;

private:
    ServerMetrics m_metrics{};
    float m_tickAccumulatorTime{ 0.0f };
    uint32_t m_ticksInCurrentSec{ 0 };
    float m_tickDurationSum{ 0.0f };
    uint32_t m_tickDurationCount{ 0 };
};

} // namespace wildbound::server

