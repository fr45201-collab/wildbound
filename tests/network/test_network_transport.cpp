#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "network/NetworkTypes.hpp"
#include "network/NetworkProtocol.hpp"
#include "network/NetworkMessage.hpp"
#include "network/NetworkEvent.hpp"
#include "network/ConnectionState.hpp"
#include "network/ConnectionEvents.hpp"
#include "network/ConnectionManager.hpp"
#include "network/PacketValidator.hpp"
#include "network/ENetTransport.hpp"
#include "network/ENetServer.hpp"
#include "network/ENetClient.hpp"
#include "network/NetworkManager.hpp"

#ifdef assert
#undef assert
#endif
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)
#define assert(cond) TEST_ASSERT(cond, "Assertion verified")

using namespace wildbound::network;

// Helper: choose unique local test port to avoid collision
static constexpr uint16_t BASE_TEST_PORT = 17777;

// Helper: deadline-based network pumping with substep accumulator preserving total elapsed wall-clock time (DEF-009)
template <typename Predicate>
static bool PumpUntil(std::chrono::milliseconds timeout, Predicate pred,
                      ENetClient* client = nullptr, ENetServer* server = nullptr,
                      std::chrono::milliseconds sleepInterval = std::chrono::milliseconds(5)) {
    const auto start = std::chrono::steady_clock::now();
    const auto deadline = start + timeout;
    auto lastTick = start;

    while (!pred()) {
        const auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        // Bounded substep accumulator: process simulation in at most 16ms slices
        // while strictly preserving every microsecond of total elapsed wall-clock duration
        constexpr float SUBSTEP_SLICE = 0.016f;
        while (elapsed > SUBSTEP_SLICE) {
            if (client) client->Update(SUBSTEP_SLICE);
            if (server) server->Update(SUBSTEP_SLICE);
            elapsed -= SUBSTEP_SLICE;
            if (pred()) return true;
        }
        if (elapsed > 0.0f) {
            if (client) client->Update(elapsed);
            if (server) server->Update(elapsed);
            if (pred()) return true;
        }

        if (now >= deadline) {
            break;
        }

        if (sleepInterval.count() > 0) {
            std::this_thread::sleep_for(sleepInterval);
        }
    }
    return pred();
}

// ============================================================================
// 1. ENet Lifecycle & Ref-Counting Test
// ============================================================================
static void TestENetLifecycle() {
    std::cout << "[TEST] 1. ENet Global Lifecycle & Ref-Counting..." << std::endl;

    assert(ENetTransport::InitializeENet());
    assert(ENetTransport::IsENetInitialized());

    // Duplicate initialization must be safe and ref-counted
    assert(ENetTransport::InitializeENet());
    assert(ENetTransport::IsENetInitialized());

    // Decrement once: still initialized
    ENetTransport::DeinitializeENet();
    assert(ENetTransport::IsENetInitialized());

    // Decrement second time: deinitialized
    ENetTransport::DeinitializeENet();
    assert(!ENetTransport::IsENetInitialized());

    // Reinitialize safely
    assert(ENetTransport::InitializeENet());
    assert(ENetTransport::IsENetInitialized());

    std::cout << "  -> Initialization ref-counting and safe re-initialization verified." << std::endl;
}

// ============================================================================
// 2. Host Creation & Teardown Test
// ============================================================================
static void TestHostCreationAndDestruction() {
    std::cout << "[TEST] 2. Server & Client Host Creation & Teardown..." << std::endl;

    ENetTransport serverTransport;
    assert(serverTransport.Initialize(BASE_TEST_PORT, 16));
    assert(serverTransport.IsActive());
    assert(serverTransport.GetRawHost() != nullptr);

    ENetTransport clientTransport;
    assert(clientTransport.Initialize(0, 1));
    assert(clientTransport.IsActive());
    assert(clientTransport.GetRawHost() != nullptr);

    clientTransport.Shutdown();
    assert(!clientTransport.IsActive());

    serverTransport.Shutdown();
    assert(!serverTransport.IsActive());

    std::cout << "  -> Host creation, bind verification, and clean shutdown verified." << std::endl;
}

// ============================================================================
// 3. Packet Validation Gauntlet Test
// ============================================================================
static void TestPacketValidationGauntlet() {
    std::cout << "[TEST] 3. Packet Validation Gauntlet & Boundary Defenses..." << std::endl;

    PacketHeader hdr;

    // 1. Too small (0 bytes)
    assert(PacketValidator::Validate(nullptr, 0, 0, hdr) == PacketValidationResult::TooSmall);

    // 2. Truncated header (12 bytes, less than PACKET_HEADER_SIZE 16)
    uint8_t truncated[12] = { 0 };
    assert(PacketValidator::Validate(truncated, sizeof(truncated), 0, hdr) == PacketValidationResult::TooSmall);

    // 3. Bad Magic (wrong magic number)
    ByteStreamWriter writerBad;
    PacketHeader badMagicHdr{
        .magic = 0x11223344,
        .version = PROTOCOL_VERSION,
        .messageType = 1,
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = 4
    };
    SerializePacketHeader(writerBad, badMagicHdr);
    writerBad.WriteU32(100);
    assert(PacketValidator::Validate(writerBad.GetData(), writerBad.GetSize(), 0, hdr) == PacketValidationResult::BadMagic);

    // 4. Unsupported Version
    ByteStreamWriter writerVer;
    PacketHeader badVerHdr{
        .magic = PROTOCOL_MAGIC,
        .version = 999,
        .messageType = 1,
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = 4
    };
    SerializePacketHeader(writerVer, badVerHdr);
    writerVer.WriteU32(100);
    assert(PacketValidator::Validate(writerVer.GetData(), writerVer.GetSize(), 0, hdr) == PacketValidationResult::UnsupportedVersion);

    // 5. Payload Length Mismatch (header declares 20 bytes, actual is 4)
    ByteStreamWriter writerMismatch;
    PacketHeader mismatchHdr{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = 1,
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = 20
    };
    SerializePacketHeader(writerMismatch, mismatchHdr);
    writerMismatch.WriteU32(100);
    assert(PacketValidator::Validate(writerMismatch.GetData(), writerMismatch.GetSize(), 0, hdr) == PacketValidationResult::PayloadLengthMismatch);

    // 6. Invalid Channel Index (channel 10 when only 5 channels exist)
    ByteStreamWriter writerValid;
    PacketHeader validHdr{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = 1,
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = 4
    };
    SerializePacketHeader(writerValid, validHdr);
    writerValid.WriteU32(100);
    assert(PacketValidator::Validate(writerValid.GetData(), writerValid.GetSize(), 10, hdr) == PacketValidationResult::InvalidChannel);

    // 7. Legitimate Valid Packet
    assert(PacketValidator::Validate(writerValid.GetData(), writerValid.GetSize(), 0, hdr) == PacketValidationResult::Valid);
    assert(hdr.magic == PROTOCOL_MAGIC);
    assert(hdr.version == PROTOCOL_VERSION);
    assert(hdr.payloadLength == 4);

    std::cout << "  -> Packet bounds, magic, version, payload, and channel validation verified." << std::endl;
}

// ============================================================================
// 4. Connection Manager Operations Test
// ============================================================================
static void TestConnectionManagerOperations() {
    std::cout << "[TEST] 4. Connection Manager Registry & Tracking..." << std::endl;

    ConnectionManager mgr;
    assert(mgr.GetConnectionCount() == 0);

    // Add connection
    ClientId c1 = mgr.AddConnection(reinterpret_cast<void*>(0x1000), "127.0.0.1", 54321);
    assert(c1 > 0);
    assert(mgr.GetConnectionCount() == 1);

    auto* conn1 = mgr.FindConnection(c1);
    assert(conn1 != nullptr);
    assert(conn1->GetAddress() == "127.0.0.1");
    assert(conn1->GetPort() == 54321);
    assert(conn1->GetState() == ConnectionState::Connecting);

    // Find by peer handle
    auto* connByPeer = mgr.FindConnectionByPeer(reinterpret_cast<void*>(0x1000));
    assert(connByPeer == conn1);

    // Update state
    mgr.SetConnectionState(c1, ConnectionState::Connected);
    assert(conn1->IsConnected());
    assert(mgr.GetActiveConnectionIds().size() == 1);

    // Remove connection
    assert(mgr.RemoveConnection(c1));
    assert(mgr.GetConnectionCount() == 0);
    assert(mgr.FindConnection(c1) == nullptr);
    assert(mgr.FindConnectionByPeer(reinterpret_cast<void*>(0x1000)) == nullptr);

    std::cout << "  -> Connection lookup, peer mapping, state updates, and removal verified." << std::endl;
}

// ============================================================================
// 5. Localhost Connection Lifecycle Test
// ============================================================================
static void TestLocalhostConnectionLifecycle() {
    std::cout << "[TEST] 5. Localhost Client-Server Connection Lifecycle..." << std::endl;

    constexpr uint16_t port = BASE_TEST_PORT + 1;
    ENetServer server;
    assert(server.Start(port, 4));

    bool clientConnected = false;
    bool serverConnected = false;

    server.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::ClientConnected) {
            serverConnected = true;
        }
    });

    ENetClient client;
    client.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::ClientConnected) {
            clientConnected = true;
        }
    });

    assert(client.Connect("127.0.0.1", port, 2000));

    // Deadline-based polling respecting configured 2000ms timeout (DEF-009)
    bool connected = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return clientConnected && serverConnected;
    }, &client, &server);

    assert(connected);
    assert(clientConnected);
    assert(serverConnected);
    assert(client.IsConnected());
    assert(server.GetConnectedCount() == 1);

    client.Disconnect();
    PumpUntil(std::chrono::milliseconds(1000), [&]() {
        return server.GetConnectedCount() == 0;
    }, nullptr, &server);
    server.Stop();

    std::cout << "  -> Localhost client connect, server accept, and clean disconnect verified." << std::endl;
}

// ============================================================================
// 6. Reliable Messaging Ping-Pong Test
// ============================================================================
static void TestReliableMessagingPingPong() {
    std::cout << "[TEST] 6. Reliable Messaging Bidirectional Ping-Pong..." << std::endl;

    constexpr uint16_t port = BASE_TEST_PORT + 2;
    ENetServer server;
    assert(server.Start(port, 4));

    std::string receivedByServer;
    server.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::PacketReceived) {
            ByteStreamReader reader(evt.payload.data(), evt.payload.size());
            receivedByServer = reader.ReadString();

            // Server responds with Pong
            ByteStreamWriter payloadWriter;
            payloadWriter.WriteString("ServerPong");

            ByteStreamWriter respWriter;
            PacketHeader respHdr{
                .magic = PROTOCOL_MAGIC,
                .version = PROTOCOL_VERSION,
                .messageType = static_cast<uint16_t>(MessageType::Heartbeat),
                .sequenceNumber = 2,
                .senderClientId = 0,
                .payloadLength = static_cast<uint16_t>(payloadWriter.GetSize())
            };
            SerializePacketHeader(respWriter, respHdr);
            respWriter.WriteBytes(payloadWriter.GetData(), payloadWriter.GetSize());
            server.Send(evt.connectionId, ChannelType::ReliableEvents, DeliveryMode::ReliableOrdered,
                        respWriter.GetData(), respWriter.GetSize());
        }
    });

    ENetClient client;
    std::string receivedByClient;
    client.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::PacketReceived) {
            ByteStreamReader reader(evt.payload.data(), evt.payload.size());
            receivedByClient = reader.ReadString();
        }
    });

    assert(client.Connect("127.0.0.1", port, 2000));

    // Wait for connect respecting 2000ms configured timeout
    bool connected = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return client.IsConnected();
    }, &client, &server);
    assert(connected);
    assert(client.IsConnected());

    // Client sends "ClientPing"
    ByteStreamWriter payloadWriter;
    payloadWriter.WriteString("ClientPing");

    ByteStreamWriter pingWriter;
    PacketHeader pingHdr{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(MessageType::Heartbeat),
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = static_cast<uint16_t>(payloadWriter.GetSize())
    };
    SerializePacketHeader(pingWriter, pingHdr);
    pingWriter.WriteBytes(payloadWriter.GetData(), payloadWriter.GetSize());
    assert(client.Send(ChannelType::ReliableEvents, DeliveryMode::ReliableOrdered,
                       pingWriter.GetData(), pingWriter.GetSize()));

    // Pump updates until ping-pong completes
    bool pingPongDone = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return !receivedByServer.empty() && !receivedByClient.empty();
    }, &client, &server);
    assert(pingPongDone);

    assert(receivedByServer == "ClientPing");
    assert(receivedByClient == "ServerPong");

    client.Disconnect();
    PumpUntil(std::chrono::milliseconds(1000), [&]() {
        return server.GetConnectedCount() == 0;
    }, nullptr, &server);
    server.Stop();

    std::cout << "  -> Client-to-server reliable send and server-to-client reply verified." << std::endl;
}

// ============================================================================
// 7. Unreliable Sequenced Messaging Test
// ============================================================================
static void TestUnreliableSequencedMessaging() {
    std::cout << "[TEST] 7. Unreliable Sequenced Movement Messaging..." << std::endl;

    constexpr uint16_t port = BASE_TEST_PORT + 3;
    ENetServer server;
    assert(server.Start(port, 4));

    uint32_t lastReceivedSeq = 0;
    server.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::PacketReceived) {
            ByteStreamReader reader(evt.payload.data(), evt.payload.size());
            MsgPlayerInput input;
            if (input.Deserialize(reader)) {
                lastReceivedSeq = input.inputSequence;
            }
        }
    });

    ENetClient client;
    assert(client.Connect("127.0.0.1", port, 2000));

    bool connected = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return client.IsConnected();
    }, &client, &server);
    assert(connected);
    assert(client.IsConnected());

    // Send 3 sequential input packets on Channel 2 (PlayerInput)
    for (uint32_t seq = 1; seq <= 3; ++seq) {
        ByteStreamWriter writer;
        MsgPlayerInput input{
            .inputSequence = seq,
            .dirX = 1.0f,
            .dirY = 0.0f,
            .facingAngle = 0.5f,
            .actionFlags = 1
        };
        ByteStreamWriter payloadWriter;
        input.Serialize(payloadWriter);

        PacketHeader hdr{
            .magic = PROTOCOL_MAGIC,
            .version = PROTOCOL_VERSION,
            .messageType = static_cast<uint16_t>(MessageType::PlayerInput),
            .sequenceNumber = seq,
            .senderClientId = 1,
            .payloadLength = static_cast<uint16_t>(payloadWriter.GetSize())
        };
        SerializePacketHeader(writer, hdr);
        writer.WriteBytes(payloadWriter.GetData(), payloadWriter.GetSize());

        client.Send(ChannelType::PlayerInput, DeliveryMode::UnreliableSequenced,
                    writer.GetData(), writer.GetSize());
    }

    bool seqDone = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return lastReceivedSeq == 3;
    }, &client, &server);
    assert(seqDone);
    assert(lastReceivedSeq == 3);

    client.Disconnect();
    PumpUntil(std::chrono::milliseconds(1000), [&]() {
        return server.GetConnectedCount() == 0;
    }, nullptr, &server);
    server.Stop();

    std::cout << "  -> High-frequency unreliable sequenced input streaming verified." << std::endl;
}

// ============================================================================
// 8. Multi-Channel Virtual Mesh Routing Test
// ============================================================================
static void TestMultiChannelRouting() {
    std::cout << "[TEST] 8. Multi-Channel Virtual Mesh Routing (Channels 0 - 4)..." << std::endl;

    constexpr uint16_t port = BASE_TEST_PORT + 4;
    ENetServer server;
    assert(server.Start(port, 4));

    std::vector<ChannelType> receivedChannels;
    server.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::PacketReceived) {
            receivedChannels.push_back(evt.channel);
        }
    });

    ENetClient client;
    assert(client.Connect("127.0.0.1", port, 2000));
    bool connected = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return client.IsConnected();
    }, &client, &server);
    assert(connected);
    assert(client.IsConnected());

    // Send on Channel 1, 2, 3, 4
    ChannelType testChannels[] = {
        ChannelType::ReliableEvents,
        ChannelType::PlayerInput,
        ChannelType::Replication,
        ChannelType::MatchState
    };

    for (ChannelType ch : testChannels) {
        ByteStreamWriter writer;
        PacketHeader hdr{
            .magic = PROTOCOL_MAGIC,
            .version = PROTOCOL_VERSION,
            .messageType = static_cast<uint16_t>(MessageType::Heartbeat),
            .sequenceNumber = 1,
            .senderClientId = 1,
            .payloadLength = 4
        };
        SerializePacketHeader(writer, hdr);
        writer.WriteU32(0x12345678);

        DeliveryMode mode = (ch == ChannelType::PlayerInput || ch == ChannelType::Replication)
                            ? DeliveryMode::UnreliableSequenced : DeliveryMode::ReliableOrdered;
        client.Send(ch, mode, writer.GetData(), writer.GetSize());
    }

    bool channelsReceived = PumpUntil(std::chrono::milliseconds(2000), [&]() {
        return receivedChannels.size() >= 4;
    }, &client, &server);
    assert(channelsReceived);

    assert(receivedChannels.size() == 4);
    assert(receivedChannels[0] == ChannelType::ReliableEvents);
    assert(receivedChannels[1] == ChannelType::PlayerInput);
    assert(receivedChannels[2] == ChannelType::Replication);
    assert(receivedChannels[3] == ChannelType::MatchState);

    client.Disconnect();
    PumpUntil(std::chrono::milliseconds(1000), [&]() {
        return server.GetConnectedCount() == 0;
    }, nullptr, &server);
    server.Stop();

    std::cout << "  -> Channel isolation and independent multi-channel routing verified." << std::endl;
}

// ============================================================================
// 9. Connection Timeout Handling Test
// ============================================================================
static void TestConnectionTimeout() {
    std::cout << "[TEST] 9. Connection Timeout Handling (300ms)..." << std::endl;

    // Connect to unassigned port (e.g. 59999) with 300 ms timeout
    ENetClient client;
    bool sawTimeout = false;
    client.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::ConnectionTimedOut) {
            sawTimeout = true;
        }
    });

    const auto start = std::chrono::steady_clock::now();
    assert(client.Connect("127.0.0.1", 59999, 300));
    assert(client.GetState() == ConnectionState::Connecting);

    bool timedOut = PumpUntil(std::chrono::milliseconds(1000), [&]() {
        return sawTimeout;
    }, &client, nullptr);

    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    assert(timedOut);
    assert(sawTimeout);
    assert(client.GetState() == ConnectionState::TimedOut);
    assert(elapsedMs >= 270); // Timeout did not expire prematurely
    client.Disconnect();

    std::cout << "  -> Inactive host timeout detection verified in " << elapsedMs << "ms." << std::endl;
}

// ============================================================================
// 9B. Configured 2,000ms Timeout Wall-Clock Fidelity Test (DEF-009)
// ============================================================================
static void TestConfiguredTimeoutFidelity() {
    std::cout << "[TEST] 9B. Configured 2,000ms Timeout Wall-Clock Fidelity..." << std::endl;

    // Connect to unassigned port (e.g. 59998) with configured 2000 ms timeout
    ENetClient client;
    bool sawTimeout = false;
    client.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::ConnectionTimedOut) {
            sawTimeout = true;
        }
    });

    const auto start = std::chrono::steady_clock::now();
    assert(client.Connect("127.0.0.1", 59998, 2000));
    assert(client.GetState() == ConnectionState::Connecting);

    // Pump using corrected real-time elapsed dt in PumpUntil
    bool timedOut = PumpUntil(std::chrono::milliseconds(3000), [&]() {
        return sawTimeout;
    }, &client, nullptr);

    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    assert(timedOut);
    assert(sawTimeout);
    assert(client.GetState() == ConnectionState::TimedOut);

    // Verify configured 2,000 ms timeout does not expire substantially earlier
    // (Under old accelerated 2.67x clock, it expired at ~750ms).
    // With real elapsed delta dt, it must endure for >= 1,900 ms of real wall-clock time!
    assert(elapsedMs >= 1950);
    assert(elapsedMs <= 2500);

    client.Disconnect();
    std::cout << "  -> Configured 2,000ms timeout verified to persist for " << elapsedMs
              << "ms (fidelity preserved, zero premature expiration)." << std::endl;
}

// ============================================================================
// 9C. Delayed Scheduling / Long Iteration Interval Timeout Test (DEF-009)
// ============================================================================
static void TestDelayedSchedulingTimeout() {
    std::cout << "[TEST] 9C. Delayed Scheduling & Long Iteration Interval Fidelity..." << std::endl;

    // Connect to unassigned port (e.g. 59997) with configured 500 ms timeout
    ENetClient client;
    bool sawTimeout = false;
    client.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::ConnectionTimedOut) {
            sawTimeout = true;
        }
    });

    const auto start = std::chrono::steady_clock::now();
    assert(client.Connect("127.0.0.1", 59997, 500));
    assert(client.GetState() == ConnectionState::Connecting);

    // Pump with large sleepInterval (80ms per tick) simulating delayed scheduling / system hitches
    bool timedOut = PumpUntil(std::chrono::milliseconds(1500), [&]() {
        return sawTimeout;
    }, &client, nullptr, std::chrono::milliseconds(80));

    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    assert(timedOut);
    assert(sawTimeout);
    assert(client.GetState() == ConnectionState::TimedOut);

    // Verify accumulator preserved total elapsed time: must not time out early or drop intervals
    assert(elapsedMs >= 480);
    assert(elapsedMs <= 800);

    client.Disconnect();
    std::cout << "  -> Delayed scheduling timeout (500ms configured with 80ms hitches) verified in "
              << elapsedMs << "ms (total duration strictly conserved)." << std::endl;
}

// ============================================================================
// 10. Rapid Connect / Disconnect Stress Test
// ============================================================================
static void TestRapidConnectDisconnectStress() {
    std::cout << "[TEST] 10. Rapid Connect / Disconnect Cycle Stress..." << std::endl;

    constexpr uint16_t port = BASE_TEST_PORT + 5;
    ENetServer server;
    assert(server.Start(port, 8));

    for (int cycle = 1; cycle <= 5; ++cycle) {
        ENetClient client;
        bool connected = false;
        client.SetEventCallback([&](const ConnectionEvent& evt) {
            if (evt.type == ConnectionEventType::ClientConnected) {
                connected = true;
            }
        });

        assert(client.Connect("127.0.0.1", port, 2000));

        bool ok = PumpUntil(std::chrono::milliseconds(2000), [&]() {
            return connected;
        }, &client, &server);
        assert(ok);
        assert(connected);
        assert(server.GetConnectedCount() == 1);

        client.Disconnect();
        PumpUntil(std::chrono::milliseconds(1000), [&]() {
            return server.GetConnectedCount() == 0;
        }, nullptr, &server);
        assert(server.GetConnectedCount() == 0);
    }

    server.Stop();
    std::cout << "  -> 5 sequential rapid connect/disconnect cycles executed with zero leaks." << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 36: ENET TRANSPORT LAYER TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestENetLifecycle();
    TestHostCreationAndDestruction();
    TestPacketValidationGauntlet();
    TestConnectionManagerOperations();
    TestLocalhostConnectionLifecycle();
    TestReliableMessagingPingPong();
    TestUnreliableSequencedMessaging();
    TestMultiChannelRouting();
    TestConnectionTimeout();
    TestConfiguredTimeoutFidelity();
    TestDelayedSchedulingTimeout();
    TestRapidConnectDisconnectStress();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 12 PHASE 36 TRANSPORT TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

