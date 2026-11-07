#pragma once

#include "Snapshot.hpp"
#include <deque>
#include <cstdint>

namespace wildbound::network {

/**
 * Client-side circular snapshot buffer for smooth interpolation of remote entities.
 * Enforces strictly increasing server ticks and rejects stale/out-of-order snapshots.
 */
class SnapshotBuffer {
public:
    explicit SnapshotBuffer(size_t maxCapacity = 32);
    ~SnapshotBuffer() = default;

    /// Attempt to push a new snapshot. Returns false if stale (serverTick <= latestTick).
    bool PushSnapshot(WorldSnapshot snapshot);

    /// Find two snapshots bounding the given target render time and calculate the interpolation factor alpha [0.0, 1.0]
    [[nodiscard]] bool GetInterpolationSnapshots(float targetRenderTime, WorldSnapshot& outFrom, WorldSnapshot& outTo, float& outAlpha) const;

    /// Get pointer to the most recently received snapshot (or nullptr if empty)
    [[nodiscard]] const WorldSnapshot* GetLatestSnapshot() const noexcept;

    /// Get pointer to the oldest stored snapshot (or nullptr if empty)
    [[nodiscard]] const WorldSnapshot* GetOldestSnapshot() const noexcept;

    /// Number of snapshots currently in the buffer
    [[nodiscard]] size_t GetSnapshotCount() const noexcept { return m_snapshots.size(); }

    /// Maximum capacity of the buffer
    [[nodiscard]] size_t GetMaxCapacity() const noexcept { return m_maxCapacity; }

    /// Total number of stale or out-of-order snapshots rejected
    [[nodiscard]] uint64_t GetRejectedStaleCount() const noexcept { return m_rejectedStaleCount; }

    /// Clear all snapshots
    void Clear();

private:
    size_t m_maxCapacity{ 32 };
    std::deque<WorldSnapshot> m_snapshots;
    TickType m_latestTick{ 0 };
    uint64_t m_rejectedStaleCount{ 0 };
};

} // namespace wildbound::network

