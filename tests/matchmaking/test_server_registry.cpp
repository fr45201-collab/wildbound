#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "coordinator/DedicatedServerRegistry.hpp"

using namespace wildbound::coordinator;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILURE] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            std::abort(); \
        } \
    } while (0)

int main() {
    std::cout << "========================================================\n"
              << "TEST SUITE: Dedicated Server Registry Subsystem (Phase 41)\n"
              << "========================================================" << std::endl;

    DedicatedServerRegistry registry;
    TEST_ASSERT(registry.GetServerCount() == 0, "Initial server count must be 0");

    // 1. Server Registration
    std::cout << "[TEST] 1. Dedicated Server Registration..." << std::endl;
    ServerInstanceId s1 = registry.RegisterServer(
        10, "DedicatedServer_US_East_1", "192.168.1.100", 7777, 32,
        "maps/world/battle_royale_island.json", "squads", 10.0f
    );
    TEST_ASSERT(s1 != INVALID_SERVER_ID, "Server 1 registration must succeed");

    ServerInstanceId s2 = registry.RegisterServer(
        11, "DedicatedServer_US_East_2", "192.168.1.101", 7778, 16,
        "maps/world/battle_royale_island.json", "solo", 10.0f
    );
    TEST_ASSERT(s2 != INVALID_SERVER_ID, "Server 2 registration must succeed");
    TEST_ASSERT(registry.GetServerCount() == 2, "Server count must be 2");
    TEST_ASSERT(registry.GetAvailableServerCount() == 2, "Both servers must be available");

    // 2. Querying by Connection and ID
    std::cout << "[TEST] 2. Querying Servers..." << std::endl;
    auto* srv1 = registry.GetServer(s1);
    TEST_ASSERT(srv1 != nullptr, "Server 1 lookup must succeed");
    TEST_ASSERT(srv1->serverName == "DedicatedServer_US_East_1", "Name must match");
    TEST_ASSERT(srv1->port == 7777, "Port must be 7777");
    TEST_ASSERT(srv1->maxCapacity == 32, "Capacity must be 32");
    TEST_ASSERT(srv1->state == ServerRegistryState::Available, "Initial state must be Available");

    auto* srvByConn = registry.GetServerByConnection(11);
    TEST_ASSERT(srvByConn != nullptr && srvByConn->serverId == s2, "Connection 11 must resolve to s2");

    // 3. Finding Available Server Matching Capacity and Mode
    std::cout << "[TEST] 3. Available Server Allocation Query..." << std::endl;
    // Look for squads server with capacity 8
    auto* foundSquad = registry.FindAvailableServer(8, "squads");
    TEST_ASSERT(foundSquad != nullptr && foundSquad->serverId == s1, "Must select s1 for squads");

    // Look for solo server
    auto* foundSolo = registry.FindAvailableServer(2, "solo");
    TEST_ASSERT(foundSolo != nullptr && foundSolo->serverId == s2, "Must select s2 for solo");

    // Look for capacity exceeding available
    auto* foundNone = registry.FindAvailableServer(100, "solo");
    TEST_ASSERT(foundNone == nullptr, "Capacity 100 must return nullptr");

    // 4. Heartbeats & State Progression
    std::cout << "[TEST] 4. Heartbeat Updating & State Progression..." << std::endl;
    TEST_ASSERT(registry.UpdateHeartbeat(s1, 8, ServerRegistryState::InMatch, 25.0f), "Heartbeat update must succeed");
    TEST_ASSERT(srv1->currentPlayers == 8, "Current players must update to 8");
    TEST_ASSERT(srv1->state == ServerRegistryState::InMatch, "State must be InMatch");
    TEST_ASSERT(registry.GetAvailableServerCount() == 1, "Only 1 server must now be available");

    // Return to available
    TEST_ASSERT(registry.SetServerState(s1, ServerRegistryState::Available), "Return to available must succeed");
    TEST_ASSERT(registry.GetAvailableServerCount() == 2, "Both servers must be available again");

    // 5. Heartbeat Timeout Processing (15s threshold)
    std::cout << "[TEST] 5. Heartbeat Timeout Processing..." << std::endl;
    // s1 last heartbeat: 25.0f. s2 last heartbeat: 10.0f.
    // At currentTime: 30.0f:
    // s1 diff = 5s (healthy). s2 diff = 20s (> 15s, timed out).
    std::vector<ServerInstanceId> timedOut;
    registry.CheckHeartbeatTimeouts(30.0f, 15.0f, timedOut);
    TEST_ASSERT(timedOut.size() == 1, "Exactly 1 server must time out");
    TEST_ASSERT(timedOut[0] == s2, "Timed out server must be s2");
    TEST_ASSERT(registry.GetServer(s2)->state == ServerRegistryState::Offline, "Server s2 must be marked Offline");
    TEST_ASSERT(registry.GetAvailableServerCount() == 1, "Only 1 available server remains");

    // 6. Deregistration
    std::cout << "[TEST] 6. Server Deregistration..." << std::endl;
    TEST_ASSERT(registry.DeregisterServer(s2), "Deregistering s2 must succeed");
    TEST_ASSERT(registry.GetServerCount() == 1, "Server count must be 1");
    TEST_ASSERT(registry.DeregisterServerByConnection(10), "Deregistering s1 by connection must succeed");
    TEST_ASSERT(registry.GetServerCount() == 0, "Server count must be 0");

    std::cout << "[SUCCESS] Dedicated Server Registry Subsystem fully verified!" << std::endl;
    return 0;
}

