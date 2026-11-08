#include "RateLimiter.hpp"
#include <algorithm>

namespace wildbound::security {

RateLimiter::RateLimiter() {
    // Default safe production thresholds
    Configure(RateLimitCategory::GeneralPacket, 120.0, 160.0);
    Configure(RateLimitCategory::PlayerInput,   75.0,  90.0);
    Configure(RateLimitCategory::CombatAction,  10.0,  15.0);
    Configure(RateLimitCategory::Interaction,   8.0,   12.0);
}

void RateLimiter::Configure(RateLimitCategory category, double tokensPerSecond, double maxBurst) {
    size_t idx = static_cast<size_t>(category);
    if (idx < static_cast<size_t>(RateLimitCategory::Count)) {
        m_configs[idx].tokensPerSecond = tokensPerSecond;
        m_configs[idx].maxTokens = maxBurst;
    }
}

uint64_t RateLimiter::GetCurrentTimeMs() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

bool RateLimiter::CheckAndConsume(uint16_t clientId, RateLimitCategory category, double cost) {
    size_t catIdx = static_cast<size_t>(category);
    if (catIdx >= static_cast<size_t>(RateLimitCategory::Count)) {
        return false;
    }

    uint64_t nowMs = GetCurrentTimeMs();
    auto& clientState = m_clientBuckets[clientId];
    auto& bucket = clientState.buckets[catIdx];
    const auto& config = m_configs[catIdx];

    if (bucket.lastUpdateTimeMs == 0) {
        bucket.tokens = config.maxTokens;
        bucket.lastUpdateTimeMs = nowMs;
    } else {
        uint64_t elapsedMs = nowMs - bucket.lastUpdateTimeMs;
        if (elapsedMs > 0) {
            double addedTokens = (static_cast<double>(elapsedMs) / 1000.0) * config.tokensPerSecond;
            bucket.tokens = std::min(config.maxTokens, bucket.tokens + addedTokens);
            bucket.lastUpdateTimeMs = nowMs;
        }
    }

    if (bucket.tokens >= cost) {
        bucket.tokens -= cost;
        return true;
    }

    return false; // Rate limit exceeded
}

void RateLimiter::RemoveClient(uint16_t clientId) {
    m_clientBuckets.erase(clientId);
}

void RateLimiter::Reset() {
    m_clientBuckets.clear();
}

} // namespace wildbound::security

