#include "ReplicationInterpolator.hpp"
#include <algorithm>
#include <numbers>

namespace wildbound::network {

ReplicationInterpolator::ReplicationInterpolator(float teleportThreshold)
    : m_teleportThreshold(teleportThreshold) {}

void ReplicationInterpolator::InterpolatePosition(float x0, float y0, float x1, float y1, float alpha, float& outX, float& outY) const noexcept {
    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float distSq = dx * dx + dy * dy;

    // If movement exceeds teleport threshold, snap instantly to target
    if (distSq > m_teleportThreshold * m_teleportThreshold) {
        outX = x1;
        outY = y1;
        return;
    }

    const float clampedAlpha = std::clamp(alpha, 0.0f, 1.0f);
    outX = x0 + dx * clampedAlpha;
    outY = y0 + dy * clampedAlpha;
}

float ReplicationInterpolator::InterpolateAngleRadians(float a0, float a1, float alpha) const noexcept {
    constexpr float TWO_PI = 2.0f * 3.14159265358979323846f;

    // Normalize diff to [-PI, PI]
    float diff = std::fmod(a1 - a0, TWO_PI);
    if (diff < -3.14159265358979323846f) diff += TWO_PI;
    if (diff > 3.14159265358979323846f) diff -= TWO_PI;

    const float clampedAlpha = std::clamp(alpha, 0.0f, 1.0f);
    return a0 + diff * clampedAlpha;
}

EntitySnapshotRecord ReplicationInterpolator::InterpolateRecord(const EntitySnapshotRecord& from, const EntitySnapshotRecord& to, float alpha) const noexcept {
    EntitySnapshotRecord result = to; // Default discrete/vital fields to target state

    InterpolatePosition(from.posX, from.posY, to.posX, to.posY, alpha, result.posX, result.posY);

    const float clampedAlpha = std::clamp(alpha, 0.0f, 1.0f);
    result.velX = from.velX + (to.velX - from.velX) * clampedAlpha;
    result.velY = from.velY + (to.velY - from.velY) * clampedAlpha;
    result.facingAngle = InterpolateAngleRadians(from.facingAngle, to.facingAngle, alpha);

    // Discrete flags & vitals take latest values (or linear health blend if desired)
    result.health = to.health;
    result.maxHealth = to.maxHealth;
    result.armor = to.armor;
    result.animState = to.animState;
    result.statusFlags = to.statusFlags;
    result.customData1 = to.customData1;
    result.customData2 = to.customData2;

    return result;
}

} // namespace wildbound::network

