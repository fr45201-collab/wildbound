#include "Minimap.hpp"
#include "../../world/World.hpp"
#include "../../world/WorldItem.hpp"
#include "../../world/Tilemap.hpp"
#include "../../entities/Player.hpp"
#include <cmath>

namespace wildbound::ui {

Minimap::Minimap()
    : m_bounds{ 520.0f, 10.0f, 108.0f, 108.0f }
    , m_visible(true)
    , m_zoom(1.0f)
    , m_zoomIndex(0)
    , m_blipPulseTimer(0.0f) {
}

void Minimap::SetZoom(float zoom) {
    if (zoom <= 1.05f) {
        m_zoom = 1.0f;
        m_zoomIndex = 0;
    } else if (zoom <= 1.6f) {
        m_zoom = 1.5f;
        m_zoomIndex = 1;
    } else {
        m_zoom = 2.0f;
        m_zoomIndex = 2;
    }
}

void Minimap::CycleZoom() {
    m_zoomIndex = (m_zoomIndex + 1) % 3;
    if (m_zoomIndex == 0) m_zoom = 1.0f;
    else if (m_zoomIndex == 1) m_zoom = 1.5f;
    else m_zoom = 2.0f;
}

Vector2 Minimap::WorldToMinimap(Vector2 worldPos, Vector2 playerPos, Rectangle worldBounds) const {
    const float centerX = m_bounds.x + m_bounds.width * 0.5f;
    const float centerY = m_bounds.y + m_bounds.height * 0.5f;

    // Base view radius in world coordinates: 1000px at 1.0x zoom
    const float viewRadiusWorld = 1000.0f / m_zoom;
    const float mapRadiusPixels = (m_bounds.width * 0.5f) - 6.0f;

    const float deltaX = worldPos.x - playerPos.x;
    const float deltaY = worldPos.y - playerPos.y;

    const float minimapX = centerX + (deltaX / viewRadiusWorld) * mapRadiusPixels;
    const float minimapY = centerY + (deltaY / viewRadiusWorld) * mapRadiusPixels;

    return Vector2{ minimapX, minimapY };
}

bool Minimap::IsWorldPosInView(Vector2 worldPos, Vector2 playerPos, Rectangle worldBounds) const {
    Vector2 mmPos = WorldToMinimap(worldPos, playerPos, worldBounds);
    const float centerX = m_bounds.x + m_bounds.width * 0.5f;
    const float centerY = m_bounds.y + m_bounds.height * 0.5f;
    const float radius = (m_bounds.width * 0.5f) - 6.0f;

    float dx = mmPos.x - centerX;
    float dy = mmPos.y - centerY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void Minimap::Update(float dt) {
    m_blipPulseTimer += dt * 3.0f;
    if (m_blipPulseTimer > 2.0f * 3.14159f) {
        m_blipPulseTimer -= 2.0f * 3.14159f;
    }
}

void Minimap::Render(const world::World* world, const entities::Player* player) {
    if (!m_visible) return;

    const float centerX = m_bounds.x + m_bounds.width * 0.5f;
    const float centerY = m_bounds.y + m_bounds.height * 0.5f;
    const float radius = m_bounds.width * 0.5f;

    // 1. Radar Circular Background
    DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius, UITheme::DarkBgOpaque);
    DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius - 3.0f, Color{ 18, 24, 34, 255 });

    // Range rings
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), (radius - 6.0f) * 0.5f, Color{ 35, 48, 68, 160 });
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), radius - 6.0f, Color{ 45, 60, 85, 200 });

    // Crosshairs
    DrawLine(static_cast<int>(centerX - (radius - 6.0f)), static_cast<int>(centerY),
             static_cast<int>(centerX + (radius - 6.0f)), static_cast<int>(centerY), Color{ 35, 48, 68, 140 });
    DrawLine(static_cast<int>(centerX), static_cast<int>(centerY - (radius - 6.0f)),
             static_cast<int>(centerX), static_cast<int>(centerY + (radius - 6.0f)), Color{ 35, 48, 68, 140 });

    Vector2 playerPos = player ? player->GetPosition() : Vector2{ 0.0f, 0.0f };
    Rectangle worldBounds = world ? world->GetWorldBounds() : Rectangle{ 0.0f, 0.0f, 3200.0f, 3200.0f };
    if (worldBounds.width <= 0.0f) worldBounds.width = 3200.0f;
    if (worldBounds.height <= 0.0f) worldBounds.height = 3200.0f;

    // 2. Render World Bounds on Radar
    Vector2 topLeftMM = WorldToMinimap({ worldBounds.x, worldBounds.y }, playerPos, worldBounds);
    Vector2 botRightMM = WorldToMinimap({ worldBounds.x + worldBounds.width, worldBounds.y + worldBounds.height }, playerPos, worldBounds);

    // Draw world boundaries lines if within rough radar area
    DrawRectangleLines(static_cast<int>(topLeftMM.x), static_cast<int>(topLeftMM.y),
                       static_cast<int>(botRightMM.x - topLeftMM.x),
                       static_cast<int>(botRightMM.y - topLeftMM.y),
                       Color{ 180, 50, 50, 100 });

    // 3. Render World Items (Ground Loot) Blips
    if (world) {
        const auto& items = world->GetWorldItems();
        for (const auto& item : items) {
            if (!item || item->IsCollected()) continue;
            Vector2 itemWorldPos = item->GetPosition();
            if (IsWorldPosInView(itemWorldPos, playerPos, worldBounds)) {
                Vector2 mmPos = WorldToMinimap(itemWorldPos, playerPos, worldBounds);
                DrawCircle(static_cast<int>(mmPos.x), static_cast<int>(mmPos.y), 2.0f, UITheme::Gold);
            }
        }
    }

    // 4. Render Player Marker (Center)
    // Pulsing halo
    float pulse = 0.5f + 0.5f * std::sin(m_blipPulseTimer);
    DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), 4.0f + pulse * 1.5f, Color{ 70, 180, 255, static_cast<unsigned char>(100 + pulse * 100) });
    DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), 3.0f, UITheme::TextLight);

    // Direction arrow
    if (player) {
        Vector2 facing = player->GetFacingDirection();
        float angle = std::atan2(facing.y, facing.x);
        Vector2 arrowTip = { centerX + std::cos(angle) * 9.0f, centerY + std::sin(angle) * 9.0f };
        Vector2 arrowLeft = { centerX + std::cos(angle + 2.5f) * 6.0f, centerY + std::sin(angle + 2.5f) * 6.0f };
        Vector2 arrowRight = { centerX + std::cos(angle - 2.5f) * 6.0f, centerY + std::sin(angle - 2.5f) * 6.0f };
        DrawTriangle(arrowTip, arrowLeft, arrowRight, UITheme::Gold);
    }

    // 5. Outer Beveled Radar Ring
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), radius, UITheme::BorderGoldDim);
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), radius - 1.0f, UITheme::BorderNormal);
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), radius - 2.0f, UITheme::BorderDark);

    // Cardinal directions
    DrawText("N", static_cast<int>(centerX - 3), static_cast<int>(centerY - radius + 4), UITheme::FontMicro, UITheme::Gold);
    DrawText("S", static_cast<int>(centerX - 3), static_cast<int>(centerY + radius - 10), UITheme::FontMicro, UITheme::TextMuted);
    DrawText("W", static_cast<int>(centerX - radius + 5), static_cast<int>(centerY - 4), UITheme::FontMicro, UITheme::TextMuted);
    DrawText("E", static_cast<int>(centerX + radius - 10), static_cast<int>(centerY - 4), UITheme::FontMicro, UITheme::TextMuted);

    // Zoom badge pill
    std::string zoomText = (m_zoomIndex == 0) ? "1.0x" : (m_zoomIndex == 1 ? "1.5x" : "2.0x");
    DrawRectangleRounded({ centerX - 14.0f, centerY + radius - 6.0f, 28.0f, 11.0f }, 0.5f, 4, UITheme::DarkBgOpaque);
    DrawRectangleRoundedLines({ centerX - 14.0f, centerY + radius - 6.0f, 28.0f, 11.0f }, 0.5f, 4, Color{ 70, 85, 110, 255 });
    int zw = MeasureText(zoomText.c_str(), UITheme::FontMicro);
    DrawText(zoomText.c_str(), static_cast<int>(centerX - zw * 0.5f), static_cast<int>(centerY + radius - 4.0f), UITheme::FontMicro, UITheme::Gold);
}

} // namespace wildbound::ui
