#pragma once

#include "StormTypes.hpp"
#include <raylib.h>
#include <vector>

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::storm {

struct StormParticle {
    Vector2 position{ 0.0f, 0.0f };
    Vector2 velocity{ 0.0f, 0.0f };
    float life = 1.0f;
    float maxLife = 1.0f;
    float size = 4.0f;
    Color color{ 200, 100, 255, 200 };
};

/**
 * World & Viewport Renderer for the Corruption Storm.
 * Manages camera-culled atmospheric haze outside the safe zone, pulsating animated energy boundaries,
 * perimeter particle emitters, and directional indicators for trapped players.
 */
class StormBoundaryRenderer {
public:
    StormBoundaryRenderer();
    ~StormBoundaryRenderer() = default;

    void Reset();
    void Update(float dt, const SafeZone& currentZone, const entities::Player* player = nullptr);

    /**
     * Renders in-world storm visual effects (must be called inside BeginMode2D).
     */
    void RenderWorld(const SafeZone& currentZone, const SafeZone& targetZone, bool isShrinking,
                     float visualIntensity, const graphics::Camera2DSystem& camera,
                     resources::AssetManager* assetMgr = nullptr);

    /**
     * Renders HUD-space screen effects and directional indicator (must be called in screen space).
     */
    void RenderScreenEffects(const SafeZone& currentZone, const entities::Player* player,
                             const graphics::Camera2DSystem& camera, float visualIntensity);

    /**
     * Renders safe zone circle and target boundary directly onto the minimap radar.
     */
    void RenderMinimap(const SafeZone& currentZone, const SafeZone& targetZone, bool isShrinking,
                       Vector2 minimapCenter, float minimapRadius, Vector2 playerPos, float zoom,
                       Rectangle worldBounds);

    // Toggles
    void SetRenderBoundary(bool enable) noexcept { m_renderBoundary = enable; }
    [[nodiscard]] bool GetRenderBoundary() const noexcept { return m_renderBoundary; }

    void SetRenderOverlay(bool enable) noexcept { m_renderOverlay = enable; }
    [[nodiscard]] bool GetRenderOverlay() const noexcept { return m_renderOverlay; }

    void SetRenderParticles(bool enable) noexcept { m_renderParticles = enable; }
    [[nodiscard]] bool GetRenderParticles() const noexcept { return m_renderParticles; }

private:
    void SpawnParticlesAroundPlayer(const SafeZone& zone, Vector2 playerPos);

    bool m_renderBoundary = true;
    bool m_renderOverlay = true;
    bool m_renderParticles = true;

    float m_animTime = 0.0f;
    float m_particleSpawnTimer = 0.0f;
    std::vector<StormParticle> m_particles;
};

} // namespace wildbound::storm

