#pragma once

#include "Snapshot.hpp"
#include <cmath>

namespace wildbound::network {

/**
 * Handles smooth mathematical interpolation and extrapolation of replicated transform states.
 * Enforces angle wrapping and snap-to thresholds for sudden teleports or respawns.
 */
class ReplicationInterpolator {
public:
    explicit ReplicationInterpolator(float teleportThreshold = 250.0f);
    ~ReplicationInterpolator() = default;

    /// Linearly interpolate 2D position with teleport boundary protection
    [[nodiscard]] void InterpolatePosition(float x0, float y0, float x1, float y1, float alpha, float& outX, float& outY) const noexcept;

    /// Shortest-path angular interpolation (in radians)
    [[nodiscard]] float InterpolateAngleRadians(float a0, float a1, float alpha) const noexcept;

    /// Interpolate an entire EntitySnapshotRecord between from and to records
    [[nodiscard]] EntitySnapshotRecord InterpolateRecord(const EntitySnapshotRecord& from, const EntitySnapshotRecord& to, float alpha) const noexcept;

    [[nodiscard]] float GetTeleportThreshold() const noexcept { return m_teleportThreshold; }
    void SetTeleportThreshold(float threshold) noexcept { m_teleportThreshold = threshold; }

private:
    float m_teleportThreshold{ 250.0f };
};

} // namespace wildbound::network

