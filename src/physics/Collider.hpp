#pragma once

#include "CollisionTypes.hpp"
#include "OverlapDetector.hpp"
#include <raylib.h>
#include <string>
#include <algorithm>

namespace wildbound::physics {

/**
 * Fundamental Axis-Aligned Bounding Box (AABB) Collider representation.
 * Supports local offsets relative to entity position, dynamic/static classification,
 * layer/mask filtering, and physical vs trigger semantics.
 */
class Collider {
public:
    Collider() = default;

    Collider(ColliderId id, EntityId entityId, CollisionLayer layer, uint32_t mask,
             Vector2 size, Vector2 offset = { 0.0f, 0.0f },
             ColliderType type = ColliderType::Physical,
             const std::string& tag = "collider",
             bool isStatic = false)
        : m_id(id)
        , m_entityId(entityId)
        , m_layer(layer)
        , m_mask(mask)
        , m_size(size)
        , m_offset(offset)
        , m_type(type)
        , m_tag(tag)
        , m_isStatic(isStatic)
    {
        ValidateDimensions();
        UpdateWorldBounds({ 0.0f, 0.0f });
    }

    // Factory helper for trigger volumes
    static Collider CreateTrigger(ColliderId id, EntityId entityId, CollisionLayer layer, uint32_t mask,
                                  Vector2 size, Vector2 offset = { 0.0f, 0.0f },
                                  const std::string& tag = "trigger",
                                  bool isStatic = false) {
        return Collider(id, entityId, layer, mask, size, offset, ColliderType::Trigger, tag, isStatic);
    }

    // Factory helper for static world geometry
    static Collider CreateStatic(ColliderId id, CollisionLayer layer, const Rectangle& bounds,
                                 const std::string& tag = "static_world") {
        Collider col(id, INVALID_ENTITY_ID, layer, GetDefaultCollisionMask(layer),
                     Vector2{ bounds.width, bounds.height }, Vector2{ 0.0f, 0.0f },
                     ColliderType::Physical, tag, true);
        col.m_worldBounds = bounds;
        return col;
    }

    // Identifiers & Tags
    ColliderId GetId() const noexcept { return m_id; }
    void SetId(ColliderId id) noexcept { m_id = id; }

    EntityId GetEntityId() const noexcept { return m_entityId; }
    void SetEntityId(EntityId id) noexcept { m_entityId = id; }

    const std::string& GetTag() const noexcept { return m_tag; }
    void SetTag(const std::string& tag) { m_tag = tag; }

    // Layers & Masks
    CollisionLayer GetLayer() const noexcept { return m_layer; }
    void SetLayer(CollisionLayer layer) noexcept { m_layer = layer; }

    uint32_t GetMask() const noexcept { return m_mask; }
    void SetMask(uint32_t mask) noexcept { m_mask = mask; }

    // Shape & Type
    ColliderType GetType() const noexcept { return m_type; }
    void SetType(ColliderType type) noexcept { m_type = type; }
    bool IsTrigger() const noexcept { return m_type == ColliderType::Trigger; }
    bool IsPhysical() const noexcept { return m_type == ColliderType::Physical; }

    bool IsStatic() const noexcept { return m_isStatic; }
    void SetStatic(bool isStatic) noexcept { m_isStatic = isStatic; }

    bool IsEnabled() const noexcept { return m_enabled; }
    void SetEnabled(bool enabled) noexcept { m_enabled = enabled; }

    // Spatial Properties
    Vector2 GetSize() const noexcept { return m_size; }
    void SetSize(Vector2 size) noexcept {
        m_size = size;
        ValidateDimensions();
    }

    Vector2 GetOffset() const noexcept { return m_offset; }
    void SetOffset(Vector2 offset) noexcept { m_offset = offset; }

    const Rectangle& GetWorldBounds() const noexcept { return m_worldBounds; }
    void SetWorldBounds(const Rectangle& bounds) noexcept { m_worldBounds = bounds; }

    /**
     * Updates world AABB bounds given the owning entity's position anchor (bottom-center).
     */
    void UpdateWorldBounds(Vector2 entityPosition) noexcept {
        if (m_isStatic) return; // Static geometry bounds are pre-computed in world space

        m_worldBounds = Rectangle{
            entityPosition.x - (m_size.x * 0.5f) + m_offset.x,
            entityPosition.y - m_size.y + m_offset.y,
            m_size.x,
            m_size.y
        };
    }

    bool Intersects(const Collider& other) const noexcept {
        if (!m_enabled || !other.m_enabled) return false;
        return OverlapDetector::Intersects(m_worldBounds, other.m_worldBounds);
    }

    bool ContainsPoint(Vector2 point) const noexcept {
        if (!m_enabled) return false;
        return OverlapDetector::ContainsPoint(m_worldBounds, point);
    }

private:
    void ValidateDimensions() noexcept {
        m_size.x = std::max(0.0f, m_size.x);
        m_size.y = std::max(0.0f, m_size.y);
    }

    ColliderId m_id = INVALID_COLLIDER_ID;
    EntityId m_entityId = INVALID_ENTITY_ID;
    CollisionLayer m_layer = CollisionLayer::None;
    uint32_t m_mask = 0u;
    Vector2 m_size{ 0.0f, 0.0f };
    Vector2 m_offset{ 0.0f, 0.0f };
    Rectangle m_worldBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    ColliderType m_type = ColliderType::Physical;
    std::string m_tag;
    bool m_enabled = true;
    bool m_isStatic = false;
};

} // namespace wildbound::physics

