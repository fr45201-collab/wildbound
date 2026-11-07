#include "NetworkStatistics.hpp"

namespace wildbound::network {

void NetworkStatistics::RecordSent(size_t bytes, bool reliable) {
    m_metrics.packetsSent++;
    m_metrics.bytesSent += bytes;
    if (reliable) {
        m_metrics.reliablePacketsSent++;
    } else {
        m_metrics.unreliablePacketsSent++;
    }
}

void NetworkStatistics::RecordReceived(size_t bytes) {
    m_metrics.packetsReceived++;
    m_metrics.bytesReceived += bytes;
}

void NetworkStatistics::RecordValidationFailure() {
    m_metrics.validationFailures++;
}

void NetworkStatistics::UpdateMetrics(uint32_t rttMs, float packetLoss) {
    m_metrics.roundTripTimeMs = rttMs;
    m_metrics.packetLossPercent = packetLoss;
}

void NetworkStatistics::Update(float dt) {
    m_sampleTimer += dt;
    if (m_sampleTimer >= 1.0f) {
        uint64_t bytesSentDelta = m_metrics.bytesSent - m_lastBytesSent;
        uint64_t bytesRecvDelta = m_metrics.bytesReceived - m_lastBytesRecv;
        uint64_t packetsSentDelta = m_metrics.packetsSent - m_lastPacketsSent;
        uint64_t packetsRecvDelta = m_metrics.packetsReceived - m_lastPacketsRecv;

        m_bytesSentPerSec = static_cast<float>(bytesSentDelta) / m_sampleTimer;
        m_bytesRecvPerSec = static_cast<float>(bytesRecvDelta) / m_sampleTimer;
        m_packetsSentPerSec = static_cast<float>(packetsSentDelta) / m_sampleTimer;
        m_packetsRecvPerSec = static_cast<float>(packetsRecvDelta) / m_sampleTimer;

        m_lastBytesSent = m_metrics.bytesSent;
        m_lastBytesRecv = m_metrics.bytesReceived;
        m_lastPacketsSent = m_metrics.packetsSent;
        m_lastPacketsRecv = m_metrics.packetsReceived;
        m_sampleTimer = 0.0f;
    }
}

void NetworkStatistics::Reset() {
    m_metrics.Reset();
    m_lastBytesSent = 0;
    m_lastBytesRecv = 0;
    m_lastPacketsSent = 0;
    m_lastPacketsRecv = 0;
    m_sampleTimer = 0.0f;
    m_bytesSentPerSec = 0.0f;
    m_bytesRecvPerSec = 0.0f;
    m_packetsSentPerSec = 0.0f;
    m_packetsRecvPerSec = 0.0f;
}

} // namespace wildbound::network

