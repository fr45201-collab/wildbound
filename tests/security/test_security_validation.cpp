#include "../../src/security/ServerSecurityManager.hpp"
#include "../../src/security/RateLimiter.hpp"
#include "../../src/security/SecurityTypes.hpp"
#include "../../src/network/NetworkMessage.hpp"
#include "../../src/core/Logger.hpp"

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <limits>
#include <cassert>
#include <cstdlib>

using namespace wildbound;
using namespace wildbound::security;
using namespace wildbound::network;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

// 1. Rate Limiter Token Bucket & Burst Prevention Tests
void TestRateLimiter() {
    std::cout << "[TEST] 1. Rate Limiter Token Bucket & Category Isolation..." << std::endl;

    RateLimiter limiter;

    // Consume all GeneralPacket tokens
    bool rejected = false;
    for (int i = 0; i < 200; ++i) {
        if (!limiter.CheckAndConsume(1, RateLimitCategory::GeneralPacket)) {
            rejected = true;
            break;
        }
    }
    TEST_ASSERT(rejected, "Excessive GeneralPacket stream must be rate limited");

    // Category isolation: PlayerInput category should still have tokens
    bool inputAllowed = limiter.CheckAndConsume(1, RateLimitCategory::PlayerInput);
    TEST_ASSERT(inputAllowed, "PlayerInput must be isolated from GeneralPacket saturation");

    // Client isolation: Client 2 should be unaffected by Client 1 saturation
    bool client2Allowed = limiter.CheckAndConsume(2, RateLimitCategory::GeneralPacket);
    TEST_ASSERT(client2Allowed, "Client 2 must be unaffected by Client 1 exhaustion");

    // Remove client lifecycle
    limiter.RemoveClient(1);
    TEST_ASSERT(limiter.GetTrackedClientCount() == 1, "Only client 2 should remain tracked");
}

// 2. Authoritative Movement Validation Tests
void TestMovementValidation() {
    std::cout << "[TEST] 2. Authoritative Movement Validation (Speed, Teleport, NaN)..." << std::endl;

    ServerSecurityManager security;
    uint16_t clientId = 10;
    uint32_t participantId = 10;
    security.OnClientConnected(clientId);

    float maxSpeed = 160.0f; // 160 px/sec
    float dt = 1.0f / 30.0f; // 33.33ms per tick
    Vector2 currentPos{ 500.0f, 500.0f };

    // 2.1 Normal valid movement
    Vector2 validRequested{ 500.0f + 4.0f, 500.0f + 3.0f }; // distance = 5px, speed = 150 px/s
    Vector2 validatedPos = currentPos;
    bool validResult = security.ValidateMovement(clientId, participantId, currentPos, validRequested, dt, maxSpeed, validatedPos);
    TEST_ASSERT(validResult, "Normal movement within speed limit must be accepted");
    TEST_ASSERT(std::abs(validatedPos.x - validRequested.x) < 0.001f, "Validated position should match requested");

    // 2.2 Speed hack attempt (speed = 750 px/s, well above 160 px/s)
    Vector2 speedHackRequested{ 500.0f + 25.0f, 500.0f }; // distance = 25px in 1 tick = 750 px/s
    Vector2 clampedPos = currentPos;
    bool speedHackResult = security.ValidateMovement(clientId, participantId, currentPos, speedHackRequested, dt, maxSpeed, clampedPos);
    TEST_ASSERT(!speedHackResult, "Speed hack movement must be flagged and rejected");
    // Clamped position must not exceed max allowed distance
    float movedDist = std::hypot(clampedPos.x - currentPos.x, clampedPos.y - currentPos.y);
    float maxAllowedDist = (maxSpeed * dt * 1.35f) + 8.0f;
    TEST_ASSERT(movedDist <= maxAllowedDist + 0.01f, "Clamped position must be constrained to max allowed speed boundary");

    // 2.3 Teleportation anomaly (huge jump > 300px in single tick)
    Vector2 teleportRequested{ 500.0f + 500.0f, 500.0f + 500.0f };
    Vector2 snappedPos = currentPos;
    bool teleportResult = security.ValidateMovement(clientId, participantId, currentPos, teleportRequested, dt, maxSpeed, snappedPos);
    TEST_ASSERT(!teleportResult, "Teleport jump must be rejected");
    TEST_ASSERT(snappedPos.x == currentPos.x && snappedPos.y == currentPos.y, "Teleport attempt must snap back to authoritative position");

    // 2.4 Poisoned NaN / Inf coordinates injection
    Vector2 nanRequested{ std::numeric_limits<float>::quiet_NaN(), 500.0f };
    Vector2 nanSafePos = currentPos;
    bool nanResult = security.ValidateMovement(clientId, participantId, currentPos, nanRequested, dt, maxSpeed, nanSafePos);
    TEST_ASSERT(!nanResult, "NaN coordinates must be rejected");
    TEST_ASSERT(nanSafePos.x == currentPos.x && nanSafePos.y == currentPos.y, "NaN coordinates must snap back safely");

    // Check diagnostics
    const auto& diag = security.GetDiagnostics();
    TEST_ASSERT(diag.movementViolations >= 3, "Movement violations must be tracked in diagnostics");
}

// 3. Authoritative Combat Action Validation Tests
void TestCombatValidation() {
    std::cout << "[TEST] 3. Authoritative Combat Action Validation (Range, Cooldown)..." << std::endl;

    ServerSecurityManager security;
    uint16_t clientId = 20;
    uint32_t participantId = 20;
    security.OnClientConnected(clientId);

    Vector2 attackerPos{ 200.0f, 200.0f };
    float weaponRange = 40.0f; // 40px base range

    // 3.1 Valid attack within range and cooldown
    Vector2 targetPos{ 230.0f, 200.0f }; // 30px distance < 40px
    bool attack1 = security.ValidateAttack(clientId, participantId, "sword_slash", attackerPos, targetPos, weaponRange, true);
    TEST_ASSERT(attack1, "Legitimate attack in range must be accepted");

    // 3.2 Rapid-fire cooldown violation (immediate second attack)
    bool attackRapid = security.ValidateAttack(clientId, participantId, "sword_slash", attackerPos, targetPos, weaponRange, true);
    TEST_ASSERT(!attackRapid, "Immediate subsequent attack must violate cooldown and be rejected");

    // 3.3 Attack out of range (target = 100px away, weapon max = 40px, tolerance = 50px)
    // Wait for cooldown to expire or use different weapon
    std::this_thread::sleep_for(std::chrono::milliseconds(260));
    Vector2 farTargetPos{ 350.0f, 200.0f }; // 150px distance > 50px limit
    bool attackFar = security.ValidateAttack(clientId, participantId, "sword_slash", attackerPos, farTargetPos, weaponRange, true);
    TEST_ASSERT(!attackFar, "Out-of-range attack must be rejected");

    // 3.4 Weapon not ready (client state desync / reloading / disabled)
    std::this_thread::sleep_for(std::chrono::milliseconds(260));
    bool attackNotReady = security.ValidateAttack(clientId, participantId, "sword_slash", attackerPos, targetPos, weaponRange, false);
    TEST_ASSERT(!attackNotReady, "Attack when weapon is not ready must be rejected");

    const auto& diag = security.GetDiagnostics();
    TEST_ASSERT(diag.combatViolations >= 3, "Combat violations must be tracked in diagnostics");
}

// 4. Atomic Loot Pickup Validation Tests
void TestAtomicLootPickup() {
    std::cout << "[TEST] 4. Atomic Loot Pickup Resolution & Concurrency Simulation..." << std::endl;

    ServerSecurityManager security;
    uint16_t clientA = 1;
    uint16_t clientB = 2;
    security.OnClientConnected(clientA);
    security.OnClientConnected(clientB);

    uint64_t lootId = 10042;
    Vector2 lootPos{ 100.0f, 100.0f };
    Vector2 playerAPos{ 110.0f, 100.0f }; // 10px away
    Vector2 playerBPos{ 115.0f, 100.0f }; // 15px away
    float maxRadius = 36.0f;

    // 4.1 Player A claims loot first
    bool claimA = security.ValidateLootPickup(clientA, 1, lootId, playerAPos, lootPos, maxRadius);
    TEST_ASSERT(claimA, "First valid loot claimant must successfully acquire loot");

    // 4.2 Player B attempts to claim same loot (race condition simulation)
    bool claimB = security.ValidateLootPickup(clientB, 2, lootId, playerBPos, lootPos, maxRadius);
    TEST_ASSERT(!claimB, "Second simultaneous claimant must be rejected because loot is already claimed");

    // 4.3 Out of range pickup attempt on new loot
    uint64_t farLootId = 10043;
    Vector2 farLootPos{ 300.0f, 300.0f };
    bool claimFar = security.ValidateLootPickup(clientA, 1, farLootId, playerAPos, farLootPos, maxRadius);
    TEST_ASSERT(!claimFar, "Out-of-range loot claim must be rejected");

    const auto& diag = security.GetDiagnostics();
    TEST_ASSERT(diag.lootViolations >= 2, "Loot race and range violations must be tracked");
}

// 5. Network Deserialization Sanitization Tests
void TestNetworkMessageSanitization() {
    std::cout << "[TEST] 5. Network Deserialization & Payload Sanitization..." << std::endl;

    // 5.1 MsgPlayerInput with NaN coordinates
    {
        ByteStreamWriter writer;
        MsgPlayerInput input;
        input.inputSequence = 100;
        input.dirX = std::numeric_limits<float>::quiet_NaN();
        input.dirY = 1.0f;
        input.facingAngle = 0.5f;
        input.Serialize(writer);

        ByteStreamReader reader(writer.GetData(), writer.GetSize());
        MsgPlayerInput deserialized;
        bool ok = deserialized.Deserialize(reader);
        TEST_ASSERT(!ok, "Deserialization must reject MsgPlayerInput containing NaN values");
    }

    // 5.2 MsgPlayerInput with overspeed un-normalized direction vector
    {
        ByteStreamWriter writer;
        MsgPlayerInput input;
        input.inputSequence = 101;
        input.dirX = 10.0f; // unnormalized length ~ 10
        input.dirY = 0.0f;
        input.facingAngle = 0.5f;
        input.Serialize(writer);

        ByteStreamReader reader(writer.GetData(), writer.GetSize());
        MsgPlayerInput deserialized;
        bool ok = deserialized.Deserialize(reader);
        TEST_ASSERT(ok, "Deserialization should succeed and normalize input vector");
        float len = std::hypot(deserialized.dirX, deserialized.dirY);
        TEST_ASSERT(len <= 1.0001f, "Deserialized direction vector must be normalized to length <= 1.0");
    }

    // 5.3 MsgAttackRequest with infinite aimAngle
    {
        ByteStreamWriter writer;
        MsgAttackRequest attack;
        attack.weaponSlot = 1;
        attack.aimAngle = std::numeric_limits<float>::infinity();
        attack.clientTick = 50;
        attack.Serialize(writer);

        ByteStreamReader reader(writer.GetData(), writer.GetSize());
        MsgAttackRequest deserialized;
        bool ok = deserialized.Deserialize(reader);
        TEST_ASSERT(!ok, "Deserialization must reject MsgAttackRequest containing Inf aimAngle");
    }

    // 5.4 Truncated / malformed packet payload safety
    {
        uint8_t garbagePayload[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
        ByteStreamReader reader(garbagePayload, sizeof(garbagePayload));
        MsgPlayerInput truncatedInput;
        bool ok = truncatedInput.Deserialize(reader);
        TEST_ASSERT(!ok, "Deserialization of truncated buffer must safely return false without buffer overrun");
    }
}

// 6. Security Event Auditing & Escalation
void TestSecurityEventAuditing() {
    std::cout << "[TEST] 6. Security Event Auditing & Violation Escalation..." << std::endl;

    ServerSecurityManager security;
    uint16_t maliciousClient = 66;
    security.OnClientConnected(maliciousClient);

    TEST_ASSERT(!security.ShouldDisconnectClient(maliciousClient, 5), "Fresh client must not require disconnect");

    // Generate 6 security violations
    for (int i = 0; i < 6; ++i) {
        security.RecordEvent({
            .type = SecurityEventType::RateLimitExceeded,
            .severity = SecuritySeverity::Violation,
            .clientId = maliciousClient,
            .details = "Rapid packet flood",
            .actionRejected = true
        });
    }

    TEST_ASSERT(security.ShouldDisconnectClient(maliciousClient, 5), "Client exceeding violation threshold must be flagged for disconnect");
    TEST_ASSERT(!security.GetRecentEvents().empty(), "Security event audit log must persist recorded events");
    TEST_ASSERT(security.GetDiagnostics().totalEventsLogged >= 6, "Total events logged metric must be accurate");
}

int main() {
    std::cout << "===========================================================" << std::endl;
    std::cout << "WILDBOUND SECURITY & OPTIMIZATION VERIFICATION SUITE" << std::endl;
    std::cout << "===========================================================" << std::endl;

    TestRateLimiter();
    TestMovementValidation();
    TestCombatValidation();
    TestAtomicLootPickup();
    TestNetworkMessageSanitization();
    TestSecurityEventAuditing();

    std::cout << "\n===========================================================" << std::endl;
    std::cout << "[SUCCESS] ALL 6 SECURITY VALIDATION TEST SUITES PASSED!" << std::endl;
    std::cout << "===========================================================" << std::endl;
    return 0;
}
