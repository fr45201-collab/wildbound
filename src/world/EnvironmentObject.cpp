#include "EnvironmentObject.hpp"

namespace wildbound::world {

EnvironmentObject::EnvironmentObject() = default;

EnvironmentObject::EnvironmentObject(int id, const std::string& name, const std::string& type,
                                     Vector2 position, Vector2 size, const std::string& textureId,
                                     Rectangle sourceRect, Vector2 origin,
                                     float scale, float rotation)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_position(position)
    , m_size(size)
    , m_textureId(textureId)
    , m_sourceRect(sourceRect)
    , m_origin(origin)
    , m_scale(scale)
    , m_rotation(rotation)
{
}

void EnvironmentObject::SetPosition(Vector2 pos) {
    m_position = pos;
    if (m_hasCollision) {
        // Adjust collision box position
        m_collision.bounds.x = m_position.x - m_origin.x + (m_size.x - m_collision.bounds.width) * 0.5f;
        m_collision.bounds.y = m_position.y - m_collision.bounds.height;
    }
}

void EnvironmentObject::SetCollision(const CollisionBox& box) {
    m_collision = box;
    m_hasCollision = (box.type != CollisionType::None && box.bounds.width > 0.0f && box.bounds.height > 0.0f);
    if (m_hasCollision) {
        // Adjust collision box position to align with object footprint base
        m_collision.bounds.x = m_position.x - m_origin.x + (m_size.x - m_collision.bounds.width) * 0.5f;
        m_collision.bounds.y = m_position.y - m_collision.bounds.height;
    }
}

CollisionBox EnvironmentObject::GetWorldCollision() const {
    if (!m_hasCollision) {
        return CollisionBox{};
    }
    CollisionBox worldBox = m_collision;
    // Bounds are already in world space or offset from anchor
    return worldBox;
}

Rectangle EnvironmentObject::GetWorldRenderBounds() const {
    float w = m_size.x * m_scale;
    float h = m_size.y * m_scale;
    return Rectangle{
        m_position.x - m_origin.x * m_scale,
        m_position.y - m_origin.y * m_scale,
        w,
        h
    };
}

} // namespace wildbound::world

