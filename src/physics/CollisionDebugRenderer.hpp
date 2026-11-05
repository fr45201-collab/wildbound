#pragma once

#include "CollisionManager.hpp"
#include <raylib.h>

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::physics {

/**
 * Diagnostic visualization engine for collision bounds, trigger volumes,
 * active overlaps, and layer classifications.
 */
class CollisionDebugRenderer {
public:
    CollisionDebugRenderer() = default;

    void Render(const CollisionManager& collisionMgr,
                const Rectangle& visibleArea,
                bool showPhysical = true,
                bool showTriggers = true,
                bool showOverlaps = true,
                bool showLabels = false,
                uint32_t layerFilterMask = 0xFFFFFFFFu);

    // Color definitions per layer
    static Color GetLayerColor(CollisionLayer layer, float alpha = 1.0f);
    static Color GetTriggerColor(float alpha = 1.0f);
};

} // namespace wildbound::physics

