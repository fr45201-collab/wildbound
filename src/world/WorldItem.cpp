#include "WorldItem.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../items/ItemManager.hpp"
#include <cmath>

namespace wildbound::world {

WorldItem::WorldItem(uint64_t id, Vector2 position, items::ItemInstance item)
    : m_id(id)
    , m_position(position)
    , m_item(std::move(item))
{
}

void WorldItem::Update(float dt) {
    if (m_isCollected) return;
    m_bobTimer += dt * 3.0f;
}

bool WorldItem::CanPickup(Vector2 playerPos) const {
    if (m_isCollected) return false;
    float dx = playerPos.x - m_position.x;
    float dy = playerPos.y - m_position.y;
    return (dx * dx + dy * dy) <= (m_pickupRadius * m_pickupRadius);
}

Rectangle WorldItem::GetBounds() const {
    return Rectangle{ m_position.x - 16.0f, m_position.y - 16.0f, 32.0f, 32.0f };
}

void WorldItem::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const items::ItemManager* itemMgr) {
    (void)renderer;
    if (m_isCollected) return;

    // Contact drop shadow
    ::DrawEllipse(static_cast<int>(m_position.x), static_cast<int>(m_position.y + 10), 12.0f, 5.0f, Color{ 0, 0, 0, 110 });

    // Floating bob offset
    float bob = std::sin(m_bobTimer) * 2.5f;
    Vector2 renderPos = { m_position.x - 16.0f, m_position.y - 16.0f + bob };

    // Resolve world texture ID from ItemManager or default convention
    std::string worldTexId = "tex_" + m_item.definitionId + "_world";
    if (itemMgr) {
        const auto* def = itemMgr->GetItem(m_item.definitionId);
        if (def && !def->visuals.worldTextureId.empty()) {
            worldTexId = def->visuals.worldTextureId;
        }
    }

    Texture2D tex = assetMgr.GetTexture(worldTexId);
    if (tex.id > 0) {
        ::DrawTextureRec(tex, Rectangle{ 0, 0, 32, 32 }, renderPos, WHITE);
    } else {
        // Fallback procedural diamond indicator
        ::DrawRectanglePro(Rectangle{ renderPos.x + 16, renderPos.y + 16, 16, 16 },
                           Vector2{ 8, 8 }, 45.0f, Color{ 245, 215, 95, 255 });
    }

    // Optional stack quantity indicator
    if (m_item.quantity > 1) {
        std::string qtyStr = "x" + std::to_string(m_item.quantity);
        ::DrawText(qtyStr.c_str(), static_cast<int>(renderPos.x + 18), static_cast<int>(renderPos.y + 18), 10, Color{ 255, 255, 255, 230 });
    }
}

} // namespace wildbound::world

