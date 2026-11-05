#include "CollisionDebugRenderer.hpp"
#include <string>

namespace wildbound::physics {

Color CollisionDebugRenderer::GetLayerColor(CollisionLayer layer, float alpha) {
    uint8_t a = static_cast<uint8_t>(alpha * 255.0f);
    switch (layer) {
        case CollisionLayer::World:        return Color{ 220, 50, 50, a };    // Red
        case CollisionLayer::Player:       return Color{ 50, 220, 90, a };    // Vibrant Green
        case CollisionLayer::Enemy:        return Color{ 240, 130, 30, a };   // Orange
        case CollisionLayer::Wildlife:     return Color{ 60, 200, 230, a };   // Cyan
        case CollisionLayer::Projectile:   return Color{ 240, 230, 40, a };   // Yellow
        case CollisionLayer::Pickup:       return Color{ 190, 70, 230, a };   // Purple
        case CollisionLayer::Trigger:      return Color{ 70, 140, 250, a };   // Soft Blue
        case CollisionLayer::Building:     return Color{ 170, 110, 60, a };   // Brown/Wood
        case CollisionLayer::Hazard:       return Color{ 230, 30, 120, a };   // Magenta
        case CollisionLayer::Interactable: return Color{ 240, 180, 50, a };   // Amber
        default:                           return Color{ 180, 180, 180, a }; // Gray
    }
}

Color CollisionDebugRenderer::GetTriggerColor(float alpha) {
    uint8_t a = static_cast<uint8_t>(alpha * 255.0f);
    return Color{ 60, 170, 255, a };
}

void CollisionDebugRenderer::Render(const CollisionManager& collisionMgr,
                                    const Rectangle& visibleArea,
                                    bool showPhysical,
                                    bool showTriggers,
                                    bool showOverlaps,
                                    bool showLabels,
                                    uint32_t layerFilterMask)
{
    const auto& colliders = collisionMgr.GetAllColliders();

    // 1. Draw Colliders
    for (const auto& [id, col] : colliders) {
        if (!col.IsEnabled()) {
            // Disabled collider: dim grey outline
            if (OverlapDetector::Intersects(visibleArea, col.GetWorldBounds())) {
                DrawRectangleLinesEx(col.GetWorldBounds(), 1.0f, Color{ 100, 100, 100, 120 });
            }
            continue;
        }

        uint32_t layerVal = static_cast<uint32_t>(col.GetLayer());
        if ((layerFilterMask & layerVal) == 0) continue;

        if (col.IsTrigger() && !showTriggers) continue;
        if (col.IsPhysical() && !showPhysical) continue;

        const Rectangle& bounds = col.GetWorldBounds();
        if (!OverlapDetector::Intersects(visibleArea, bounds)) continue;

        if (col.IsTrigger()) {
            // Trigger: translucent fill + distinct blue outline
            Color fill = GetTriggerColor(0.20f);
            Color outline = GetTriggerColor(0.85f);
            DrawRectangleRec(bounds, fill);
            DrawRectangleLinesEx(bounds, 1.5f, outline);
        } else {
            // Physical: layer-coded fill + strong border
            Color fill = GetLayerColor(col.GetLayer(), 0.25f);
            Color outline = GetLayerColor(col.GetLayer(), 0.90f);
            DrawRectangleRec(bounds, fill);
            DrawRectangleLinesEx(bounds, 1.0f, outline);
        }

        if (showLabels) {
            std::string label = col.GetTag() + " [" + std::to_string(col.GetId()) + "]";
            DrawText(label.c_str(), static_cast<int>(bounds.x + 2), static_cast<int>(bounds.y + 2), 8, WHITE);
        }
    }

    // 2. Draw Active Overlaps (Highlighted In White/Bright Amber)
    if (showOverlaps) {
        const auto& overlaps = collisionMgr.GetCurrentOverlaps();
        for (const auto& [pair, rect] : overlaps) {
            if (OverlapDetector::Intersects(visibleArea, rect)) {
                DrawRectangleRec(rect, Color{ 255, 255, 255, 120 });
                DrawRectangleLinesEx(rect, 2.0f, Color{ 255, 220, 50, 240 });
            }
        }
    }
}

} // namespace wildbound::physics

