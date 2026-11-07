#include "ServerDiagnostics.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace wildbound::server {

void ServerDiagnostics::Reset() {
    m_metrics = ServerMetrics{};
    m_tickAccumulatorTime = 0.0f;
    m_ticksInCurrentSec = 0;
    m_tickDurationSum = 0.0f;
    m_tickDurationCount = 0;
}

void ServerDiagnostics::Update(float dt, ServerLifecycleState currentState) {
    m_metrics.uptimeSeconds += dt;
    m_metrics.state = currentState;

    m_tickAccumulatorTime += dt;
    if (m_tickAccumulatorTime >= 1.0f) {
        m_metrics.actualTicksPerSecond = static_cast<float>(m_ticksInCurrentSec) / m_tickAccumulatorTime;
        m_ticksInCurrentSec = 0;
        m_tickAccumulatorTime = 0.0f;

        if (m_tickDurationCount > 0) {
            m_metrics.averageTickDurationMs = m_tickDurationSum / static_cast<float>(m_tickDurationCount);
            m_tickDurationSum = 0.0f;
            m_tickDurationCount = 0;
        }
    }
}

void ServerDiagnostics::RecordTickDuration(float durationMs, bool wasOverload) {
    m_metrics.currentTick++;
    m_ticksInCurrentSec++;

    m_tickDurationSum += durationMs;
    m_tickDurationCount++;

    if (durationMs > m_metrics.maxTickDurationMs) {
        m_metrics.maxTickDurationMs = durationMs;
    }

    if (wasOverload) {
        m_metrics.simulationOverloadCount++;
    }
}

void ServerDiagnostics::RecordPacketReceived(size_t bytes) {
    m_metrics.packetsReceived++;
    m_metrics.bytesReceived += bytes;
}

void ServerDiagnostics::RecordPacketSent(size_t bytes) {
    m_metrics.packetsSent++;
    m_metrics.bytesSent += bytes;
}

void ServerDiagnostics::RecordInvalidRequest() {
    m_metrics.invalidRequests++;
}

void ServerDiagnostics::RecordRejectedConnection() {
    m_metrics.rejectedConnections++;
}

void ServerDiagnostics::RecordDisconnect() {
    m_metrics.disconnects++;
}

void ServerDiagnostics::RecordTimeout() {
    m_metrics.timeouts++;
}

void ServerDiagnostics::SetEntityCounts(size_t total, size_t enemies, size_t loot) {
    m_metrics.activeEntities = total;
    m_metrics.activeEnemies = enemies;
    m_metrics.activeLoot = loot;
}

void ServerDiagnostics::SetClientCounts(size_t connected, size_t registered) {
    m_metrics.connectedClients = connected;
    m_metrics.registeredParticipants = registered;
}

std::string ServerDiagnostics::FormatSummary() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "[SERVER DIAGNOSTICS] State: " << ServerLifecycleStateToString(m_metrics.state)
       << " | Uptime: " << m_metrics.uptimeSeconds << "s"
       << " | Tick: " << m_metrics.currentTick
       << " | TPS: " << m_metrics.actualTicksPerSecond << " / " << m_metrics.configuredTickRate << " Hz"
       << " | AvgTick: " << m_metrics.averageTickDurationMs << "ms (Peak: " << m_metrics.maxTickDurationMs << "ms)"
       << " | Overloads: " << m_metrics.simulationOverloadCount
       << "\n  Clients: " << m_metrics.connectedClients << " connected (" << m_metrics.registeredParticipants << " registered)"
       << " | Entities: " << m_metrics.activeEntities << " (" << m_metrics.activeEnemies << " enemies, " << m_metrics.activeLoot << " loot)"
       << " | Net I/O: " << (m_metrics.bytesReceived / 1024.0f) << " KB in (" << m_metrics.packetsReceived << " pkts) / "
       << (m_metrics.bytesSent / 1024.0f) << " KB out (" << m_metrics.packetsSent << " pkts)";
    return ss.str();
}

} // namespace wildbound::server

