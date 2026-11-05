#pragma once

#include <raylib.h>
#include <vector>
#include <cstdint>

namespace wildbound::ai {

struct AvoidanceEntity {
    uint32_t id = 0;
    Vector2 position{ 0.0f, 0.0f };
    float radius = 16.0f;
};

struct AvoidanceConfig {
    float separationRadius = 32.0f;
    float maxForce = 0.6f;
    float weight = 1.0f;
};

class ObstacleAvoidance {
public:
    explicit ObstacleAvoidance(AvoidanceConfig config = {});
    ~ObstacleAvoidance() = default;

    Vector2 ComputeAvoidanceForce(uint32_t selfId, Vector2 selfPos, const std::vector<AvoidanceEntity>& nearbyEntities) const;

    void SetConfig(const AvoidanceConfig& cfg) { m_config = cfg; }
    const AvoidanceConfig& GetConfig() const noexcept { return m_config; }

private:
    AvoidanceConfig m_config;
};

} // namespace wildbound::ai

