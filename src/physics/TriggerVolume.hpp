#pragma once

#include "Collider.hpp"
#include "CollisionEvent.hpp"
#include <unordered_set>
#include <vector>
#include <string>

namespace wildbound::physics {

/**
 * Dedicated Trigger Volume component managing spatial overlap tracking.
 * Triggers do not exert physical forces or block entities; they produce
 * TriggerEnter, TriggerStay, and TriggerExit lifecycle events.
 */
class TriggerVolume {
public:
    TriggerVolume(ColliderId colliderId, EntityId ownerEntityId,
                  CollisionLayer layer = CollisionLayer::Trigger,
                  uint32_t mask = GetDefaultCollisionMask(CollisionLayer::Trigger),
                  Vector2 size = { 32.0f, 32.0f },
                  Vector2 offset = { 0.0f, 0.0f },
                  const std::string& name = "trigger_volume")
        : m_name(name)
        , m_collider(Collider::CreateTrigger(colliderId, ownerEntityId, layer, mask, size, offset, name))
    {
    }

    const std::string& GetName() const noexcept { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    const Collider& GetCollider() const noexcept { return m_collider; }
    Collider& GetCollider() noexcept { return m_collider; }

    ColliderId GetColliderId() const noexcept { return m_collider.GetId(); }
    EntityId GetOwnerEntityId() const noexcept { return m_collider.GetEntityId(); }

    bool IsEnabled() const noexcept { return m_collider.IsEnabled(); }
    void SetEnabled(bool enabled) noexcept { m_collider.SetEnabled(enabled); }

    void UpdatePosition(Vector2 position) noexcept {
        m_collider.UpdateWorldBounds(position);
    }

    // Overlap Tracking Queries
    bool IsEntityInside(EntityId entityId) const noexcept {
        return m_entitiesInside.find(entityId) != m_entitiesInside.end();
    }

    size_t GetEntityCount() const noexcept {
        return m_entitiesInside.size();
    }

    const std::unordered_set<EntityId>& GetEntitiesInside() const noexcept {
        return m_entitiesInside;
    }

    // Called by CollisionManager to maintain active overlap state
    void OnEntityEnter(EntityId entityId) {
        if (entityId != INVALID_ENTITY_ID) {
            m_entitiesInside.insert(entityId);
        }
    }

    void OnEntityExit(EntityId entityId) {
        m_entitiesInside.erase(entityId);
    }

    void Clear() noexcept {
        m_entitiesInside.clear();
    }

private:
    std::string m_name;
    Collider m_collider;
    std::unordered_set<EntityId> m_entitiesInside;
};

} // namespace wildbound::physics

