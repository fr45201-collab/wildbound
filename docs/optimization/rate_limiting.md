# Wildbound — Rate Limiting & Abuse Prevention

**Phase:** 47  
**Status:** Completed  
**Subsystems:** RateLimiter, ServerSecurityManager, DedicatedServer  

---

## 1. Rate Limiting Strategy & Objectives

Unbounded packet processing allows malicious or buggy clients to perform denial-of-service (DoS) on dedicated servers, starve other clients of CPU cycles, or trigger integer overflows.

### Objectives:
1. Limit ingress datagrams per connected client.
2. Isolate traffic categories so packet bursts in one domain (e.g. chat or cosmetics queries) cannot choke critical gameplay packets (e.g. movement or attacks).
3. Allow legitimate burstiness (e.g. rapid direction changes or ability combos) while throttling sustained abuse.

---

## 2. Token Bucket Implementation (`RateLimiter.cpp`)

The token-bucket algorithm refills tokens continuously at a designated rate up to a configured maximum burst capacity.

### Category Configuration:

| Category | Category Enum | Refill Rate (Tokens/sec) | Max Burst Capacity | Typical Packets |
| :--- | :--- | :--- | :--- | :--- |
| **GeneralPacket** | `0` | 60.0 | 80.0 | Hello, Ping, Disconnect, Appearance |
| **PlayerInput** | `1` | 40.0 | 60.0 | `MsgPlayerInput` (Client tick inputs) |
| **CombatAction** | `2` | 15.0 | 25.0 | `MsgAttackRequest`, Ability activations |
| **Interaction** | `3` | 10.0 | 20.0 | `MsgPickupRequest`, Chest open, NPC interact |

### Algorithm:
```cpp
bool RateLimiter::CheckAndConsume(uint16_t clientId, RateLimitCategory category, double cost) {
    auto& state = m_clientBuckets[clientId];
    auto& bucket = state.buckets[static_cast<size_t>(category)];
    const auto& config = m_configs[static_cast<size_t>(category)];

    uint64_t nowMs = GetCurrentTimeMs();
    if (bucket.lastUpdateTimeMs == 0) {
        bucket.lastUpdateTimeMs = nowMs;
        bucket.tokens = config.maxTokens;
    } else {
        double elapsedSec = static_cast<double>(nowMs - bucket.lastUpdateTimeMs) / 1000.0;
        bucket.tokens = std::min(config.maxTokens, bucket.tokens + elapsedSec * config.tokensPerSecond);
        bucket.lastUpdateTimeMs = nowMs;
    }

    if (bucket.tokens >= cost) {
        bucket.tokens -= cost;
        return true;
    }
    return false;
}
```

---

## 3. Disconnection & Cleanup Lifecycle

- When a client disconnects (`ClientDisconnected` event), `RateLimiter::RemoveClient(clientId)` erases the client's token buckets.
- Zero memory leakage over long-running server lifespans with high player turnover.
- Match resets invoke `RateLimiter::Reset()`, wiping all client tracking entries.

