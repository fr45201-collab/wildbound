#include "SnapshotBuffer.hpp"
#include <algorithm>

namespace wildbound::network {

SnapshotBuffer::SnapshotBuffer(size_t maxCapacity)
    : m_maxCapacity(std::max<size_t>(2, maxCapacity)) {}

bool SnapshotBuffer::PushSnapshot(WorldSnapshot snapshot) {
    if (!m_snapshots.empty()) {
        if (snapshot.serverTick <= m_latestTick) {
            m_rejectedStaleCount++;
            return false;
        }
    }

    m_latestTick = snapshot.serverTick;
    m_snapshots.push_back(std::move(snapshot));

    while (m_snapshots.size() > m_maxCapacity) {
        m_snapshots.pop_front();
    }

    return true;
}

bool SnapshotBuffer::GetInterpolationSnapshots(float targetRenderTime, WorldSnapshot& outFrom, WorldSnapshot& outTo, float& outAlpha) const {
    if (m_snapshots.empty()) {
        return false;
    }

    if (m_snapshots.size() == 1) {
        outFrom = m_snapshots.front();
        outTo = m_snapshots.front();
        outAlpha = 0.0f;
        return true;
    }

    // Target time is older than the oldest snapshot
    if (targetRenderTime <= m_snapshots.front().timestamp) {
        outFrom = m_snapshots.front();
        outTo = m_snapshots.front();
        outAlpha = 0.0f;
        return true;
    }

    // Target time is newer than the newest snapshot
    if (targetRenderTime >= m_snapshots.back().timestamp) {
        outFrom = m_snapshots.back();
        outTo = m_snapshots.back();
        outAlpha = 1.0f;
        return true;
    }

    // Search for the two snapshots enclosing targetRenderTime
    for (size_t i = 0; i < m_snapshots.size() - 1; ++i) {
        const auto& s0 = m_snapshots[i];
        const auto& s1 = m_snapshots[i + 1];

        if (targetRenderTime >= s0.timestamp && targetRenderTime <= s1.timestamp) {
            outFrom = s0;
            outTo = s1;
            const float dt = s1.timestamp - s0.timestamp;
            if (dt > 0.0001f) {
                outAlpha = std::clamp((targetRenderTime - s0.timestamp) / dt, 0.0f, 1.0f);
            } else {
                outAlpha = 0.0f;
            }
            return true;
        }
    }

    outFrom = m_snapshots.back();
    outTo = m_snapshots.back();
    outAlpha = 1.0f;
    return true;
}

const WorldSnapshot* SnapshotBuffer::GetLatestSnapshot() const noexcept {
    if (m_snapshots.empty()) {
        return nullptr;
    }
    return &m_snapshots.back();
}

const WorldSnapshot* SnapshotBuffer::GetOldestSnapshot() const noexcept {
    if (m_snapshots.empty()) {
        return nullptr;
    }
    return &m_snapshots.front();
}

void SnapshotBuffer::Clear() {
    m_snapshots.clear();
    m_latestTick = 0;
    m_rejectedStaleCount = 0;
}

} // namespace wildbound::network

