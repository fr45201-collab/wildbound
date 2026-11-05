#pragma once

#include "../items/ItemInstance.hpp"
#include <raylib.h>
#include <string>
#include <cstdint>

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::world {

/**
 * Physical Ground Representation of an Item in the World.
 * Renders the 32x32 world representation with elliptical contact drop shadow and gentle floating bob.
 */
class WorldItem {
public:
    WorldItem(uint64_t id, Vector2 position, items::ItemInstance item);
    ~WorldItem() = default;

    void Update(float dt);
    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const items::ItemManager* itemMgr = nullptr);

    [[nodiscard]] uint64_t GetId() const { return m_id; }
    [[nodiscard]] Vector2 GetPosition() const { return m_position; }
    [[nodiscard]] const items::ItemInstance& GetItem() const { return m_item; }
    [[nodiscard]] items::ItemInstance& GetItem() { return m_item; }
    [[nodiscard]] float GetPickupRadius() const { return m_pickupRadius; }

    [[nodiscard]] bool IsCollected() const { return m_isCollected; }
    void MarkCollected() { m_isCollected = true; }

    [[nodiscard]] bool CanPickup(Vector2 playerPos) const;
    [[nodiscard]] Rectangle GetBounds() const;

private:
    uint64_t m_id = 0;
    Vector2 m_position{ 0.0f, 0.0f };
    items::ItemInstance m_item;
    float m_pickupRadius = 36.0f;
    float m_bobTimer = 0.0f;
    bool m_isCollected = false;
};

} // namespace wildbound::world

