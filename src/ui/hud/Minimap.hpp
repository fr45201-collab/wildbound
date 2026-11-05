#pragma once

#include "../UITheme.hpp"
#include <raylib.h>
#include <vector>
#include <memory>
#include <string>

namespace wildbound::world {
    class World;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::ui {

/**
 * Minimap Subsystem for Wildbound.
 * Renders a circular/rectangular radar HUD element with world-to-minimap coordinate
 * translation, player position & orientation indicator, terrain landmarks, and zoom switching.
 */
class Minimap {
public:
    Minimap();
    ~Minimap() = default;

    void SetBounds(Rectangle bounds) { m_bounds = bounds; }
    [[nodiscard]] Rectangle GetBounds() const { return m_bounds; }

    void SetVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool IsVisible() const { return m_visible; }
    void ToggleVisible() { m_visible = !m_visible; }

    void SetZoom(float zoom);
    [[nodiscard]] float GetZoom() const { return m_zoom; }
    void CycleZoom();

    // Coordinate Transformations
    [[nodiscard]] Vector2 WorldToMinimap(Vector2 worldPos, Vector2 playerPos, Rectangle worldBounds) const;
    [[nodiscard]] bool IsWorldPosInView(Vector2 worldPos, Vector2 playerPos, Rectangle worldBounds) const;

    void Update(float dt);
    void Render(const world::World* world, const entities::Player* player);

private:
    Rectangle m_bounds{ 520.0f, 10.0f, 108.0f, 108.0f };
    bool m_visible = true;
    float m_zoom = 1.0f; // 1.0f, 1.5f, 2.0f
    int m_zoomIndex = 0;

    // Pulse animation for player blip
    float m_blipPulseTimer = 0.0f;
};

} // namespace wildbound::ui

