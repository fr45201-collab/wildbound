#pragma once

#include "Camera2D.hpp"
#include "DrawContext.hpp"
#include <raylib.h>

namespace wildbound::graphics {

/**
 * Temporary verification test scene for Phase 1.
 * Provides world boundaries, grids, geometric shapes, animated follow target,
 * and coordinate indicators.
 */
class TestScene {
public:
    TestScene();

    void Update(float dt);
    void RenderWorld(const Camera2DSystem& camera, const DrawContext& ctx);
    void RenderUI(const DrawContext& ctx, const Camera2DSystem& camera, const Viewport& viewport);

    Rectangle GetWorldBounds() const { return m_worldBounds; }
    Vector2 GetTargetPos() const { return m_targetPos; }
    const Vector2* GetTargetPosPtr() const { return &m_targetPos; }

    void SetTargetPos(Vector2 pos) { m_targetPos = pos; }

private:
    Rectangle m_worldBounds{ 0.0f, 0.0f, 1600.0f, 1200.0f };
    Vector2 m_targetPos{ 800.0f, 600.0f };
    float m_animTime = 0.0f;
};

} // namespace wildbound::graphics

