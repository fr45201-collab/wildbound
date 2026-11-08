#include "ServerSecurityManager.hpp"
#include "../core/Logger.hpp"
#include <cmath>
#include <chrono>

namespace wildbound::security {

ServerSecurityManager::ServerSecurityManager() = default;

void ServerSecurityManager::Reset() {
    m_rateLimiter.Reset();
    m_clientViolations.clear();
    m_lastAttackTimeMs.clear();
    m_claimedLootIds.clear();
    m_recentEvents.clear();
}

void ServerSecurityManager::OnClientConnected(uint16_t clientId) {
    m_clientViolations[clientId] = 0;
}

void ServerSecurityManager::OnClientDisconnected(uint16_t clientId) {
    m_clientViolations.erase(clientId);
    m_rateLimiter.RemoveClient(clientId);
}

uint64_t ServerSecurityManager::GetCurrentTimeMs() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

void ServerSecurityManager::RecordEvent(SecurityEvent event) {
    if (event.timestampMs == 0) {
        event.timestampMs = GetCurrentTimeMs();
    }
    if (event.matchId.empty()) {
        event.matchId = m_activeMatchId;
    }

    m_diagnostics.totalEventsLogged++;
    if (event.actionRejected) {
        m_diagnostics.totalRejectedActions++;
    }

    if (event.severity >= SecuritySeverity::Violation) {
        m_clientViolations[event.clientId]++;
    }

    // Console logging for severe events
    if (event.severity >= SecuritySeverity::Violation) {
        core::Logger::Warn("SECURITY", "[" + std::string(ToString(event.type)) + "] Client #" +
                           std::to_string(event.clientId) + ": " + event.details);
    } else {
        core::Logger::Debug("SECURITY", "[" + std::string(ToString(event.type)) + "] Client #" +
                            std::to_string(event.clientId) + ": " + event.details);
    }

    // Maintain bounded ring buffer
    if (m_recentEvents.size() >= MAX_RECENT_EVENTS) {
        m_recentEvents.pop_front();
    }
    m_recentEvents.push_back(std::move(event));
}

uint32_t ServerSecurityManager::GetClientViolationCount(uint16_t clientId) const noexcept {
    auto it = m_clientViolations.find(clientId);
    return (it != m_clientViolations.end()) ? it->second : 0;
}

bool ServerSecurityManager::ShouldDisconnectClient(uint16_t clientId, uint32_t threshold) const noexcept {
    return GetClientViolationCount(clientId) >= threshold;
}

bool ServerSecurityManager::CheckRateLimit(uint16_t clientId, RateLimitCategory category, uint16_t messageType) {
    if (!m_rateLimiter.CheckAndConsume(clientId, category)) {
        m_diagnostics.rateLimitViolations++;
        RecordEvent({
            .type = SecurityEventType::RateLimitExceeded,
            .severity = SecuritySeverity::Warning,
            .clientId = clientId,
            .details = "Rate limit exceeded on category " + std::to_string(static_cast<int>(category)) +
                       " (msgType: " + std::to_string(messageType) + ")",
            .actionRejected = true
        });
        return false;
    }
    return true;
}

bool ServerSecurityManager::ValidateMovement(uint16_t clientId, uint32_t participantId,
                                            Vector2 currentPos, Vector2 requestedPos, float dt,
                                            float maxAllowedSpeed, Vector2& outValidatedPos)
{
    // 1. Validate numerical sanity (NaN / Inf protection)
    if (!std::isfinite(requestedPos.x) || !std::isfinite(requestedPos.y)) {
        m_diagnostics.movementViolations++;
        outValidatedPos = currentPos;
        RecordEvent({
            .type = SecurityEventType::NonFiniteCoordinates,
            .severity = SecuritySeverity::Violation,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Non-finite coordinates received from client: (" +
                       std::to_string(requestedPos.x) + ", " + std::to_string(requestedPos.y) + ")",
            .actionRejected = true
        });
        return false;
    }

    // Clamp dt to reasonable bounds (0.001s to 0.25s) to prevent simulation exploits
    float effectiveDt = std::max(0.001f, std::min(dt, 0.25f));

    float dx = requestedPos.x - currentPos.x;
    float dy = requestedPos.y - currentPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Maximum distance allows standard speed * dt + 35% margin for network latency/jitter + 8px tolerance
    float maxAllowedDist = (maxAllowedSpeed * effectiveDt * 1.35f) + 8.0f;

    if (distance > maxAllowedDist) {
        m_diagnostics.movementViolations++;

        // Distinguish impossible teleportation vs modest speed hack
        if (distance > maxAllowedDist * 3.5f) {
            outValidatedPos = currentPos; // Snap back to authoritative position
            RecordEvent({
                .type = SecurityEventType::TeleportAttempt,
                .severity = SecuritySeverity::Critical,
                .clientId = clientId,
                .participantId = participantId,
                .details = "Teleportation anomaly: requested " + std::to_string(distance) +
                           " px vs max " + std::to_string(maxAllowedDist) + " px",
                .actionRejected = true
            });
            return false;
        } else {
            // Clamp movement vector to maximum allowed distance
            float factor = maxAllowedDist / distance;
            outValidatedPos = Vector2{ currentPos.x + dx * factor, currentPos.y + dy * factor };
            RecordEvent({
                .type = SecurityEventType::InvalidMovementSpeed,
                .severity = SecuritySeverity::Warning,
                .clientId = clientId,
                .participantId = participantId,
                .details = "Excessive movement speed: distance " + std::to_string(distance) +
                           " clamped to " + std::to_string(maxAllowedDist),
                .actionRejected = true
            });
            return false;
        }
    }

    outValidatedPos = requestedPos;
    return true;
}

bool ServerSecurityManager::ValidateAttack(uint16_t clientId, uint32_t participantId,
                                          const std::string& weaponId, Vector2 attackerPos, Vector2 targetPos,
                                          float maxWeaponRange, bool isWeaponReady)
{
    // 1. Attack Cooldown check
    if (!isWeaponReady) {
        m_diagnostics.combatViolations++;
        RecordEvent({
            .type = SecurityEventType::AttackCooldownViolation,
            .severity = SecuritySeverity::Warning,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Attack rejected: weapon [" + weaponId + "] is on cooldown",
            .actionRejected = true
        });
        return false;
    }

    uint64_t nowMs = GetCurrentTimeMs();
    auto it = m_lastAttackTimeMs.find(participantId);
    if (it != m_lastAttackTimeMs.end() && (nowMs - it->second < 150)) {
        m_diagnostics.combatViolations++;
        RecordEvent({
            .type = SecurityEventType::AttackCooldownViolation,
            .severity = SecuritySeverity::Warning,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Attack rate violation: consecutive attack within " + std::to_string(nowMs - it->second) + "ms",
            .actionRejected = true
        });
        return false;
    }

    // 2. Numerical sanity
    if (!std::isfinite(attackerPos.x) || !std::isfinite(attackerPos.y) ||
        !std::isfinite(targetPos.x) || !std::isfinite(targetPos.y)) {
        m_diagnostics.combatViolations++;
        RecordEvent({
            .type = SecurityEventType::NonFiniteCoordinates,
            .severity = SecuritySeverity::Violation,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Attack rejected: non-finite combat coordinates",
            .actionRejected = true
        });
        return false;
    }

    // 3. Physical range check (with 25% tolerance for ping & hitboxes)
    float dx = targetPos.x - attackerPos.x;
    float dy = targetPos.y - attackerPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    float maxAllowedRange = (maxWeaponRange * 1.25f) + 12.0f;

    if (dist > maxAllowedRange) {
        m_diagnostics.combatViolations++;
        RecordEvent({
            .type = SecurityEventType::AttackRangeViolation,
            .severity = SecuritySeverity::Violation,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Attack range anomaly: target distance " + std::to_string(dist) +
                       " exceeds max allowed range " + std::to_string(maxAllowedRange),
            .actionRejected = true
        });
        return false;
    }

    m_lastAttackTimeMs[participantId] = nowMs;
    return true;
}

bool ServerSecurityManager::ValidateLootPickup(uint16_t clientId, uint32_t participantId,
                                              uint64_t lootId, Vector2 playerPos, Vector2 lootPos,
                                              float maxPickupRadius)
{
    // 1. Atomic resolution: verify loot item was not already claimed
    if (m_claimedLootIds.find(lootId) != m_claimedLootIds.end()) {
        m_diagnostics.lootViolations++;
        RecordEvent({
            .type = SecurityEventType::DuplicateLootClaim,
            .severity = SecuritySeverity::Violation,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Duplicate loot pickup attempt on already-claimed loot #" + std::to_string(lootId),
            .actionRejected = true
        });
        return false;
    }

    // 2. Interaction range check
    float dx = lootPos.x - playerPos.x;
    float dy = lootPos.y - playerPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    float maxAllowedDist = (maxPickupRadius * 1.30f) + 16.0f;

    if (dist > maxAllowedDist) {
        m_diagnostics.lootViolations++;
        RecordEvent({
            .type = SecurityEventType::LootRangeViolation,
            .severity = SecuritySeverity::Warning,
            .clientId = clientId,
            .participantId = participantId,
            .details = "Loot pickup out of physical reach: distance " + std::to_string(dist) +
                       " exceeds radius " + std::to_string(maxAllowedDist),
            .actionRejected = true
        });
        return false;
    }

    // Atomically claim the item
    m_claimedLootIds.insert(lootId);
    return true;
}

} // namespace wildbound::security
