#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <memory>

#include "network/NetworkTypes.hpp"
#include "network/NetworkProtocol.hpp"
#include "network/NetworkMessage.hpp"
#include "network/NetworkEvent.hpp"
#include "network/ReplicationTypes.hpp"
#include "network/NetworkConfiguration.hpp"
#include "network/INetworkTransport.hpp"

using namespace wildbound::network;

// ============================================================================
// 1. Network Entity Identity & Generational Invalidation Test
// ============================================================================
static void TestNetworkEntityIdentity() {
    std::cout << "[TEST] 1. Network Entity Identity & Generational Invalidation..." << std::endl;

    NetworkEntityIdentity id1{ .id = NetworkEntityId{ 42 }, .generation = 1 };
    assert(id1.IsValid());
    assert(id1.id.value == 42);
    assert(id1.generation == 1);

    // Test handle packing / unpacking
    uint64_t handle = id1.ToHandle();
    NetworkEntityIdentity unpacked = NetworkEntityIdentity::FromHandle(handle);
    assert(unpacked == id1);
    assert(unpacked.id.value == 42);
    assert(unpacked.generation == 1);

    // Test generation increment on destruction / recreation
    NetworkEntityIdentity id1NextGen{ .id = NetworkEntityId{ 42 }, .generation = 2 };
    assert(id1NextGen.IsValid());
    assert(id1 != id1NextGen); // Stale check must fail equality
    assert(id1NextGen.generation > id1.generation);

    // Invalid sentinel identity
    NetworkEntityIdentity invalidId{};
    assert(!invalidId.IsValid());

    std::cout << "  -> Generational entity handle packing and stale invalidation verified." << std::endl;
}

// ============================================================================
// 2. Protocol Magic, Versioning & Header Serialization Test
// ============================================================================
static void TestProtocolHeaderSerialization() {
    std::cout << "[TEST] 2. Protocol Magic, Versioning & Header Serialization..." << std::endl;

    PacketHeader header{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(MessageType::PlayerInput),
        .sequenceNumber = 1042,
        .senderClientId = 7,
        .payloadLength = 48
    };

    assert(header.IsValid());

    ByteStreamWriter writer;
    assert(SerializePacketHeader(writer, header));
    assert(writer.GetSize() == PACKET_HEADER_SIZE);

    ByteStreamReader reader(writer.GetData(), writer.GetSize());
    PacketHeader deserialized;
    assert(DeserializePacketHeader(reader, deserialized));
    assert(deserialized.magic == PROTOCOL_MAGIC);
    assert(deserialized.version == PROTOCOL_VERSION);
    assert(deserialized.messageType == static_cast<uint16_t>(MessageType::PlayerInput));
    assert(deserialized.sequenceNumber == 1042);
    assert(deserialized.senderClientId == 7);
    assert(deserialized.payloadLength == 48);

    // Verify rejection of invalid magic
    PacketHeader badMagic = header;
    badMagic.magic = 0xDEADBEEF;
    ByteStreamWriter writerBad;
    SerializePacketHeader(writerBad, badMagic);
    ByteStreamReader readerBad(writerBad.GetData(), writerBad.GetSize());
    PacketHeader resultBad;
    assert(!DeserializePacketHeader(readerBad, resultBad));

    std::cout << "  -> 16-byte fixed envelope and magic validation verified." << std::endl;
}

// ============================================================================
// 3. ByteStream Safe Field Serialization Test
// ============================================================================
static void TestByteStreamSerialization() {
    std::cout << "[TEST] 3. ByteStream Bounds & Type Serialization..." << std::endl;

    ByteStreamWriter writer;
    writer.WriteU8(0x7F);
    writer.WriteU16(0x1234);
    writer.WriteU32(0xAABBCCDD);
    writer.WriteU64(0x0102030405060708ULL);
    writer.WriteI16(-12345);
    writer.WriteFloat(3.14159f);
    writer.WriteString("Wildbound Battle Royale");

    ByteStreamReader reader(writer.GetData(), writer.GetSize());
    assert(reader.ReadU8() == 0x7F);
    assert(reader.ReadU16() == 0x1234);
    assert(reader.ReadU32() == 0xAABBCCDD);
    assert(reader.ReadU64() == 0x0102030405060708ULL);
    assert(reader.ReadI16() == -12345);
    assert(std::abs(reader.ReadFloat() - 3.14159f) < 0.0001f);
    assert(reader.ReadString() == "Wildbound Battle Royale");
    assert(reader.GetRemaining() == 0);

    // Test out of bounds read safety
    assert(reader.ReadU32() == 0);

    std::cout << "  -> Endian-safe scalar and string binary streams verified." << std::endl;
}

// ============================================================================
// 4. Client Input Command Serialization Test
// ============================================================================
static void TestPlayerInputSerialization() {
    std::cout << "[TEST] 4. Client Input Command Serialization..." << std::endl;

    MsgPlayerInput input{
        .inputSequence = 502,
        .dirX = 0.7071f,
        .dirY = -0.7071f,
        .facingAngle = 1.5708f,
        .actionFlags = 0x03 // Sprint + Attack
    };

    ByteStreamWriter writer;
    input.Serialize(writer);

    ByteStreamReader reader(writer.GetData(), writer.GetSize());
    MsgPlayerInput deserialized;
    assert(deserialized.Deserialize(reader));
    assert(deserialized.inputSequence == 502);
    assert(std::abs(deserialized.dirX - 0.7071f) < 0.001f);
    assert(std::abs(deserialized.dirY - (-0.7071f)) < 0.001f);
    assert(std::abs(deserialized.facingAngle - 1.5708f) < 0.001f);
    assert(deserialized.actionFlags == 0x03);

    std::cout << "  -> 60 Hz PlayerInput serialization and angle preservation verified." << std::endl;
}

// ============================================================================
// 5. State Snapshot Replication & Component Dirty Flags Test
// ============================================================================
static void TestStateSnapshotReplication() {
    std::cout << "[TEST] 5. State Snapshot Replication & Component Dirty Flags..." << std::endl;

    MsgStateSnapshot snapshot{
        .serverTick = 1200,
        .acknowledgedInputSeq = 499
    };

    EntityReplicationState ent1{
        .identity = NetworkEntityIdentity{ .id = NetworkEntityId{ 1 }, .generation = 1 },
        .dirtyMask = EntityDirtyMask::DirtyPosition | EntityDirtyMask::DirtyHealthArmor | EntityDirtyMask::DirtyWeapon,
        .posX = 1500.5f,
        .posY = 2200.0f,
        .health = 85.0f,
        .armor = 50.0f,
        .activeWeaponId = 101,
        .isAlive = true
    };

    EntityReplicationState ent2{
        .identity = NetworkEntityIdentity{ .id = NetworkEntityId{ 2 }, .generation = 1 },
        .dirtyMask = EntityDirtyMask::DirtyPosition | EntityDirtyMask::DirtyVelocity,
        .posX = 1600.0f,
        .posY = 2250.0f,
        .velX = 120.0f,
        .velY = -80.0f,
        .isAlive = true
    };

    snapshot.entities.push_back(ent1);
    snapshot.entities.push_back(ent2);

    ByteStreamWriter writer;
    snapshot.Serialize(writer);

    ByteStreamReader reader(writer.GetData(), writer.GetSize());
    MsgStateSnapshot outSnap;
    assert(outSnap.Deserialize(reader));
    assert(outSnap.serverTick == 1200);
    assert(outSnap.acknowledgedInputSeq == 499);
    assert(outSnap.entities.size() == 2);

    // Check entity 1
    const auto& res1 = outSnap.entities[0];
    assert(res1.identity.id.value == 1);
    assert(HasFlag(res1.dirtyMask, EntityDirtyMask::DirtyPosition));
    assert(std::abs(res1.posX - 1500.5f) < 0.01f);
    assert(std::abs(res1.health - 85.0f) < 0.01f);
    assert(res1.activeWeaponId == 101);

    // Check entity 2
    const auto& res2 = outSnap.entities[1];
    assert(res2.identity.id.value == 2);
    assert(HasFlag(res2.dirtyMask, EntityDirtyMask::DirtyVelocity));
    assert(std::abs(res2.velX - 120.0f) < 0.01f);
    assert(std::abs(res2.velY - (-80.0f)) < 0.01f);

    std::cout << "  -> Delta snapshot packing and dirty component bitmask filtering verified." << std::endl;
}

// ============================================================================
// 6. Gameplay Request Contracts Test
// ============================================================================
static void TestGameplayRequests() {
    std::cout << "[TEST] 6. Gameplay Request Contracts (Attack & Pickup)..." << std::endl;

    MsgAttackRequest attack{
        .weaponSlot = 1,
        .aimAngle = 3.14159f,
        .clientTick = 1205
    };
    ByteStreamWriter attackWriter;
    attack.Serialize(attackWriter);
    ByteStreamReader attackReader(attackWriter.GetData(), attackWriter.GetSize());
    MsgAttackRequest outAttack;
    assert(outAttack.Deserialize(attackReader));
    assert(outAttack.weaponSlot == 1);
    assert(std::abs(outAttack.aimAngle - 3.14159f) < 0.001f);
    assert(outAttack.clientTick == 1205);

    MsgPickupRequest pickup{
        .targetItemId = NetworkEntityIdentity{ .id = NetworkEntityId{ 88 }, .generation = 3 }
    };
    ByteStreamWriter pickupWriter;
    pickup.Serialize(pickupWriter);
    ByteStreamReader pickupReader(pickupWriter.GetData(), pickupWriter.GetSize());
    MsgPickupRequest outPickup;
    assert(outPickup.Deserialize(pickupReader));
    assert(outPickup.targetItemId.id.value == 88);
    assert(outPickup.targetItemId.generation == 3);

    std::cout << "  -> Attack and Pickup request binary contracts verified." << std::endl;
}

// ============================================================================
// 7. Handshake & Disconnect Message Lifecycle Test
// ============================================================================
static void TestHandshakeMessages() {
    std::cout << "[TEST] 7. Handshake & Connection Negotiation Messages..." << std::endl;

    MsgClientHello hello{
        .protocolVersion = PROTOCOL_VERSION,
        .playerName = "WildRanger",
        .authToken = "auth_sample_token_xyz"
    };
    ByteStreamWriter helloWriter;
    hello.Serialize(helloWriter);
    ByteStreamReader helloReader(helloWriter.GetData(), helloWriter.GetSize());
    MsgClientHello outHello;
    assert(outHello.Deserialize(helloReader));
    assert(outHello.protocolVersion == PROTOCOL_VERSION);
    assert(outHello.playerName == "WildRanger");
    assert(outHello.authToken == "auth_sample_token_xyz");

    MsgConnectionAccepted accepted{
        .assignedClientId = 4,
        .assignedPlayerId = NetworkEntityIdentity{ .id = NetworkEntityId{ 12 }, .generation = 1 },
        .matchId = "match_br_001",
        .mapName = "battle_royale_island"
    };
    ByteStreamWriter accWriter;
    accepted.Serialize(accWriter);
    ByteStreamReader accReader(accWriter.GetData(), accWriter.GetSize());
    MsgConnectionAccepted outAcc;
    assert(outAcc.Deserialize(accReader));
    assert(outAcc.assignedClientId == 4);
    assert(outAcc.assignedPlayerId.id.value == 12);
    assert(outAcc.matchId == "match_br_001");
    assert(outAcc.mapName == "battle_royale_island");

    std::cout << "  -> Connection initiation and server acceptance flow verified." << std::endl;
}

// ============================================================================
// 8. Protocol Security & Malformed Payload Rejection Test
// ============================================================================
static void TestProtocolSecurityRejection() {
    std::cout << "[TEST] 8. Protocol Security & Malformed Payload Rejection..." << std::endl;

    // Buffer truncation simulation
    uint8_t truncatedData[8] = { 0x44, 0x4E, 0x42, 0x57 }; // Less than PACKET_HEADER_SIZE
    ByteStreamReader truncatedReader(truncatedData, sizeof(truncatedData));
    PacketHeader hdr;
    assert(!DeserializePacketHeader(truncatedReader, hdr));

    // Oversized payload rejection check
    PacketHeader oversizedHdr{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = 1,
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = 5000 // Exceeds MAX_PACKET_PAYLOAD_SIZE (1200)
    };
    assert(!oversizedHdr.IsValid());

    // Version mismatch check
    PacketHeader versionMismatchHdr{
        .magic = PROTOCOL_MAGIC,
        .version = 999, // Incompatible future version
        .messageType = 1,
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = 10
    };
    assert(!versionMismatchHdr.IsValid());

    std::cout << "  -> Bounds enforcement, truncation safety, and version rejection verified." << std::endl;
}

// ============================================================================
// 9. Network Configuration JSON Schema & Validation Test
// ============================================================================
static void TestNetworkConfiguration() {
    std::cout << "[TEST] 9. Network Configuration JSON Schema & Validation..." << std::endl;

    NetworkConfiguration config;
    assert(config.LoadFromFile("assets/data/networking/network_config.json"));
    assert(config.version == 1);
    assert(config.server.bindPort == 7777);
    assert(config.server.maxClients == 64);
    assert(config.server.simulationTickRate == 60);
    assert(config.server.snapshotBroadcastRate == 30);
    assert(config.client.interpolationDelayMs == 100);
    assert(config.client.predictionEnabled == true);
    assert(config.interest.enabled == true);
    assert(config.interest.cellWidth == 640.0f);
    assert(config.protocol.protocolMagic == PROTOCOL_MAGIC);

    std::cout << "  -> Network JSON configuration parsing and schema validation verified." << std::endl;
}

// ============================================================================
// 10. Transport Abstraction Interface Verification Test
// ============================================================================
class MockNetworkTransport : public INetworkTransport {
public:
    bool Initialize(uint16_t port, size_t maxPeers) override {
        m_port = port;
        m_maxPeers = maxPeers;
        m_active = true;
        return true;
    }
    void Shutdown() override { m_active = false; }
    bool Connect(const std::string& address, uint16_t port) override {
        (void)address; (void)port;
        m_peerCount++;
        return true;
    }
    void Disconnect(uint16_t peerId, uint32_t reason) override {
        (void)peerId; (void)reason;
        if (m_peerCount > 0) m_peerCount--;
    }
    bool Send(uint16_t peerId, ChannelType channel, DeliveryMode mode, const uint8_t* data, size_t size) override {
        (void)peerId; (void)channel; (void)mode;
        m_sentBytes += size;
        return (data != nullptr && size > 0);
    }
    bool PollEvent(TransportEvent& outEvent) override {
        (void)outEvent;
        return false;
    }
    void Flush() override {}
    bool IsActive() const noexcept override { return m_active; }
    size_t GetConnectedPeerCount() const noexcept override { return m_peerCount; }

    size_t GetSentBytes() const noexcept { return m_sentBytes; }

private:
    bool m_active{ false };
    uint16_t m_port{ 0 };
    size_t m_maxPeers{ 0 };
    size_t m_peerCount{ 0 };
    size_t m_sentBytes{ 0 };
};

static void TestTransportAbstraction() {
    std::cout << "[TEST] 10. Transport Abstraction Interface Verification..." << std::endl;

    std::unique_ptr<INetworkTransport> transport = std::make_unique<MockNetworkTransport>();
    assert(!transport->IsActive());
    assert(transport->Initialize(7777, 32));
    assert(transport->IsActive());

    assert(transport->Connect("127.0.0.1", 7777));
    assert(transport->GetConnectedPeerCount() == 1);

    uint8_t samplePacket[] = { 0x01, 0x02, 0x03, 0x04 };
    assert(transport->Send(1, ChannelType::PlayerInput, DeliveryMode::UnreliableSequenced, samplePacket, sizeof(samplePacket)));

    auto* mock = dynamic_cast<MockNetworkTransport*>(transport.get());
    assert(mock != nullptr);
    assert(mock->GetSentBytes() == 4);

    transport->Disconnect(1);
    assert(transport->GetConnectedPeerCount() == 0);
    transport->Shutdown();
    assert(!transport->IsActive());

    std::cout << "  -> INetworkTransport polymorphic decoupling and lifecycle verified." << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 35: MULTIPLAYER ARCHITECTURE TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestNetworkEntityIdentity();
    TestProtocolHeaderSerialization();
    TestByteStreamSerialization();
    TestPlayerInputSerialization();
    TestStateSnapshotReplication();
    TestGameplayRequests();
    TestHandshakeMessages();
    TestProtocolSecurityRejection();
    TestNetworkConfiguration();
    TestTransportAbstraction();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 PHASE 35 ARCHITECTURE TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

