#pragma once

#include "SecurityTypes.hpp"
#include "RateLimiter.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <memory>

namespace wildbound::security {

struct SecurityDiagnostics {
    uint64_t totalEventsLogged{ 0 };
    uint64_t totalRejectedActions{ 0 };
    uint64_t movementViolations{ 0 };
    uint64_t combatViolations{ 0 };
    uint64_t lootViolations{ 0 };
    uint64_t rateLimitViolations{ 0 };
};

class ServerSecurityManager {
public:
    ServerSecurityManager();
    ~ServerSecurityManager() = default;

    /// Reset state between matches
    void Reset();

    /// Client connection lifecycle
    void OnClientConnected(uint16_t clientId);
    void OnClientDisconnected(uint16_t clientId);

    /// Rate limiting check
    [[nodiscard]] bool CheckRateLimit(uint16_t clientId, RateLimitCategory category, uint16_t messageType);

    /// Authoritative Movement Validation & Sanitization
    [[nodiscard]] bool ValidateMovement(uint16_t clientId, uint32_t participantId,
                                        Vector2 currentPos, Vector2 requestedPos, float dt,
                                        float maxAllowedSpeed, Vector2& outValidatedPos);

    /// Authoritative Combat Action Validation
    [[nodiscard]] bool ValidateAttack(uint16_t clientId, uint32_t participantId,
                                      const std::string& weaponId, Vector2 attackerPos, Vector2 targetPos,
                                      float maxWeaponRange, bool isWeaponReady);

    /// Atomic Loot Pickup Transaction Validation
    [[nodiscard]] bool ValidateLootPickup(uint16_t clientId, uint32_t participantId,
                                          uint64_t lootId, Vector2 playerPos, Vector2 lootPos,
                                          float maxPickupRadius);

    /// Record a structured security event
    void RecordEvent(SecurityEvent event);

    /// Check if client has accumulated excessive violations requiring disconnect
    [[nodiscard]] bool ShouldDisconnectClient(uint16_t clientId, uint32_t threshold = 15) const noexcept;

    /// Diagnostics and inspection
    [[nodiscard]] const SecurityDiagnostics& GetDiagnostics() const noexcept { return m_diagnostics; }
    [[nodiscard]] const std::deque<SecurityEvent>& GetRecentEvents() const noexcept { return m_recentEvents; }
    [[nodiscard]] uint32_t GetClientViolationCount(uint16_t clientId) const noexcept;

    [[nodiscard]] RateLimiter& GetRateLimiter() noexcept { return m_rateLimiter; }
    [[nodiscard]] const RateLimiter& GetRateLimiter() const noexcept { return m_rateLimiter; }

private:
    RateLimiter m_rateLimiter;
    SecurityDiagnostics m_diagnostics;
    std::deque<SecurityEvent> m_recentEvents;
    static constexpr size_t MAX_RECENT_EVENTS = 120;

    std::unordered_map<uint16_t, uint32_t> m_clientViolations;
    std::unordered_map<uint32_t, uint64_t> m_lastAttackTimeMs;
    std::unordered_set<uint64_t> m_claimedLootIds; // Atomic resolution of claimed loot
    std::string m_activeMatchId{ "wildbound_default" };

    uint64_t GetCurrentTimeMs() const;
};

} // namespace wildbound::security
