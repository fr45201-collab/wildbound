#include "ReplicationConfiguration.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace wildbound::network {

ReplicationConfiguration ReplicationConfiguration::GetDefault() noexcept {
    return ReplicationConfiguration{};
}

bool ReplicationConfiguration::Validate() {
    bool modified = false;

    if (playerUpdateRate < 1 || playerUpdateRate > 120) {
        playerUpdateRate = std::clamp(playerUpdateRate, 1u, 120u);
        modified = true;
    }
    if (enemyUpdateRate < 1 || enemyUpdateRate > 120) {
        enemyUpdateRate = std::clamp(enemyUpdateRate, 1u, 120u);
        modified = true;
    }
    if (snapshotInterpolationDelayMs < 10 || snapshotInterpolationDelayMs > 1000) {
        snapshotInterpolationDelayMs = std::clamp(snapshotInterpolationDelayMs, 10u, 1000u);
        modified = true;
    }
    if (maxSnapshotBuffer < 4 || maxSnapshotBuffer > 256) {
        maxSnapshotBuffer = std::clamp(maxSnapshotBuffer, 4u, 256u);
        modified = true;
    }
    if (maxReplicatedEntities < 16 || maxReplicatedEntities > 16384) {
        maxReplicatedEntities = std::clamp(maxReplicatedEntities, 16u, 16384u);
        modified = true;
    }
    if (maxReplicationPayloadBytes < 512 || maxReplicationPayloadBytes > 65535) {
        maxReplicationPayloadBytes = std::clamp(maxReplicationPayloadBytes, 512u, 65535u);
        modified = true;
    }

    return !modified;
}

bool ReplicationConfiguration::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("replication")) {
            const auto& rep = j["replication"];
            playerUpdateRate = rep.value("player_update_rate", playerUpdateRate);
            enemyUpdateRate = rep.value("enemy_update_rate", enemyUpdateRate);
            snapshotInterpolationDelayMs = rep.value("snapshot_interpolation_delay_ms", snapshotInterpolationDelayMs);
            maxSnapshotBuffer = rep.value("max_snapshot_buffer", maxSnapshotBuffer);
            enableDeltaSnapshots = rep.value("enable_delta_snapshots", enableDeltaSnapshots);
            enableMovementInterpolation = rep.value("enable_movement_interpolation", enableMovementInterpolation);
            enableLocalPrediction = rep.value("enable_local_prediction", enableLocalPrediction);
            enableEntityRelevancy = rep.value("enable_entity_relevancy", enableEntityRelevancy);
            maxReplicatedEntities = rep.value("max_replicated_entities", maxReplicatedEntities);
            maxReplicationPayloadBytes = rep.value("max_replication_payload_bytes", maxReplicationPayloadBytes);
        }

        Validate();
        return true;
    } catch (...) {
        return false;
    }
}

bool ReplicationConfiguration::SaveToFile(const std::string& filepath) const {
    nlohmann::json j;
    j["replication"] = {
        { "player_update_rate", playerUpdateRate },
        { "enemy_update_rate", enemyUpdateRate },
        { "snapshot_interpolation_delay_ms", snapshotInterpolationDelayMs },
        { "max_snapshot_buffer", maxSnapshotBuffer },
        { "enable_delta_snapshots", enableDeltaSnapshots },
        { "enable_movement_interpolation", enableMovementInterpolation },
        { "enable_local_prediction", enableLocalPrediction },
        { "enable_entity_relevancy", enableEntityRelevancy },
        { "max_replicated_entities", maxReplicatedEntities },
        { "max_replication_payload_bytes", maxReplicationPayloadBytes }
    };

    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(2);
    return true;
}

} // namespace wildbound::network

