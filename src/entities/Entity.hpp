#pragma once

#include <raylib.h>
#include <string>
#include <cstdint>

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {

/**
 * Abstract base class for all interactive world entities.
 */
class Entity {
public:
    Entity(uint32_t id, const std::string& name, const std::string& tag = "entity");
    virtual ~Entity() = default;

    virtual void Update(float dt) = 0;
    virtual void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) = 0;

    // Spatial Properties (Position represents the logical feet/bottom-center anchor in world space)
    Vector2 GetPosition() const { return m_position; }
    void SetPosition(Vector2 pos) { m_position = pos; }

    virtual Rectangle GetWorldBounds() const = 0;
    virtual Rectangle GetCollisionBounds() const = 0;

    uint32_t GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetTag() const { return m_tag; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

protected:
    uint32_t m_id = 0;
    std::string m_name;
    std::string m_tag;
    Vector2 m_position{ 0.0f, 0.0f };
    bool m_active = true;
};

} // namespace wildbound::entities

