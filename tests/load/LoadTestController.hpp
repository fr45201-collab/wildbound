#pragma once

#include "LoadTestClient.hpp"
#include "LoadTestMetrics.hpp"
#include "../../src/server/DedicatedServer.hpp"
#include <vector>
#include <memory>
#include <string>

namespace wildbound::testing {

class LoadTestController {
public:
    LoadTestController(uint16_t serverPort = 17777);
    ~LoadTestController();

    // Start / Stop dedicated server
    bool StartServer(uint16_t port = 17777, const std::string& mode = "solo");
    void StopServer();
    void RestartServer(uint16_t port = 17777);

    // Incremental concurrency testing
    LoadTestLevelResult RunConcurrentLoadLevel(uint32_t clientCount, float durationSeconds,
                                              uint32_t connectionIntervalMs = 25);

    // Adverse network condition testing
    LatencyScenarioResult RunLatencyTest(uint32_t clientCount, uint32_t latencyMs, float durationSeconds = 5.0f);
    PacketLossScenarioResult RunPacketLossTest(uint32_t clientCount, double lossPercent, float durationSeconds = 5.0f);

    // Match lifecycle & repeated matches
    MatchStabilityResult RunMatchStabilityTest(uint32_t matchCount, uint32_t clientCount);

    // Disconnect & reconnect testing
    bool RunDisconnectReconnectTest(uint32_t clientCount);

    // Server failure & recovery testing
    ServerRecoveryResult RunServerFailureRecoveryTest();

    // Extended duration stability
    LoadTestLevelResult RunLongDurationTest(uint32_t clientCount, float durationSeconds);

    // Tick pump
    void Pump(float dt);

    [[nodiscard]] server::DedicatedServer* GetServer() { return m_server.get(); }
    [[nodiscard]] uint16_t GetPort() const noexcept { return m_serverPort; }

private:
    uint16_t m_serverPort{ 17777 };
    std::unique_ptr<server::DedicatedServer> m_server;
    std::vector<std::unique_ptr<LoadTestClient>> m_clients;
    bool m_serverRunning{ false };
};

} // namespace wildbound::testing

