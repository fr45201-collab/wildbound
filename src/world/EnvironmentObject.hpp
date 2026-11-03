#pragma once

#include "CollisionMetadata.hpp"
#include <raylib.h>
#include <string>

namespace wildbound::world {

/**
 * Data-driven environmental object representation.
 */
class EnvironmentObject {
public:
    EnvironmentObject();
    EnvironmentObject(int id, const std::string& name, const std::string& type,
                      Vector2 position, Vector2 size, const std::string& textureId,
                      Rectangle sourceRect, Vector2 origin = { 0.0f, 0.0f },
                      float scale = 1.0f, float rotation = 0.0f);

    int GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetType() const { return m_type; }
    Vector2 GetPosition() const { return m_position; }
    Vector2 GetSize() const { return m_size; }
    const std::string& GetTextureId() const { return m_textureId; }
    Rectangle GetSourceRect() const { return m_sourceRect; }
    Vector2 GetOrigin() const { return m_origin; }
    float GetScale() const { return m_scale; }
    float GetRotation() const { return m_rotation; }

    void SetPosition(Vector2 pos);
    void SetOrigin(Vector2 origin) { m_origin = origin; }
    void SetScale(float s) { m_scale = s; }

    // Collision
    bool HasCollision() const { return m_hasCollision; }
    void SetCollision(const CollisionBox& box);
    const CollisionBox& GetCollision() const { return m_collision; }
    CollisionBox GetWorldCollision() const;

    // Bounds for camera culling & depth sorting
    Rectangle GetWorldRenderBounds() const;
    float GetZOrder() const { return m_position.y; }

private:
    int m_id = 0;
    std::string m_name;
    std::string m_type;
    Vector2 m_position{ 0.0f, 0.0f };
    Vector2 m_size{ 32.0f, 32.0f };
    std::string m_textureId;
    Rectangle m_sourceRect{ 0.0f, 0.0f, 32.0f, 32.0f };
    Vector2 m_origin{ 0.0f, 0.0f };
    float m_scale = 1.0f;
    float m_rotation = 0.0f;

    bool m_hasCollision = false;
    CollisionBox m_collision;
};

} // namespace wildbound::world

