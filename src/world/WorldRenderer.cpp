#include "WorldRenderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../graphics/ColorPalette.hpp"
#include "../entities/Entity.hpp"
#include <algorithm>
#include <cmath>

namespace wildbound::world {

WorldRenderer::WorldRenderer() = default;

void WorldRenderer::Render(graphics::Renderer& renderer, const World& world, const graphics::Camera2DSystem& camera,
                           resources::AssetManager& assetMgr, const std::vector<entities::Entity*>& entities)
{
    const Tilemap* tilemap = world.GetTilemap();
    if (!tilemap) return;

    m_renderedTileCount = 0;
    m_renderedObjectCount = 0;

    // 1. Calculate camera-visible world rectangle
    const auto& viewport = renderer.GetViewport();
    float zoom = camera.GetZoom() > 0.05f ? camera.GetZoom() : 1.0f;
    float visibleWidth = static_cast<float>(viewport.GetVirtualWidth()) / zoom;
    float visibleHeight = static_cast<float>(viewport.GetVirtualHeight()) / zoom;
    Vector2 target = camera.GetTarget();

    // Add a 1-tile safety margin around the visible rect to prevent edge pop-in
    int tw = tilemap->GetTileWidth();
    int th = tilemap->GetTileHeight();
    float marginX = static_cast<float>(tw);
    float marginY = static_cast<float>(th);

    m_lastVisibleArea = Rectangle{
        target.x - (visibleWidth * 0.5f) - marginX,
        target.y - (visibleHeight * 0.5f) - marginY,
        visibleWidth + (marginX * 2.0f),
        visibleHeight + (marginY * 2.0f)
    };

    // Calculate visible tile coordinate boundaries
    int startCol = std::max(0, static_cast<int>(std::floor(m_lastVisibleArea.x / tw)));
    int endCol = std::min(tilemap->GetWidth() - 1, static_cast<int>(std::ceil((m_lastVisibleArea.x + m_lastVisibleArea.width) / tw)));
    int startRow = std::max(0, static_cast<int>(std::floor(m_lastVisibleArea.y / th)));
    int endRow = std::min(tilemap->GetHeight() - 1, static_cast<int>(std::ceil((m_lastVisibleArea.y + m_lastVisibleArea.height) / th)));

    // 2. Render Tile Layers in order
    const Tileset* cachedTileset = nullptr;
    Texture2D cachedTexture{ 0 };

    for (const auto& layer : tilemap->GetLayers()) {
        if (!layer.IsVisible() || layer.GetOpacity() <= 0.0f) {
            continue;
        }

        // Skip internal collision layer from visual render pass
        if (layer.GetName() == "Collision" || layer.GetName() == "Collision Metadata") {
            continue;
        }

        Color layerTint = ColorAlpha(WHITE, layer.GetOpacity());

        for (int r = startRow; r <= endRow; ++r) {
            for (int c = startCol; c <= endCol; ++c) {
                int gid = layer.GetTileGid(c, r);
                if (gid <= 0) continue;

                const Tileset* ts = tilemap->FindTilesetForGid(gid);
                if (!ts) continue;

                // Cache tileset texture handle to avoid thousands of string hash lookups per frame
                if (ts != cachedTileset) {
                    cachedTileset = ts;
                    cachedTexture = assetMgr.GetTexture(ts->GetTextureId());
                }
                if (cachedTexture.id == 0) continue;

                Rectangle srcRect = ts->GetTileSourceRect(gid);
                Rectangle destRect{
                    static_cast<float>(c * tw),
                    static_cast<float>(r * th),
                    static_cast<float>(tw),
                    static_cast<float>(th)
                };

                renderer.DrawSprite(cachedTexture, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, layerTint);
                m_renderedTileCount++;
            }
        }
    }

    // 3. Render Environmental Objects and Entities with Camera Frustum Culling & Depth Y-Sorting
    world.GetVisibleObjects(m_lastVisibleArea, m_cachedVisibleObjs);

    m_cachedRenderItems.clear();
    m_cachedRenderItems.reserve(m_cachedVisibleObjs.size() + entities.size());

    for (const auto* obj : m_cachedVisibleObjs) {
        m_cachedRenderItems.push_back({ obj->GetZOrder(), obj, nullptr });
    }

    m_renderedEntityCount = 0;
    m_culledEntityCount = 0;

    // Safety margin (64px) for entity bounds to prevent pop-in at camera borders
    constexpr float entityMargin = 64.0f;
    const float minX = m_lastVisibleArea.x - entityMargin;
    const float maxX = m_lastVisibleArea.x + m_lastVisibleArea.width + entityMargin;
    const float minY = m_lastVisibleArea.y - entityMargin;
    const float maxY = m_lastVisibleArea.y + m_lastVisibleArea.height + entityMargin;

    for (auto* ent : entities) {
        if (!ent || !ent->IsActive()) continue;

        Vector2 pos = ent->GetPosition();
        if (pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY) {
            m_cachedRenderItems.push_back({ pos.y, nullptr, ent });
            m_renderedEntityCount++;
        } else {
            m_culledEntityCount++;
        }
    }

    std::sort(m_cachedRenderItems.begin(), m_cachedRenderItems.end(), [](const RenderItem& a, const RenderItem& b) {
        return a.zOrder < b.zOrder;
    });

    for (const auto& item : m_cachedRenderItems) {
        if (item.entity) {
            item.entity->Render(renderer, assetMgr);
        } else if (item.envObj) {
            if (item.envObj->GetType() == "spawn" || item.envObj->GetTextureId().empty()) {
                continue;
            }
            Texture2D tex = assetMgr.GetTexture(item.envObj->GetTextureId());
            if (tex.id != 0) {
                Rectangle srcRect = item.envObj->GetSourceRect();
                Rectangle destRect = item.envObj->GetWorldRenderBounds();
                renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, item.envObj->GetRotation(), WHITE);
            }
            m_renderedObjectCount++;
        }
    }

    // 4. Debug Visualization Passes
    RenderDebug(world, camera, entities);
}

void WorldRenderer::Render(graphics::Renderer& renderer, const World& world, const graphics::Camera2DSystem& camera,
                           resources::AssetManager& assetMgr, entities::Entity* playerEntity)
{
    std::vector<entities::Entity*> list;
    if (playerEntity) list.push_back(playerEntity);
    Render(renderer, world, camera, assetMgr, list);
}

void WorldRenderer::RenderDebug(const World& world, const graphics::Camera2DSystem& /*camera*/,
                                const std::vector<entities::Entity*>& entities)
{
    const Tilemap* tilemap = world.GetTilemap();
    if (!tilemap) return;

    int tw = tilemap->GetTileWidth();
    int th = tilemap->GetTileHeight();

    // 1. Tile Grid Overlay
    if (m_showGrid) {
        int startCol = std::max(0, static_cast<int>(std::floor(m_lastVisibleArea.x / tw)));
        int endCol = std::min(tilemap->GetWidth(), static_cast<int>(std::ceil((m_lastVisibleArea.x + m_lastVisibleArea.width) / tw)));
        int startRow = std::max(0, static_cast<int>(std::floor(m_lastVisibleArea.y / th)));
        int endRow = std::min(tilemap->GetHeight(), static_cast<int>(std::ceil((m_lastVisibleArea.y + m_lastVisibleArea.height) / th)));

        Color gridColor{ 70, 90, 130, 90 };
        for (int c = startCol; c <= endCol; ++c) {
            float x = static_cast<float>(c * tw);
            DrawLineV(Vector2{ x, static_cast<float>(startRow * th) },
                      Vector2{ x, static_cast<float>(endRow * th) }, gridColor);
        }
        for (int r = startRow; r <= endRow; ++r) {
            float y = static_cast<float>(r * th);
            DrawLineV(Vector2{ static_cast<float>(startCol * tw), y },
                      Vector2{ static_cast<float>(endCol * tw), y }, gridColor);
        }
    }

    // 2. Collision Bounds Overlay
    if (m_showCollision) {
        auto collisions = world.GetCollisionBoxes(m_lastVisibleArea);
        for (const auto& box : collisions) {
            Color fill = box.type == CollisionType::Water ? Color{ 40, 120, 220, 90 } : Color{ 230, 50, 70, 90 };
            Color outline = box.type == CollisionType::Water ? Color{ 80, 180, 255, 230 } : Color{ 255, 80, 90, 230 };
            DrawRectangleRec(box.bounds, fill);
            DrawRectangleLinesEx(box.bounds, 1.0f, outline);
        }

        for (const auto* ent : entities) {
            if (ent && ent->IsActive()) {
                Rectangle col = ent->GetCollisionBounds();
                Color cFill = (ent->GetTag() == "player") ? Color{ 50, 220, 100, 110 } : Color{ 240, 120, 40, 110 };
                Color cOutline = (ent->GetTag() == "player") ? Color{ 80, 255, 120, 240 } : Color{ 255, 160, 60, 240 };
                DrawRectangleRec(col, cFill);
                DrawRectangleLinesEx(col, 1.0f, cOutline);
            }
        }
    }

    // 3. World Boundaries Overlay
    if (m_showBounds) {
        Rectangle bounds = world.GetWorldBounds();
        DrawRectangleLinesEx(bounds, 3.0f, graphics::Palette::DebugBounds);
    }

    // 4. Object Origin Anchors Overlay
    if (m_showOrigins) {
        for (const auto& obj : tilemap->GetObjects()) {
            Vector2 p = obj.GetPosition();
            DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), 3.0f, YELLOW);
            DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), 6.0f, ORANGE);
        }

        for (const auto* ent : entities) {
            if (ent && ent->IsActive()) {
                Vector2 pPos = ent->GetPosition();
                Color oColor = (ent->GetTag() == "player") ? GREEN : RED;
                DrawCircle(static_cast<int>(pPos.x), static_cast<int>(pPos.y), 3.0f, oColor);
                DrawCircleLines(static_cast<int>(pPos.x), static_cast<int>(pPos.y), 7.0f, oColor);
            }
        }
    }
}

void WorldRenderer::RenderDebug(const World& world, const graphics::Camera2DSystem& camera,
                                const entities::Entity* playerEntity)
{
    std::vector<entities::Entity*> list;
    if (playerEntity) list.push_back(const_cast<entities::Entity*>(playerEntity));
    RenderDebug(world, camera, list);
}

} // namespace wildbound::world

