#include "network/replication/NetworkEntityTypes.hpp"
#include "network/replication/ReplicationRegistry.hpp"
#include "network/replication/Snapshot.hpp"
#include "network/replication/SnapshotBuffer.hpp"
#include "network/replication/ReplicationInterpolator.hpp"
#include "network/replication/ReplicationEvents.hpp"
#include "network/replication/ReplicationConfiguration.hpp"
#include "network/replication/ReplicationManager.hpp"
#include "network/ENetServer.hpp"
#include "network/ENetClient.hpp"
#include "network/NetworkProtocol.hpp"
#include "network/NetworkMessage.hpp"
#include "core/Logger.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

using namespace wildbound::network;

// Macro helper for clean assertions
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while(0)

// ----------------------------------------------------------------------------
// Test 1: Network Entity Identity & Generational Invalidation
// ----------------------------------------------------------------------------
void Test1_EntityIdentityAndGeneration() {
    std::cout << "[TEST] 1. Network Entity Identity & Generational Invalidation..." << std::endl;

    ReplicationRegistry registry;
    TEST_ASSERT(registry.GetActiveEntityCount() == 0, "Initial registry count must be 0");

    // Register Player
    auto id1 = registry.RegisterEntity(NetworkEntityType::Player, 101, 10);
    TEST_ASSERT(id1.IsValid(), "Registered player identity must be valid");
    TEST_ASSERT(id1.generation == 1, "Initial generation must be 1");
    TEST_ASSERT(registry.IsEntityValid(id1), "Entity must be valid in registry");
    TEST_ASSERT(registry.GetEntityType(id1) == NetworkEntityType::Player, "Type must match Player");
    TEST_ASSERT(registry.GetLocalEntityId(id1) == 101, "Local ID must match 101");

    // Unregister
    bool unregOk = registry.UnregisterEntity(id1);
    TEST_ASSERT(unregOk, "Unregistering entity must succeed");
    TEST_ASSERT(!registry.IsEntityValid(id1), "Old identity handle must now be invalid");
    TEST_ASSERT(registry.GetActiveEntityCount() == 0, "Active entity count must return to 0");

    // Re-allocate slot - should reuse slot with incremented generation
    auto id2 = registry.RegisterEntity(NetworkEntityType::Enemy, 202, 20);
    TEST_ASSERT(id2.id == id1.id, "Slot ID should be reused");
    TEST_ASSERT(id2.generation == 2, "Generation counter must be incremented to 2");
    TEST_ASSERT(registry.IsEntityValid(id2), "New handle must be valid");
    TEST_ASSERT(!registry.IsEntityValid(id1), "Old handle (gen 1) must remain invalid");

    std::cout << "  -> Generational entity slots and stale invalidation verified." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 2: Snapshot Serialization (Full & Delta)
// ----------------------------------------------------------------------------
void Test2_SnapshotSerialization() {
    std::cout << "[TEST] 2. Snapshot Binary Serialization (Full & Delta)..." << std::endl;

    // Full snapshot
    WorldSnapshot fullSnap;
    fullSnap.serverTick = 42;
    fullSnap.timestamp = 1.4f;
    fullSnap.isDelta = false;

    EntitySnapshotRecord rec1{};
    rec1.identity = NetworkEntityIdentity{ .id = NetworkEntityId{ 1 }, .generation = 1 };
    rec1.type = NetworkEntityType::Player;
    rec1.posX = 150.5f;
    rec1.posY = 320.25f;
    rec1.velX = 160.0f;
    rec1.velY = 0.0f;
    rec1.facingAngle = 1.57f;
    rec1.health = 85.0f;
    rec1.maxHealth = 100.0f;
    rec1.armor = 25.0f;
    rec1.SetAlive(true);
    rec1.SetMoving(true);
    fullSnap.entities.push_back(rec1);

    ByteStreamWriter writer;
    fullSnap.Serialize(writer);
    TEST_ASSERT(writer.GetSize() > 0, "Serialized snapshot must have non-zero size");

    ByteStreamReader reader(writer.GetData(), writer.GetSize());
    WorldSnapshot deserialized;
    bool desOk = deserialized.Deserialize(reader);
    TEST_ASSERT(desOk, "Deserializing full snapshot must succeed");
    TEST_ASSERT(deserialized.serverTick == 42, "Server tick must match");
    TEST_ASSERT(!deserialized.isDelta, "Snapshot must not be marked delta");
    TEST_ASSERT(deserialized.entities.size() == 1, "Entity count must match 1");

    const auto& entOut = deserialized.entities[0];
    TEST_ASSERT(entOut.identity == rec1.identity, "Entity identity must match");
    TEST_ASSERT(std::abs(entOut.posX - 150.5f) < 0.001f, "Position X must match");
    TEST_ASSERT(std::abs(entOut.posY - 320.25f) < 0.001f, "Position Y must match");
    TEST_ASSERT(entOut.IsAlive(), "Alive status must match");
    TEST_ASSERT(entOut.IsMoving(), "Moving status must match");

    // Delta snapshot
    WorldSnapshot deltaSnap;
    deltaSnap.serverTick = 43;
    deltaSnap.timestamp = 1.45f;
    deltaSnap.isDelta = true;
    deltaSnap.baselineTick = 42;

    EntitySnapshotRecord recDelta = rec1;
    recDelta.posX = 158.5f; // Moved 8 units
    recDelta.dirtyMask = EntityDirtyMask::DirtyPosition;
    deltaSnap.entities.push_back(recDelta);

    ByteStreamWriter deltaWriter;
    deltaSnap.Serialize(deltaWriter);
    TEST_ASSERT(deltaWriter.GetSize() < writer.GetSize(), "Delta snapshot must be smaller than full snapshot");

    ByteStreamReader deltaReader(deltaWriter.GetData(), deltaWriter.GetSize());
    WorldSnapshot deserializedDelta;
    bool deltaDesOk = deserializedDelta.Deserialize(deltaReader);
    TEST_ASSERT(deltaDesOk, "Deserializing delta snapshot must succeed");
    TEST_ASSERT(deserializedDelta.isDelta, "Snapshot must be marked delta");
    TEST_ASSERT(std::abs(deserializedDelta.entities[0].posX - 158.5f) < 0.001f, "Delta position must match");

    std::cout << "  -> Full and delta binary serialization verified with bandwidth savings." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 3: Snapshot Buffer Ordering & Stale Rejection
// ----------------------------------------------------------------------------
void Test3_SnapshotBufferOrdering() {
    std::cout << "[TEST] 3. Snapshot Buffer Ordering & Stale Rejection..." << std::endl;

    SnapshotBuffer buffer(8);
    TEST_ASSERT(buffer.GetSnapshotCount() == 0, "Initial buffer must be empty");

    WorldSnapshot s1; s1.serverTick = 10; s1.timestamp = 0.5f;
    WorldSnapshot s2; s2.serverTick = 11; s2.timestamp = 0.55f;
    WorldSnapshot s3; s3.serverTick = 12; s3.timestamp = 0.60f;

    TEST_ASSERT(buffer.PushSnapshot(s1), "Pushing s1 (tick 10) must succeed");
    TEST_ASSERT(buffer.PushSnapshot(s2), "Pushing s2 (tick 11) must succeed");
    TEST_ASSERT(buffer.PushSnapshot(s3), "Pushing s3 (tick 12) must succeed");
    TEST_ASSERT(buffer.GetSnapshotCount() == 3, "Buffer must contain 3 snapshots");

    // Attempt to push out-of-order or stale snapshot (tick 10)
    WorldSnapshot stale; stale.serverTick = 10; stale.timestamp = 0.5f;
    bool pushStale = buffer.PushSnapshot(stale);
    TEST_ASSERT(!pushStale, "Pushing stale snapshot must be rejected");
    TEST_ASSERT(buffer.GetRejectedStaleCount() == 1, "Rejected stale counter must increment");

    // Buffer capacity boundary
    for (uint64_t t = 13; t <= 25; ++t) {
        WorldSnapshot s; s.serverTick = t; s.timestamp = 0.6f + (t - 12) * 0.05f;
        buffer.PushSnapshot(s);
    }
    TEST_ASSERT(buffer.GetSnapshotCount() == 8, "Buffer must not exceed max capacity of 8");
    TEST_ASSERT(buffer.GetLatestSnapshot()->serverTick == 25, "Latest snapshot tick must be 25");

    std::cout << "  -> Snapshot chronological ordering and stale rejection verified." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 4: Interpolation Math & Teleport Snapping
// ----------------------------------------------------------------------------
void Test4_InterpolationMath() {
    std::cout << "[TEST] 4. Interpolation Math & Teleport Boundary Snapping..." << std::endl;

    ReplicationInterpolator interp(150.0f); // 150 units snap threshold

    // Smooth movement
    float outX = 0.0f, outY = 0.0f;
    interp.InterpolatePosition(100.0f, 100.0f, 200.0f, 100.0f, 0.5f, outX, outY);
    TEST_ASSERT(std::abs(outX - 150.0f) < 0.001f, "Midpoint X must be 150");
    TEST_ASSERT(std::abs(outY - 100.0f) < 0.001f, "Midpoint Y must be 100");

    // Teleport threshold snap (> 150 units distance)
    interp.InterpolatePosition(0.0f, 0.0f, 500.0f, 500.0f, 0.2f, outX, outY);
    TEST_ASSERT(std::abs(outX - 500.0f) < 0.001f, "Teleport snap must jump directly to target X");
    TEST_ASSERT(std::abs(outY - 500.0f) < 0.001f, "Teleport snap must jump directly to target Y");

    // Angular wrapping (350 deg to 10 deg radians)
    constexpr float rad350 = 6.10865f;
    constexpr float rad10 = 0.17453f;
    float blendedAngle = interp.InterpolateAngleRadians(rad350, rad10, 0.5f);
    // Should cross through 0 rad rather than spinning back through 180 deg
    TEST_ASSERT(blendedAngle < 0.01f || blendedAngle > 6.27f, "Angle interpolation must take shortest path");

    std::cout << "  -> Shortest-path angular wrapping and teleport clamping verified." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 5: Event Deduplication & Application Idempotency
// ----------------------------------------------------------------------------
void Test5_EventDeduplication() {
    std::cout << "[TEST] 5. Event Deduplication & Application Idempotency..." << std::endl;

    EventDeduplicator dedup(64);

    uint64_t ev1 = 5001;
    uint64_t ev2 = 5002;

    TEST_ASSERT(!dedup.IsDuplicateAndRecord(ev1), "First arrival of ev1 must not be marked duplicate");
    TEST_ASSERT(dedup.HasSeenEvent(ev1), "ev1 must now be in history");
    TEST_ASSERT(dedup.IsDuplicateAndRecord(ev1), "Second arrival of ev1 MUST be rejected as duplicate");
    TEST_ASSERT(dedup.GetDuplicateCount() == 1, "Duplicate count must be 1");

    TEST_ASSERT(!dedup.IsDuplicateAndRecord(ev2), "First arrival of ev2 must not be marked duplicate");
    TEST_ASSERT(dedup.IsDuplicateAndRecord(ev2), "Second arrival of ev2 MUST be duplicate");
    TEST_ASSERT(dedup.GetDuplicateCount() == 2, "Duplicate count must be 2");

    std::cout << "  -> Event deduplicator prevents duplicate damage and actions." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 6: Authoritative Movement Validation & Speed Limits
// ----------------------------------------------------------------------------
void Test6_AuthoritativeMovement() {
    std::cout << "[TEST] 6. Authoritative Movement Validation & Speed Limits..." << std::endl;

    ReplicationManager server(NetworkRole::Server);
    server.Initialize();

    auto player = server.RegisterPlayer(1, 100.0f, 100.0f);
    TEST_ASSERT(player.IsValid(), "Registered player must be valid");

    // Send valid walking input command (moving right)
    MsgPlayerInput walkInput{
        .inputSequence = 1,
        .dirX = 1.0f,
        .dirY = 0.0f,
        .facingAngle = 0.0f,
        .actionFlags = 0 // walk
    };

    server.ServerProcessInput(player, walkInput, 0.1f); // 0.1 sec dt
    auto snapshot1 = server.BuildSnapshot(true);
    const auto* rec1 = snapshot1.FindEntity(player);
    TEST_ASSERT(rec1 != nullptr, "Player must be in snapshot");
    TEST_ASSERT(std::abs(rec1->posX - 116.0f) < 0.1f, "Player should move ~16 units (160 units/s * 0.1s)");
    TEST_ASSERT(rec1->IsMoving(), "Player should be marked moving");
    TEST_ASSERT(!rec1->IsRunning(), "Player should not be marked running");

    // Send speed-hack attempted input (dirX = 50.0f)
    MsgPlayerInput hackInput{
        .inputSequence = 2,
        .dirX = 50.0f,
        .dirY = 0.0f,
        .facingAngle = 0.0f,
        .actionFlags = 0
    };

    server.ServerProcessInput(player, hackInput, 0.1f);
    auto snapshot2 = server.BuildSnapshot(true);
    const auto* rec2 = snapshot2.FindEntity(player);
    // Speed should be clamped to unit length * base speed (160 * 0.1 = 16 units)
    TEST_ASSERT(std::abs(rec2->posX - 132.0f) < 0.5f, "Excessive speed input must be normalized and clamped");

    std::cout << "  -> Server-side speed hack prevention and movement validation verified." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 7: Combat Event Validation & Elimination Flow
// ----------------------------------------------------------------------------
void Test7_CombatEventFlow() {
    std::cout << "[TEST] 7. Combat Event Validation & Elimination Flow..." << std::endl;

    ReplicationManager server(NetworkRole::Server);
    server.Initialize();

    auto p1 = server.RegisterPlayer(1, 100.0f, 100.0f);
    auto enemy = server.RegisterEnemy(2, 120.0f, 100.0f, 50.0f, 0); // 20 units away, 50 HP

    std::vector<CombatEvent> eventsReceived;
    server.SetCombatEventCallback([&](const CombatEvent& evt) {
        eventsReceived.push_back(evt);
    });

    // P1 attacks enemy (melee weapon, slot 0 -> 25 dmg)
    MsgAttackRequest attack1{ .weaponSlot = 0, .aimAngle = 0.0f, .clientTick = 1 };
    bool hitOk = server.ServerProcessAttack(p1, attack1);
    TEST_ASSERT(hitOk, "Server must confirm melee hit within 20 units range");
    TEST_ASSERT(eventsReceived.size() == 1, "Combat event must be emitted");
    TEST_ASSERT(eventsReceived[0].damageAmount == 25.0f, "Damage must be 25");
    TEST_ASSERT(eventsReceived[0].remainingHealth == 25.0f, "Remaining health must be 25");
    TEST_ASSERT(!eventsReceived[0].isElimination, "Enemy should not be eliminated yet");

    // Second hit -> lethal (50 - 25 - 25 = 0 HP)
    server.ServerProcessAttack(p1, attack1);
    TEST_ASSERT(eventsReceived.size() == 2, "Second combat event must be emitted");
    TEST_ASSERT(eventsReceived[1].remainingHealth == 0.0f, "Health must reach 0");
    TEST_ASSERT(eventsReceived[1].isElimination, "Enemy must be marked eliminated");

    // Third hit -> target already dead, should miss
    bool thirdHit = server.ServerProcessAttack(p1, attack1);
    TEST_ASSERT(!thirdHit, "Dead target must not be hit again");

    std::cout << "  -> Authoritative damage calculation, health updates, and elimination verified." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 8: Atomic Loot Pickup & Concurrent Race Protection
// ----------------------------------------------------------------------------
void Test8_AtomicLootPickup() {
    std::cout << "[TEST] 8. Atomic Loot Pickup & Concurrent Race Protection..." << std::endl;

    ReplicationManager server(NetworkRole::Server);
    server.Initialize();

    auto p1 = server.RegisterPlayer(1, 100.0f, 100.0f);
    auto p2 = server.RegisterPlayer(2, 110.0f, 100.0f);
    auto loot = server.RegisterLoot(10, 501, 105.0f, 100.0f, 5); // 5 health potions

    std::vector<PickupResultEvent> results;
    server.SetPickupResultCallback([&](const PickupResultEvent& res) {
        results.push_back(res);
    });

    // Both players simultaneously attempt to loot the same item
    PickupRequestEvent req1{ .targetLootEntity = loot, .requestingPlayer = p1, .clientTick = 10 };
    PickupRequestEvent req2{ .targetLootEntity = loot, .requestingPlayer = p2, .clientTick = 10 };

    bool p1Success = server.ServerProcessPickup(req1);
    bool p2Success = server.ServerProcessPickup(req2);

    TEST_ASSERT(p1Success, "First pickup request must succeed");
    TEST_ASSERT(!p2Success, "Second concurrent pickup request for same loot MUST be rejected");
    TEST_ASSERT(results.size() == 1, "Only one successful pickup result event should be emitted");
    TEST_ASSERT(results[0].recipientPlayer == p1, "Loot recipient must be player 1");
    TEST_ASSERT(!server.GetRegistry().IsEntityValid(loot), "Loot entity must be removed from world");

    std::cout << "  -> Atomic pickup transaction guarantees single-owner loot resolution." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 9: Environmental & Storm State Replication
// ----------------------------------------------------------------------------
void Test9_EnvironmentalAndStormSync() {
    std::cout << "[TEST] 9. Environmental & Storm State Replication..." << std::endl;

    ReplicationManager server(NetworkRole::Server);
    server.Initialize();

    auto door = server.RegisterEnvironment(5, 200.0f, 200.0f, 0); // Closed door

    EnvironmentalStateEvent lastEnvEvent{};
    server.SetEnvironmentalCallback([&](const EnvironmentalStateEvent& evt) {
        lastEnvEvent = evt;
    });

    StormStateEvent lastStormEvent{};
    server.SetStormCallback([&](const StormStateEvent& evt) {
        lastStormEvent = evt;
    });

    // Toggle Door to Open
    server.ServerUpdateEnvironment(door, 1, true, false);
    TEST_ASSERT(lastEnvEvent.entityId == door, "Door entity ID must match");
    TEST_ASSERT(lastEnvEvent.stateId == 1, "Door state must be 1 (Open)");
    TEST_ASSERT(lastEnvEvent.active, "Door must be marked active");

    // Advance Storm Phase
    server.ServerUpdateStorm(2, 4000.0f, 4000.0f, 2500.0f, 1800.0f, 5.0f);
    TEST_ASSERT(lastStormEvent.currentPhase == 2, "Storm phase must be 2");
    TEST_ASSERT(lastStormEvent.dps == 5.0f, "DPS must be 5.0");
    TEST_ASSERT(lastStormEvent.currentRadius == 2500.0f, "Current radius must match");

    std::cout << "  -> Environmental interactions and storm boundaries synchronized." << std::endl;
}

// ----------------------------------------------------------------------------
// Test 10: 2-Client Localhost Replication Integration
// ----------------------------------------------------------------------------
void Test10_TwoClientLocalhostIntegration() {
    std::cout << "[TEST] 10. Two-Client Localhost Replication Integration..." << std::endl;

    const uint16_t port = 9123;

    // 1. Start Server
    ENetServer server;
    bool srvOk = server.Start(port, 16);
    TEST_ASSERT(srvOk, "Server host must start");

    ReplicationManager serverRep(NetworkRole::Server);
    serverRep.Initialize();

    // 2. Connect Client A & Client B
    ENetClient clientA, clientB;
    bool cAOk = clientA.Connect("127.0.0.1", port, 3000);
    bool cBOk = clientB.Connect("127.0.0.1", port, 3000);
    TEST_ASSERT(cAOk && cBOk, "Both clients must initiate connection");

    // Poll until both are connected
    for (int i = 0; i < 50; ++i) {
        server.Update(0.016f);
        clientA.Update(0.016f);
        clientB.Update(0.016f);
        if (clientA.IsConnected() && clientB.IsConnected() && server.GetConnectedCount() == 2) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    TEST_ASSERT(clientA.IsConnected() && clientB.IsConnected(), "Both clients must be fully connected");
    ReplicationManager clientARep(NetworkRole::Client);
    ReplicationManager clientBRep(NetworkRole::Client);
    clientARep.Initialize();
    clientBRep.Initialize();

    // 3. Register Player A and Player B on server
    auto pA = serverRep.RegisterPlayer(1, 100.0f, 100.0f);
    auto pB = serverRep.RegisterPlayer(2, 300.0f, 300.0f);

    // 4. Client A moves right (speed 160 u/s)
    MsgPlayerInput moveInput{ .inputSequence = 1, .dirX = 1.0f, .dirY = 0.0f, .facingAngle = 0.0f, .actionFlags = 0 };
    serverRep.ServerProcessInput(pA, moveInput, 0.1f); // Player A moved to (116, 100)

    // 5. Server builds snapshot and broadcasts to clients over Channel 3 (Replication)
    auto snap = serverRep.BuildSnapshot(true);
    ByteStreamWriter payloadWriter;
    snap.Serialize(payloadWriter);

    ByteStreamWriter packetWriter;
    PacketHeader hdr{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(MessageType::StateSnapshot),
        .sequenceNumber = 1,
        .senderClientId = SERVER_CLIENT_ID,
        .payloadLength = static_cast<uint16_t>(payloadWriter.GetSize())
    };
    SerializePacketHeader(packetWriter, hdr);
    packetWriter.WriteBytes(payloadWriter.GetData(), payloadWriter.GetSize());

    server.Broadcast(ChannelType::Replication, DeliveryMode::UnreliableSequenced, packetWriter.GetData(), packetWriter.GetSize());

    // 6. Network pump
    std::vector<uint8_t> clientBReceivedPacket;
    clientB.SetEventCallback([&](const ConnectionEvent& ev) {
        if (ev.type == ConnectionEventType::PacketReceived && ev.channel == ChannelType::Replication) {
            clientBReceivedPacket = ev.payload;
        }
    });

    for (int i = 0; i < 20; ++i) {
        server.Update(0.016f);
        clientA.Update(0.016f);
        clientB.Update(0.016f);
        if (!clientBReceivedPacket.empty()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(!clientBReceivedPacket.empty(), "Client B must receive replication snapshot over Channel 3");

    // 7. Client B parses snapshot
    ByteStreamReader snapReader(clientBReceivedPacket.data(), clientBReceivedPacket.size());
    WorldSnapshot receivedSnap;
    bool parseOk = receivedSnap.Deserialize(snapReader);
    TEST_ASSERT(parseOk, "Client B must cleanly deserialize snapshot");
    TEST_ASSERT(receivedSnap.entities.size() == 2, "Snapshot must replicate both players");

    clientBRep.ClientReceiveSnapshot(receivedSnap);
    TEST_ASSERT(clientBRep.GetRegistry().IsEntityValid(pA), "Client B registry must now track Player A");
    TEST_ASSERT(clientBRep.GetRegistry().IsEntityValid(pB), "Client B registry must now track Player B");

    // 8. Clean teardown
    clientA.Disconnect();
    clientB.Disconnect();
    for (int i = 0; i < 20; ++i) {
        server.Update(0.016f);
        clientA.Update(0.016f);
        clientB.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    server.Stop();

    std::cout << "  -> Multi-client localhost network replication verified end-to-end." << std::endl;
}

// ----------------------------------------------------------------------------
// Main Test Runner
// ----------------------------------------------------------------------------
int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 37: ENTITY SYNCHRONIZATION TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    Test1_EntityIdentityAndGeneration();
    Test2_SnapshotSerialization();
    Test3_SnapshotBufferOrdering();
    Test4_InterpolationMath();
    Test5_EventDeduplication();
    Test6_AuthoritativeMovement();
    Test7_CombatEventFlow();
    Test8_AtomicLootPickup();
    Test9_EnvironmentalAndStormSync();
    Test10_TwoClientLocalhostIntegration();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 PHASE 37 REPLICATION TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

