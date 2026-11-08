#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

namespace wildbound::testing {

struct ClientConnectionMetric {
    uint32_t clientId = 0;
    std::string playerName;
    bool connected = false;
    bool accepted = false;
    double connectionTimeMs = 0.0;
    uint32_t rttMs = 0;
    uint32_t packetsSent = 0;
    uint32_t packetsReceived = 0;
    uint32_t snapshotsReceived = 0;
    uint32_t attacksSent = 0;
    uint32_t pickupsSent = 0;
};

struct ServerTickMetric {
    uint64_t tickNumber = 0;
    double tickDurationMs = 0.0;
    bool isOverload = false;
    size_t activeParticipants = 0;
};

struct LoadTestLevelResult {
    uint32_t configuredClients = 0;
    uint32_t connectedClients = 0;
    uint32_t admittedParticipants = 0;
    uint32_t failedConnections = 0;
    uint32_t disconnectCount = 0;
    uint32_t unexpectedDisconnects = 0;

    double avgConnectionTimeMs = 0.0;
    double maxConnectionTimeMs = 0.0;

    double avgTickDurationMs = 0.0;
    double maxTickDurationMs = 0.0;
    uint32_t tickOverrunCount = 0;

    double avgRttMs = 0.0;
    double medianRttMs = 0.0;
    double p95RttMs = 0.0;
    double p99RttMs = 0.0;

    uint64_t totalPacketsSent = 0;
    uint64_t totalPacketsReceived = 0;
    uint64_t totalSnapshotsReceived = 0;
    double totalBytesTransferredKb = 0.0;

    bool serverRemainedStable = true;
    std::string notes;
};

struct LatencyScenarioResult {
    uint32_t configuredLatencyMs = 0;
    double measuredRttAvgMs = 0.0;
    double measuredRttMedianMs = 0.0;
    double measuredRttP95Ms = 0.0;
    double measuredRttP99Ms = 0.0;
    bool synchronizationMaintained = true;
};

struct PacketLossScenarioResult {
    double configuredLossPercent = 0.0;
    uint32_t packetsSent = 0;
    uint32_t packetsLost = 0;
    bool stateRecovered = true;
    bool connectionMaintained = true;
};

struct MatchStabilityResult {
    uint32_t matchesStarted = 0;
    uint32_t matchesCompleted = 0;
    uint32_t matchesInterrupted = 0;
    uint32_t totalEliminationsProcessed = 0;
    bool winnerAuthoritativelyDeclared = false;
    bool memoryGrowthBounded = true;
};

struct ServerRecoveryResult {
    bool gracefulShutdownClean = false;
    bool unexpectedTerminationDetected = false;
    bool restartSuccessful = false;
    bool clientReconnectionHandledSafely = false;
    bool activeMatchStatePersisted = false; // Evaluated based on actual architecture
    std::string activeMatchRecoveryClassification; // "Process Recovery", "Service Recovery", "Match Recovery"
};

} // namespace wildbound::testing

