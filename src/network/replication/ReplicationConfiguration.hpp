#pragma once

#include <cstdint>
#include <string>

namespace wildbound::network {

struct ReplicationConfiguration {
    uint32_t playerUpdateRate{ 20 };               // Hz
    uint32_t enemyUpdateRate{ 15 };                // Hz
    uint32_t snapshotInterpolationDelayMs{ 100 };  // ms
    uint32_t maxSnapshotBuffer{ 32 };              // frames
    bool enableDeltaSnapshots{ true };
    bool enableMovementInterpolation{ true };
    bool enableLocalPrediction{ false };
    bool enableEntityRelevancy{ false };
    uint32_t maxReplicatedEntities{ 2048 };
    uint32_t maxReplicationPayloadBytes{ 65536 };

    /// Load from JSON file (checks path or fallback)
    bool LoadFromFile(const std::string& filepath);

    /// Save configuration to JSON file
    bool SaveToFile(const std::string& filepath) const;

    /// Validate values against hard limits and clamp if out of range
    bool Validate();

    /// Get standard production defaults
    static ReplicationConfiguration GetDefault() noexcept;
};

} // namespace wildbound::network

