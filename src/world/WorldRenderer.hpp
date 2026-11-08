#pragma once

#include "World.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/Camera2D.hpp"
#include <raylib.h>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {
    class Entity;
}

namespace wildbound::world {

/**
 * High-performance, camera-culled World and Tilemap Renderer.
 * Employs visible-tile bound clamping and Y-sorted object depth.
 */
class WorldRenderer {
public:
    WorldRenderer();

    void Render(graphics::Renderer& renderer, const World& world, const graphics::Camera2DSystem& camera,
                resources::AssetManager& assetMgr, const std::vector<entities::Entity*>& entities);

    void Render(graphics::Renderer& renderer, const World& world, const graphics::Camera2DSystem& camera,
                resources::AssetManager& assetMgr, entities::Entity* playerEntity = nullptr);

    void RenderDebug(const World& world, const graphics::Camera2DSystem& camera,
                     const std::vector<entities::Entity*>& entities);

    void RenderDebug(const World& world, const graphics::Camera2DSystem& camera,
                     const entities::Entity* playerEntity = nullptr);

    // Debug visualization toggles
    void SetShowGrid(bool show) { m_showGrid = show; }
    bool GetShowGrid() const { return m_showGrid; }

    void SetShowCollision(bool show) { m_showCollision = show; }
    bool GetShowCollision() const { return m_showCollision; }

    void SetShowBounds(bool show) { m_showBounds = show; }
    bool GetShowBounds() const { return m_showBounds; }

    void SetShowOrigins(bool show) { m_showOrigins = show; }
    bool GetShowOrigins() const { return m_showOrigins; }

    // Metrics
    size_t GetRenderedTileCount() const { return m_renderedTileCount; }
    size_t GetRenderedObjectCount() const { return m_renderedObjectCount; }
    size_t GetRenderedEntityCount() const { return m_renderedEntityCount; }
    size_t GetCulledEntityCount() const { return m_culledEntityCount; }
    Rectangle GetVisibleWorldArea() const { return m_lastVisibleArea; }

private:
    struct RenderItem {
        float zOrder = 0.0f;
        const EnvironmentObject* envObj = nullptr;
        entities::Entity* entity = nullptr;
    };

    bool m_showGrid = false;
    bool m_showCollision = false;
    bool m_showBounds = true;
    bool m_showOrigins = false;

    size_t m_renderedTileCount = 0;
    size_t m_renderedObjectCount = 0;
    size_t m_renderedEntityCount = 0;
    size_t m_culledEntityCount = 0;
    Rectangle m_lastVisibleArea{ 0.0f, 0.0f, 0.0f, 0.0f };

    // Persistent scratch containers avoiding per-frame heap allocations
    std::vector<const EnvironmentObject*> m_cachedVisibleObjs;
    std::vector<RenderItem> m_cachedRenderItems;
};

} // namespace wildbound::world

