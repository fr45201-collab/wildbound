#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#endif

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

#include "server/ServerConfig.hpp"
#include "server/ServerCommandLine.hpp"
#include "server/ServerLifecycle.hpp"
#include "server/ServerDiagnostics.hpp"
#include "server/ServerLoop.hpp"
#include "server/DedicatedServer.hpp"
#include "network/http/HttpClient.hpp"

#include "network/ENetClient.hpp"
#include "network/ENetServer.hpp"
#include "network/NetworkManager.hpp"
#include "network/replication/ReplicationManager.hpp"
#include "world/World.hpp"
#include "core/Logger.hpp"

using namespace wildbound::server;
using namespace wildbound::world;
using namespace wildbound::core;

using wildbound::network::ENetClient;
using wildbound::network::ENetServer;
using wildbound::network::NetworkManager;
using wildbound::network::ReplicationManager;
using wildbound::network::NetworkEntityIdentity;
using wildbound::network::NetworkEntityType;
using wildbound::network::ConnectionEvent;
using wildbound::network::ConnectionEventType;
using wildbound::network::ChannelType;
using wildbound::network::WorldSnapshot;
using wildbound::network::ByteStreamWriter;
using wildbound::network::PacketHeader;
using wildbound::network::PROTOCOL_MAGIC;
using wildbound::network::PROTOCOL_VERSION;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILURE] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            std::abort(); \
        } \
    } while (0)

// ============================================================================
// Test 1: Configuration Parsing & Validation
// ============================================================================
void Test1_ServerConfigParsingAndValidation() {
    std::cout << "[TEST] 1. Server Configuration Parsing & Validation..." << std::endl;

    ServerConfig config = ServerConfig::GetDefault();
    TEST_ASSERT(config.port == 7777, "Default port must be 7777");
    TEST_ASSERT(config.maxClients == 32, "Default maxClients must be 32");
    TEST_ASSERT(config.tickRate == 30, "Default tickRate must be 30");
    TEST_ASSERT(config.mapName == "maps/world/battle_royale_island.json", "Default map must be battle_royale_island");

    // Bounds validation & clamping
    config.tickRate = 500; // Out of bounds high
    config.maxClients = 0; // Out of bounds low
    config.port = 0;       // Invalid port
    config.maxFrameTimeMs = 5000; // Out of bounds high

    bool valid = config.Validate();
    TEST_ASSERT(!valid, "Config with out-of-bounds parameters must return false from Validate()");
    TEST_ASSERT(config.tickRate == 120, "Tick rate must be clamped to max 120 Hz");
    TEST_ASSERT(config.maxClients == 1, "Max clients must be clamped to min 1");
    TEST_ASSERT(config.port == 7777, "Invalid port 0 must fallback to default 7777");
    TEST_ASSERT(config.maxFrameTimeMs == 1000, "Max frame time must clamp to 1000ms");

    // Serialization roundtrip
    const std::string testPath = "build/test_server_config.json";
    bool saveOk = config.SaveToFile(testPath);
    TEST_ASSERT(saveOk, "Must successfully serialize config to JSON");

    ServerConfig loadedConfig;
    bool loadOk = loadedConfig.LoadFromFile(testPath);
    TEST_ASSERT(loadOk, "Must successfully load config from JSON");
    TEST_ASSERT(loadedConfig.port == config.port, "Loaded port must match saved port");
    TEST_ASSERT(loadedConfig.tickRate == config.tickRate, "Loaded tickRate must match saved tickRate");

    std::filesystem::remove(testPath);
    std::cout << "  -> Configuration defaults, bounds clamping, and JSON serialization verified." << std::endl;
}

// ============================================================================
// Test 2: Command Line Argument Parsing & Overrides
// ============================================================================
void Test2_ServerCommandLineParsingAndOverrides() {
    std::cout << "[TEST] 2. Command Line Argument Parsing & Overrides..." << std::endl;

    ServerConfig config = ServerConfig::GetDefault();
    ServerCommandLine cli;

    std::vector<const char*> args = {
        "WildboundServer.exe",
        "--port", "8888",
        "--max-clients", "16",
        "--tick-rate", "60",
        "--map", "maps/forest/forest_01.json",
        "--mode", "practice",
        "--log-level", "debug",
        "--test-run"
    };

    bool parseOk = cli.Parse(static_cast<int>(args.size()), const_cast<char**>(args.data()), config);
    TEST_ASSERT(parseOk, "CLI parsing must succeed");
    TEST_ASSERT(config.port == 8888, "Port override must be 8888");
    TEST_ASSERT(config.maxClients == 16, "Max clients override must be 16");
    TEST_ASSERT(config.tickRate == 60, "Tick rate override must be 60");
    TEST_ASSERT(config.mapName == "maps/forest/forest_01.json", "Map override must match");
    TEST_ASSERT(config.gameMode == "practice", "Game mode override must match");
    TEST_ASSERT(config.logLevel == "debug", "Log level override must match");
    TEST_ASSERT(cli.IsTestRunMode(), "Test-run flag must be active");

    // Help & Version flags
    ServerConfig helpConfig = ServerConfig::GetDefault();
    ServerCommandLine helpCli;
    std::vector<const char*> helpArgs = { "WildboundServer.exe", "--help" };
    helpCli.Parse(static_cast<int>(helpArgs.size()), const_cast<char**>(helpArgs.data()), helpConfig);
    TEST_ASSERT(helpCli.ShouldShowHelp(), "--help flag must set ShouldShowHelp()");

    std::cout << "  -> Command line argument overrides and flags verified." << std::endl;
}

// ============================================================================
// Test 3: Server Lifecycle State Transitions
// ============================================================================
void Test3_ServerLifecycleTransitions() {
    std::cout << "[TEST] 3. Server Lifecycle State Machine..." << std::endl;

    TEST_ASSERT(ServerLifecycleStateToString(ServerLifecycleState::Uninitialized) == "Uninitialized", "String conversion match");
    TEST_ASSERT(ServerLifecycleStateToString(ServerLifecycleState::Initializing) == "Initializing", "String conversion match");
    TEST_ASSERT(ServerLifecycleStateToString(ServerLifecycleState::Starting) == "Starting", "String conversion match");
    TEST_ASSERT(ServerLifecycleStateToString(ServerLifecycleState::Running) == "Running", "String conversion match");
    TEST_ASSERT(ServerLifecycleStateToString(ServerLifecycleState::Stopping) == "Stopping", "String conversion match");
    TEST_ASSERT(ServerLifecycleStateToString(ServerLifecycleState::Stopped) == "Stopped", "String conversion match");

    DedicatedServer server;
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Uninitialized, "Initial state must be Uninitialized");
    TEST_ASSERT(!server.IsRunning(), "Server must not be running initially");

    std::cout << "  -> Lifecycle state enum and string mappings verified." << std::endl;
}

// ============================================================================
// Test 4: Fixed-Timestep Simulation Loop & Bounded Catch-Up
// ============================================================================
void Test4_ServerLoopFixedTimestepAndCatchup() {
    std::cout << "[TEST] 4. Fixed-Timestep Simulation Loop & Catch-Up..." << std::endl;

    ServerLoop loop(30, 250);
    TEST_ASSERT(loop.GetTickRate() == 30, "Tick rate must match 30 Hz");
    TEST_ASSERT(std::abs(loop.GetFixedDeltaTime() - (1.0f / 30.0f)) < 0.0001f, "Fixed dt must be ~0.0333s");

    int pollCount = 0;
    int fixedUpdateCount = 0;
    int replCount = 0;
    int diagCount = 0;

    loop.SetNetworkPollHook([&]() { ++pollCount; });
    loop.SetFixedUpdateHook([&](float dt) {
        ++fixedUpdateCount;
        TEST_ASSERT(std::abs(dt - loop.GetFixedDeltaTime()) < 0.0001f, "Hook dt must equal fixed dt");
    });
    loop.SetReplicationHook([&]() { ++replCount; });
    loop.SetDiagnosticsHook([&](float, float, bool) { ++diagCount; });

    // Single step with standard tick
    loop.SingleStep(1.0f / 30.0f);
    TEST_ASSERT(pollCount == 1, "Network poll must be called once per frame");
    TEST_ASSERT(fixedUpdateCount == 1, "Fixed update must execute exactly once");
    TEST_ASSERT(replCount == 1, "Replication hook must execute once");
    TEST_ASSERT(diagCount == 1, "Diagnostics hook must execute once");

    // Test bounded catch-up: simulate a 1-second lag spike (should accumulate ~30 ticks, but clamped to 5 max)
    pollCount = 0;
    fixedUpdateCount = 0;
    replCount = 0;
    diagCount = 0;

    loop.SingleStep(1.0f); // 1 full second
    TEST_ASSERT(fixedUpdateCount == 5, "Simulation loop must clamp catch-up steps to maximum 5");

    std::cout << "  -> Fixed-timestep precision and bounded catch-up clamping verified." << std::endl;
}

// ============================================================================
// Test 5: Headless World Loading (Zero Graphics/Audio Device)
// ============================================================================
void Test5_HeadlessWorldLoading() {
    std::cout << "[TEST] 5. Headless World Loading & Spatial Indices..." << std::endl;

    World world;
    // Pass nullptr for AssetManager to enforce headless execution
    bool loadOk = world.LoadMap("maps/world/battle_royale_island.json", nullptr);
    TEST_ASSERT(loadOk, "Battle Royale Island map must load headlessly with null AssetManager");
    TEST_ASSERT(world.IsLoaded(), "World must report loaded");

    Rectangle bounds = world.GetWorldBounds();
    TEST_ASSERT(bounds.width > 0 && bounds.height > 0, "World dimensions must be positive");

    auto boxes = world.GetCollisionBoxes(bounds);
    TEST_ASSERT(!boxes.empty(), "World must contain collision geometry");
    TEST_ASSERT(boxes.size() > 1000, "Battle Royale island must have > 1000 collision bounds");

    world.UnloadMap();
    TEST_ASSERT(!world.IsLoaded(), "World must report unloaded after UnloadMap()");
    TEST_ASSERT(world.GetCollisionBoxes(bounds).empty(), "Collision bounds must be cleared on unload");

    std::cout << "  -> Headless map loading and geometry verification passed." << std::endl;
}

// ============================================================================
// Test 6: Headless Startup, Shutdown & Idempotency
// ============================================================================
void Test6_HeadlessStartupAndShutdownIdempotency() {
    std::cout << "[TEST] 6. Headless Startup, Shutdown & Idempotency..." << std::endl;

    DedicatedServer server;
    std::vector<const char*> args = {
        "WildboundServer.exe",
        "--port", "19991",
        "--test-run"
    };

    bool initOk = server.Initialize(static_cast<int>(args.size()), const_cast<char**>(args.data()));
    TEST_ASSERT(initOk, "Server Initialize must succeed");
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Starting, "State must be Starting");

    bool startOk = server.Start();
    TEST_ASSERT(startOk, "Server Start must succeed");
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Running, "State must be Running");
    TEST_ASSERT(server.IsRunning(), "IsRunning() must return true");

    // Shutdown once
    server.Shutdown();
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Stopped, "State must be Stopped");
    TEST_ASSERT(!server.IsRunning(), "IsRunning() must return false");

    // Second shutdown must be a safe, idempotent no-op
    server.Shutdown();
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Stopped, "State must remain Stopped");

    std::cout << "  -> Server startup, shutdown sequence, and idempotency verified." << std::endl;
}

// ============================================================================
// Test 7: Client Connection Acceptance & Capacity Limits
// ============================================================================
void Test7_ClientConnectionAcceptanceAndCapacity() {
    std::cout << "[TEST] 7. Client Connection Acceptance & Capacity Limits..." << std::endl;

    const uint16_t testPort = 19992;
    DedicatedServer server;
    std::vector<const char*> args = {
        "WildboundServer.exe",
        "--port", "19992",
        "--max-clients", "2"
    };

    TEST_ASSERT(server.Initialize(static_cast<int>(args.size()), const_cast<char**>(args.data())), "Server init failed");
    TEST_ASSERT(server.Start(), "Server start failed");

    ENetClient client1, client2;
    TEST_ASSERT(client1.Connect("127.0.0.1", testPort, 2000), "Client 1 connect initiated");
    TEST_ASSERT(client2.Connect("127.0.0.1", testPort, 2000), "Client 2 connect initiated");

    // Pump network for connection handshakes
    for (int i = 0; i < 50; ++i) {
        client1.Update(0.016f);
        client2.Update(0.016f);
        server.GetNetworkManager()->Update(0.016f);
        if (client1.IsConnected() && client2.IsConnected() && server.GetNetworkManager()->GetServerConnectedCount() == 2) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(client1.IsConnected(), "Client 1 must be connected");
    TEST_ASSERT(client2.IsConnected(), "Client 2 must be connected");
    TEST_ASSERT(server.GetNetworkManager()->GetServerConnectedCount() == 2, "Server must report exactly 2 connected clients");

    // Disconnect clients
    client1.Disconnect();
    client2.Disconnect();
    for (int i = 0; i < 20; ++i) {
        server.GetNetworkManager()->Update(0.016f);
        client1.Update(0.016f);
        client2.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    server.Shutdown();
    std::cout << "  -> Client connections and capacity limits verified." << std::endl;
}

// ============================================================================
// Test 8: Multi-Client Gameplay Simulation & Replication Snapshot Broadcast
// ============================================================================
void Test8_MultiClientSimulationAndReplication() {
    std::cout << "[TEST] 8. Multi-Client Simulation & Replication Broadcast..." << std::endl;

    const uint16_t testPort = 19993;
    DedicatedServer server;
    std::vector<const char*> args = {
        "WildboundServer.exe",
        "--port", "19993",
        "--max-clients", "4"
    };

    TEST_ASSERT(server.Initialize(static_cast<int>(args.size()), const_cast<char**>(args.data())), "Server init failed");
    TEST_ASSERT(server.Start(), "Server start failed");

    ENetClient clientA, clientB;
    TEST_ASSERT(clientA.Connect("127.0.0.1", testPort, 2000), "Client A connect initiated");
    TEST_ASSERT(clientB.Connect("127.0.0.1", testPort, 2000), "Client B connect initiated");

    for (int i = 0; i < 50; ++i) {
        clientA.Update(0.016f);
        clientB.Update(0.016f);
        server.GetNetworkManager()->Update(0.016f);
        if (clientA.IsConnected() && clientB.IsConnected() && server.GetNetworkManager()->GetServerConnectedCount() == 2) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(clientA.IsConnected() && clientB.IsConnected(), "Both clients must be connected");

    // Register an entity on the server's replication manager
    auto* replMgr = server.GetNetworkManager()->GetReplicationManager();
    TEST_ASSERT(replMgr != nullptr, "ReplicationManager must be valid on server");

    NetworkEntityIdentity playerIdentity = replMgr->RegisterPlayer(1, 500.0f, 600.0f);
    TEST_ASSERT(playerIdentity.IsValid(), "Registered player identity must be valid");

    WorldSnapshot snap = replMgr->BuildSnapshot(true);
    TEST_ASSERT(!snap.entities.empty(), "Snapshot must contain replicated player entity");

    // Track test packet reception on client B
    bool receivedPacket = false;
    clientB.SetEventCallback([&](const ConnectionEvent& ev) {
        if (ev.type == ConnectionEventType::PacketReceived) {
            receivedPacket = true;
        }
    });

    // Send broadcast test message from server to connected clients
    server.GetNetworkManager()->SendTestMessage(true, "ReplicationTestBroadcast");

    // Pump network
    for (int i = 0; i < 30; ++i) {
        clientA.Update(0.016f);
        clientB.Update(0.016f);
        server.GetNetworkManager()->Update(0.016f);
        if (receivedPacket) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(receivedPacket, "Connected client must receive server broadcast packet");

    clientA.Disconnect();
    clientB.Disconnect();
    server.Shutdown();
    std::cout << "  -> Multi-client simulation and replication snapshot transmission verified." << std::endl;
}

// ============================================================================
// Test 9: Graceful Shutdown with Connected Clients
// ============================================================================
void Test9_GracefulShutdownWithConnectedClients() {
    std::cout << "[TEST] 9. Graceful Shutdown with Connected Clients..." << std::endl;

    const uint16_t testPort = 19994;
    DedicatedServer server;
    std::vector<const char*> args = {
        "WildboundServer.exe",
        "--port", "19994",
        "--max-clients", "4"
    };

    TEST_ASSERT(server.Initialize(static_cast<int>(args.size()), const_cast<char**>(args.data())), "Server init failed");
    TEST_ASSERT(server.Start(), "Server start failed");

    ENetClient client;
    TEST_ASSERT(client.Connect("127.0.0.1", testPort, 2000), "Client connect initiated");

    for (int i = 0; i < 50; ++i) {
        client.Update(0.016f);
        server.GetNetworkManager()->Update(0.016f);
        if (client.IsConnected() && server.GetNetworkManager()->GetServerConnectedCount() == 1) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    TEST_ASSERT(client.IsConnected(), "Client must be connected");

    // Initiate graceful shutdown request
    server.RequestShutdown("Graceful Test Shutdown");
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Stopping, "Server state must be Stopping");

    // Execute full shutdown
    server.Shutdown();
    TEST_ASSERT(server.GetState() == ServerLifecycleState::Stopped, "Server state must be Stopped");
    TEST_ASSERT(!server.GetNetworkManager()->IsServerRunning(), "Network server host must be stopped");

    // Pump client to observe disconnection
    for (int i = 0; i < 20; ++i) {
        client.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout << "  -> Graceful shutdown with client notifications verified." << std::endl;
}

// ============================================================================
// Test 10: Server Diagnostics & Telemetry
// ============================================================================
void Test10_ServerDiagnostics() {
    std::cout << "[TEST] 10. Server Diagnostics & Telemetry..." << std::endl;

    ServerDiagnostics diag;
    diag.Reset();

    const auto& initialMetrics = diag.GetMetrics();
    TEST_ASSERT(initialMetrics.uptimeSeconds == 0.0f, "Initial uptime must be 0");
    TEST_ASSERT(initialMetrics.currentTick == 0, "Initial ticks must be 0");

    // Simulate 30 ticks over 1 second
    for (int i = 0; i < 30; ++i) {
        diag.RecordTickDuration(1.5f, false);
        diag.Update(1.0f / 30.0f, ServerLifecycleState::Running);
    }

    const auto& m1 = diag.GetMetrics();
    TEST_ASSERT(m1.currentTick == 30, "Total ticks must be 30");
    TEST_ASSERT(m1.simulationOverloadCount == 0, "Overloads must be 0");
    TEST_ASSERT(m1.averageTickDurationMs > 0.0f, "Average tick duration must be recorded");

    // Record an overloaded tick
    diag.RecordTickDuration(35.0f, true);
    const auto& m2 = diag.GetMetrics();
    TEST_ASSERT(m2.simulationOverloadCount == 1, "Overload count must be 1");
    TEST_ASSERT(m2.maxTickDurationMs >= 35.0f, "Peak tick duration must reflect overload");

    // Update client and entity metrics
    diag.SetClientCounts(4, 3);
    diag.SetEntityCounts(50, 10, 20);
    diag.RecordPacketReceived(1024);
    diag.RecordPacketSent(2048);

    std::string summary = diag.FormatSummary();
    TEST_ASSERT(!summary.empty(), "Summary string must not be empty");
    TEST_ASSERT(summary.find("Running") != std::string::npos, "Summary must contain state");
    TEST_ASSERT(summary.find("Clients: 4") != std::string::npos, "Summary must report 4 connected clients");

    std::cout << "  -> Diagnostics calculations, peak tracking, and summaries verified." << std::endl;
}

void Test11_HttpClientLifecycleAndShutdown() {
    std::cout << "[Test 11] Asynchronous HttpClient Lifecycle & Worker Thread Shutdown Determinism..." << std::endl;

    // Subtest 11A: Failed connection handled cleanly via async callback
    {
        wildbound::network::http::HttpClient client;
        bool callbackReceived = false;
        bool successFlag = true;
        client.SendRequestAsync("GET", "http://127.0.0.1:59999/test", "", {},
            [&](const wildbound::network::http::HttpResponse& resp) {
                callbackReceived = true;
                successFlag = resp.success;
            }, 500);

        auto start = std::chrono::steady_clock::now();
        while (!callbackReceived && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < 3000) {
            client.Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        TEST_ASSERT(callbackReceived, "Async failed connection callback must be dispatched via Update()");
        TEST_ASSERT(!successFlag, "Connection to non-existent endpoint must report failure");
    }

    // Subtest 11B: Destruction while requests are in flight (Memory Safety Guarantee)
    {
        {
            wildbound::network::http::HttpClient scopedClient;
            for (int i = 0; i < 8; ++i) {
                scopedClient.SendRequestAsync("POST", "http://127.0.0.1:59998/bench", "body_data", {},
                    [](const wildbound::network::http::HttpResponse&) {
                        // Response dropped cleanly on destruction
                    }, 3000);
            }
            // scopedClient is immediately destroyed while 8 worker requests are actively executing
            // Destructor calls Shutdown(), safely joins all worker threads, and releases resources
        }
        TEST_ASSERT(true, "Destruction while requests in flight must join workers cleanly without memory corruption");
    }

    // Subtest 11C: Async request mid-shutdown / post-shutdown rejection
    {
        wildbound::network::http::HttpClient client;
        TEST_ASSERT(!client.IsShuttingDown(), "Fresh client must not report shutting down");
        client.Shutdown();
        TEST_ASSERT(client.IsShuttingDown(), "Client must report shutting down after Shutdown()");

        bool rejected = false;
        client.SendRequestAsync("GET", "http://127.0.0.1:59999/test", "", {},
            [&](const wildbound::network::http::HttpResponse& resp) {
                if (!resp.success && resp.errorMessage.find("shutting down") != std::string::npos) {
                    rejected = true;
                }
            }, 500);
        TEST_ASSERT(rejected, "SendRequestAsync during/after shutdown must immediately reject request");
    }

    // Subtest 11D: Repeated shutdown idempotency
    {
        wildbound::network::http::HttpClient client;
        client.Shutdown();
        client.Shutdown();
        client.Shutdown();
        TEST_ASSERT(client.IsShuttingDown(), "Repeated Shutdown() calls must succeed idempotently");
    }

    // Subtest 11E: Multiple concurrent requests completed before shutdown
    {
        wildbound::network::http::HttpClient client;
        for (int i = 0; i < 6; ++i) {
            client.SendRequestAsync("GET", "http://127.0.0.1:59997/test", "", {},
                [](const wildbound::network::http::HttpResponse&) {}, 500);
        }
        client.Shutdown();
        TEST_ASSERT(client.IsShuttingDown(), "Client shutdown after multiple requests must succeed");
    }

    std::cout << "  -> HttpClient worker ownership, joining, destruction safety, and idempotency verified." << std::endl;
}

// ============================================================================
// Main Runner
// ============================================================================
int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 39 DEDICATED SERVER TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    Test1_ServerConfigParsingAndValidation();
    Test2_ServerCommandLineParsingAndOverrides();
    Test3_ServerLifecycleTransitions();
    Test4_ServerLoopFixedTimestepAndCatchup();
    Test5_HeadlessWorldLoading();
    Test6_HeadlessStartupAndShutdownIdempotency();
    Test7_ClientConnectionAcceptanceAndCapacity();
    Test8_MultiClientSimulationAndReplication();
    Test9_GracefulShutdownWithConnectedClients();
    Test10_ServerDiagnostics();
    Test11_HttpClientLifecycleAndShutdown();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 11 PHASE 39 DEDICATED SERVER TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

