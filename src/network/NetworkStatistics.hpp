#pragma once

#include <cstdint>
#include <chrono>

namespace wildbound::network {

struct ConnectionMetrics {
    uint64_t packetsSent{ 0 };
    uint64_t packetsReceived{ 0 };
    uint64_t bytesSent{ 0 };
    uint64_t bytesReceived{ 0 };
    uint64_t reliablePacketsSent{ 0 };
    uint64_t unreliablePacketsSent{ 0 };
    uint32_t roundTripTimeMs{ 0 };
    float packetLossPercent{ 0.0f };
    uint32_t validationFailures{ 0 };

    void Reset() noexcept {
        packetsSent = 0;
        packetsReceived = 0;
        bytesSent = 0;
        bytesReceived = 0;
        reliablePacketsSent = 0;
        unreliablePacketsSent = 0;
        roundTripTimeMs = 0;
        packetLossPercent = 0.0f;
        validationFailures = 0;
    }
};

class NetworkStatistics {
public:
    NetworkStatistics() = default;

    void RecordSent(size_t bytes, bool reliable);
    void RecordReceived(size_t bytes);
    void RecordValidationFailure();
    void UpdateMetrics(uint32_t rttMs, float packetLoss);

    void Update(float dt);

    [[nodiscard]] const ConnectionMetrics& GetMetrics() const noexcept { return m_metrics; }
    [[nodiscard]] float GetBytesSentPerSec() const noexcept { return m_bytesSentPerSec; }
    [[nodiscard]] float GetBytesReceivedPerSec() const noexcept { return m_bytesRecvPerSec; }
    [[nodiscard]] float GetPacketsSentPerSec() const noexcept { return m_packetsSentPerSec; }
    [[nodiscard]] float GetPacketsReceivedPerSec() const noexcept { return m_packetsRecvPerSec; }

    void Reset();

private:
    ConnectionMetrics m_metrics;

    uint64_t m_lastBytesSent{ 0 };
    uint64_t m_lastBytesRecv{ 0 };
    uint64_t m_lastPacketsSent{ 0 };
    uint64_t m_lastPacketsRecv{ 0 };

    float m_sampleTimer{ 0.0f };
    float m_bytesSentPerSec{ 0.0f };
    float m_bytesRecvPerSec{ 0.0f };
    float m_packetsSentPerSec{ 0.0f };
    float m_packetsRecvPerSec{ 0.0f };
};

} // namespace wildbound::network

