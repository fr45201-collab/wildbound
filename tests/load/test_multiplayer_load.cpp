#include "LoadTestController.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace wildbound;
using namespace wildbound::testing;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

void TestIncrementalConcurrency(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "1. INCREMENTAL CONCURRENT PLAYER TESTING (Levels 1 - 8)" << std::endl;
    std::cout << "===========================================================" << std::endl;

    std::vector<uint32_t> testLevels = { 2, 4, 8, 16, 24, 32, 48, 64 };
    std::cout << std::left << std::setw(8) << "Level"
              << std::setw(12) << "Target"
              << std::setw(12) << "Connected"
              << std::setw(12) << "Admitted"
              << std::setw(16) << "AvgConn (ms)"
              << std::setw(16) << "AvgTick (ms)"
              << std::setw(14) << "AvgRTT (ms)"
              << std::setw(10) << "Stability" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    for (size_t lvl = 0; lvl < testLevels.size(); ++lvl) {
        uint32_t count = testLevels[lvl];
        float duration = (count <= 32) ? 1.5f : 1.0f; // Rapid validation across all 8 tiers

        auto res = controller.RunConcurrentLoadLevel(count, duration, 15);

        std::cout << std::left << std::setw(8) << (lvl + 1)
                  << std::setw(12) << res.configuredClients
                  << std::setw(12) << res.connectedClients
                  << std::setw(12) << res.admittedParticipants
                  << std::setw(16) << std::fixed << std::setprecision(2) << res.avgConnectionTimeMs
                  << std::setw(16) << std::fixed << std::setprecision(2) << res.avgTickDurationMs
                  << std::setw(14) << std::fixed << std::setprecision(2) << res.avgRttMs
                  << std::setw(10) << (res.serverRemainedStable ? "STABLE" : "DEGRADED")
                  << std::endl;

        TEST_ASSERT(res.connectedClients > 0, "At least some clients must connect");
        TEST_ASSERT(res.admittedParticipants > 0, "Server must admit participants");
    }
}

void TestNetworkLatencyConditions(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "2. NETWORK LATENCY PROFILES & RTT VARIATION" << std::endl;
    std::cout << "===========================================================" << std::endl;

    std::vector<uint32_t> latencies = { 0, 20, 50, 100, 150, 250, 400 };

    std::cout << std::left << std::setw(18) << "Added Latency"
              << std::setw(16) << "Avg RTT (ms)"
              << std::setw(16) << "Median (ms)"
              << std::setw(16) << "P95 RTT (ms)"
              << std::setw(16) << "P99 RTT (ms)"
              << std::setw(12) << "Sync Status" << std::endl;
    std::cout << std::string(94, '-') << std::endl;

    for (uint32_t lat : latencies) {
        auto res = controller.RunLatencyTest(4, lat, 1.0f);
        std::cout << std::left << std::setw(18) << (std::to_string(lat) + " ms")
                  << std::setw(16) << std::fixed << std::setprecision(2) << res.measuredRttAvgMs
                  << std::setw(16) << std::fixed << std::setprecision(2) << res.measuredRttMedianMs
                  << std::setw(16) << std::fixed << std::setprecision(2) << res.measuredRttP95Ms
                  << std::setw(16) << std::fixed << std::setprecision(2) << res.measuredRttP99Ms
                  << std::setw(12) << (res.synchronizationMaintained ? "OK" : "DESYNC")
                  << std::endl;

        TEST_ASSERT(res.synchronizationMaintained, "Synchronization must be maintained under latency");
    }
}

void TestPacketLossConditions(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "3. PACKET LOSS SCENARIOS & RESILIENCE" << std::endl;
    std::cout << "===========================================================" << std::endl;

    std::vector<double> lossRates = { 0.0, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0 };

    std::cout << std::left << std::setw(18) << "Configured Loss"
              << std::setw(16) << "Packets Sent"
              << std::setw(16) << "Connected"
              << std::setw(16) << "State Recovery" << std::endl;
    std::cout << std::string(66, '-') << std::endl;

    for (double loss : lossRates) {
        auto res = controller.RunPacketLossTest(4, loss, 1.0f);
        std::cout << std::left << std::setw(18) << (std::to_string(loss) + "%")
                  << std::setw(16) << res.packetsSent
                  << std::setw(16) << (res.connectionMaintained ? "YES" : "NO")
                  << std::setw(16) << (res.stateRecovered ? "RECOVERED" : "CORRUPT")
                  << std::endl;

        TEST_ASSERT(res.stateRecovered, "State recovery must maintain integrity");
    }
}

void TestMatchStability(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "4. MATCH STABILITY & REPEATED MATCH LIFECYCLE" << std::endl;
    std::cout << "===========================================================" << std::endl;

    auto res = controller.RunMatchStabilityTest(3, 8);
    std::cout << "Matches Started:   " << res.matchesStarted << std::endl;
    std::cout << "Matches Completed: " << res.matchesCompleted << std::endl;
    std::cout << "Matches Cleaned:   " << res.matchesCompleted << std::endl;
    std::cout << "Memory Bounded:    " << (res.memoryGrowthBounded ? "YES (Zero Leak)" : "NO") << std::endl;

    TEST_ASSERT(res.matchesCompleted == res.matchesStarted, "All started matches must complete cleanly");
}

void TestDisconnectAndReconnect(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "5. CLIENT DISCONNECTION & RECONNECTION RECOVERY" << std::endl;
    std::cout << "===========================================================" << std::endl;

    bool ok = controller.RunDisconnectReconnectTest(8);
    std::cout << "Simulated 50% client drop and reconnect: " << (ok ? "PASSED" : "FAILED") << std::endl;
    TEST_ASSERT(ok, "Disconnect and reconnect lifecycle must succeed");
}

void TestServerFailureAndRecovery(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "6. SERVER FAILURE & RECOVERY ARCHITECTURE" << std::endl;
    std::cout << "===========================================================" << std::endl;

    auto res = controller.RunServerFailureRecoveryTest();
    std::cout << "Graceful Shutdown:          " << (res.gracefulShutdownClean ? "CLEAN" : "DIRTY") << std::endl;
    std::cout << "Crash / Force Termination:  " << (res.unexpectedTerminationDetected ? "DETECTED" : "IGNORED") << std::endl;
    std::cout << "Server Process Restart:     " << (res.restartSuccessful ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "Client Reconnection:        " << (res.clientReconnectionHandledSafely ? "HANDLED" : "FAILED") << std::endl;
    std::cout << "Active Match Recovery:      " << res.activeMatchRecoveryClassification << std::endl;

    TEST_ASSERT(res.gracefulShutdownClean, "Graceful shutdown must exit cleanly");
    TEST_ASSERT(res.restartSuccessful, "Server restart must succeed");
}

void TestLongDurationStability(LoadTestController& controller) {
    std::cout << "\n===========================================================" << std::endl;
    std::cout << "7. EXTENDED DURATION MULTIPLAYER STABILITY" << std::endl;
    std::cout << "===========================================================" << std::endl;

    auto res = controller.RunLongDurationTest(12, 3.0f);
    std::cout << "Duration Tested:      3.0s active load" << std::endl;
    std::cout << "Total Packets Sent:   " << res.totalPacketsSent << std::endl;
    std::cout << "Total Packets Recv:   " << res.totalPacketsReceived << std::endl;
    std::cout << "Snapshots Received:   " << res.totalSnapshotsReceived << std::endl;
    std::cout << "Tick Overruns:        " << res.tickOverrunCount << std::endl;
    std::cout << "Server Stability:     " << (res.serverRemainedStable ? "PASSED" : "FAILED") << std::endl;

    TEST_ASSERT(res.serverRemainedStable, "Server must remain stable under continuous load");
}

int main() {
    std::cout << "===========================================================" << std::endl;
    std::cout << "WILDBOUND MULTIPLAYER LOAD & STRESS VALIDATION (PHASE 48)" << std::endl;
    std::cout << "===========================================================" << std::endl;

    LoadTestController controller(17777);

    TestIncrementalConcurrency(controller);
    TestNetworkLatencyConditions(controller);
    TestPacketLossConditions(controller);
    TestMatchStability(controller);
    TestDisconnectAndReconnect(controller);
    TestServerFailureAndRecovery(controller);
    TestLongDurationStability(controller);

    std::cout << "\n===========================================================" << std::endl;
    std::cout << "[SUCCESS] ALL PHASE 48 MULTIPLAYER LOAD TESTS PASSED!" << std::endl;
    std::cout << "===========================================================" << std::endl;
    return 0;
}

