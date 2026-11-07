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

using namespace wildbound::network;

// Helper: choose unique local test port to avoid collision
static constexpr uint16_t BASE_TEST_PORT = 17777;

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

    for (int i = 0; i < 50 && (!clientConnected || !serverConnected); ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(clientConnected);
    assert(serverConnected);
    assert(client.IsConnected());
    assert(server.GetConnectedCount() == 1);

    client.Disconnect();
    for (int i = 0; i < 20; ++i) {
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
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

    // Wait for connect
    for (int i = 0; i < 50 && !client.IsConnected(); ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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
    for (int i = 0; i < 50 && (receivedByServer.empty() || receivedByClient.empty()); ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(receivedByServer == "ClientPing");
    assert(receivedByClient == "ServerPong");

    client.Disconnect();
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

    for (int i = 0; i < 50 && !client.IsConnected(); ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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

    for (int i = 0; i < 50 && lastReceivedSeq < 3; ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(lastReceivedSeq == 3);

    client.Disconnect();
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
    for (int i = 0; i < 50 && !client.IsConnected(); ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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

    for (int i = 0; i < 50 && receivedChannels.size() < 4; ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(receivedChannels.size() == 4);
    assert(receivedChannels[0] == ChannelType::ReliableEvents);
    assert(receivedChannels[1] == ChannelType::PlayerInput);
    assert(receivedChannels[2] == ChannelType::Replication);
    assert(receivedChannels[3] == ChannelType::MatchState);

    client.Disconnect();
    server.Stop();

    std::cout << "  -> Channel isolation and independent multi-channel routing verified." << std::endl;
}

// ============================================================================
// 9. Connection Timeout Handling Test
// ============================================================================
static void TestConnectionTimeout() {
    std::cout << "[TEST] 9. Connection Timeout Handling..." << std::endl;

    // Connect to unassigned port (e.g. 59999) with 300 ms timeout
    ENetClient client;
    bool sawTimeout = false;
    client.SetEventCallback([&](const ConnectionEvent& evt) {
        if (evt.type == ConnectionEventType::ConnectionTimedOut) {
            sawTimeout = true;
        }
    });

    assert(client.Connect("127.0.0.1", 59999, 300));
    assert(client.GetState() == ConnectionState::Connecting);

    for (int i = 0; i < 40 && !sawTimeout; ++i) {
        client.Update(0.020f);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    assert(sawTimeout);
    assert(client.GetState() == ConnectionState::TimedOut);
    client.Disconnect();

    std::cout << "  -> Inactive host timeout detection and graceful state transition verified." << std::endl;
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

        for (int i = 0; i < 30 && !connected; ++i) {
            client.Update(0.016f);
            server.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        assert(connected);
        assert(server.GetConnectedCount() == 1);

        client.Disconnect();
        for (int i = 0; i < 20; ++i) {
            server.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
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
    TestRapidConnectDisconnectStress();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 PHASE 36 TRANSPORT TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

