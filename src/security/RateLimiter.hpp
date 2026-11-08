#pragma once

#include <cstdint>
#include <unordered_map>
#include <chrono>

namespace wildbound::security {

enum class RateLimitCategory : uint8_t {
    GeneralPacket = 0,
    PlayerInput   = 1,
    CombatAction  = 2,
    Interaction   = 3,
    Count         = 4
};

struct RateLimitConfig {
    double tokensPerSecond = 60.0;
    double maxTokens = 80.0;
};

class RateLimiter {
public:
    RateLimiter();

    void Configure(RateLimitCategory category, double tokensPerSecond, double maxBurst);

    /// Checks if a client request is within rate limits. Consumes tokens if allowed.
    [[nodiscard]] bool CheckAndConsume(uint16_t clientId, RateLimitCategory category, double cost = 1.0);

    /// Removes tracking state for a disconnected client
    void RemoveClient(uint16_t clientId);

    /// Clear all client tracking records
    void Reset();

    [[nodiscard]] size_t GetTrackedClientCount() const noexcept { return m_clientBuckets.size(); }

private:
    struct TokenBucket {
        double tokens = 0.0;
        uint64_t lastUpdateTimeMs = 0;
    };

    struct ClientRateState {
        TokenBucket buckets[static_cast<size_t>(RateLimitCategory::Count)];
    };

    RateLimitConfig m_configs[static_cast<size_t>(RateLimitCategory::Count)];
    std::unordered_map<uint16_t, ClientRateState> m_clientBuckets;
    uint64_t GetCurrentTimeMs() const;
};

} // namespace wildbound::security

