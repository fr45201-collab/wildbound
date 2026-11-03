#include "TestScene.hpp"
#include "ColorPalette.hpp"
#include <cmath>
#include <string>

namespace wildbound::graphics {

TestScene::TestScene() {
    m_targetPos = Vector2{ 800.0f, 600.0f };
}

void TestScene::Update(float dt) {
    m_animTime += dt;

    // Follow target wanders smoothly around world center (800, 600)
    float radiusX = 280.0f;
    float radiusY = 160.0f;
    float speed = 0.6f;
    m_targetPos.x = 800.0f + std::cos(m_animTime * speed) * radiusX;
    m_targetPos.y = 600.0f + std::sin(m_animTime * speed * 1.5f) * radiusY;
}

void TestScene::RenderWorld(const Camera2DSystem& /*camera*/, const DrawContext& /*ctx*/) {
    // 0. World Background
    ClearBackground(Palette::MidnightSky);

    // 1. World Terrain Base
    DrawRectangleRec(m_worldBounds, Color{ 22, 28, 40, 255 });

    // 2. Visible World Grid (32x32 minor, 128x128 major)
    for (float x = m_worldBounds.x; x <= m_worldBounds.x + m_worldBounds.width; x += 32.0f) {
        bool isMajor = (static_cast<int>(x) % 128 == 0);
        Color col = isMajor ? Color{ 48, 62, 88, 200 } : Color{ 32, 42, 60, 150 };
        DrawLineV(Vector2{ x, m_worldBounds.y }, Vector2{ x, m_worldBounds.y + m_worldBounds.height }, col);
    }
    for (float y = m_worldBounds.y; y <= m_worldBounds.y + m_worldBounds.height; y += 32.0f) {
        bool isMajor = (static_cast<int>(y) % 128 == 0);
        Color col = isMajor ? Color{ 48, 62, 88, 200 } : Color{ 32, 42, 60, 150 };
        DrawLineV(Vector2{ m_worldBounds.x, y }, Vector2{ m_worldBounds.x + m_worldBounds.width, y }, col);
    }

    // 3. World Boundary Frame
    DrawRectangleLinesEx(m_worldBounds, 4.0f, Palette::DebugBounds);

    // 4. Primitive Shapes & Environment Landmarks
    // Ancient Central Monolith / Shrine
    DrawRectangle(740, 540, 120, 120, Palette::SoilDark);
    DrawRectangle(750, 550, 100, 100, Palette::TimberWarm);
    DrawRectangle(770, 570, 60, 60, Palette::AncientGold);
    DrawText("SHRINE (800, 600)", 748, 670, 10, Palette::AncientGold);

    // Water Pond (East)
    DrawCircle(1150, 450, 90.0f, Palette::DeepOcean);
    DrawCircle(1150, 450, 70.0f, Palette::RiverBlue);
    DrawCircle(1150, 450, 45.0f, Palette::StreamCyan);

    // Stone Ruins (North-West)
    for (int i = 0; i < 5; ++i) {
        DrawRectangle(250 + i * 50, 260, 32, 64, Palette::DarkSlate);
        DrawRectangle(252 + i * 50, 262, 28, 60, Palette::SlateGrid);
    }
    DrawText("ANCIENT RUINS", 280, 240, 12, Palette::PureWhite);

    // Forest Grove (South-West)
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            float treeX = 300.0f + col * 70.0f + (row % 2) * 30.0f;
            float treeY = 850.0f + row * 65.0f;
            DrawRectangle(static_cast<int>(treeX) - 4, static_cast<int>(treeY), 8, 20, Palette::BarkBrown);
            DrawCircle(static_cast<int>(treeX), static_cast<int>(treeY) - 10, 22.0f, Palette::DeepMoss);
            DrawCircle(static_cast<int>(treeX), static_cast<int>(treeY) - 14, 16.0f, Palette::ForestGrass);
        }
    }
    DrawText("WHISPERING GROVE", 320, 810, 12, Palette::CanopyGreen);

    // 5. World Coordinate Indicators
    // Origin (0, 0)
    DrawCircle(0, 0, 8.0f, Palette::CrystalRuby);
    DrawText("(0, 0) WORLD ORIGIN", 12, 12, 12, Palette::CrystalRuby);

    // Corner Indicators
    DrawCircle(1600, 0, 8.0f, Palette::CrystalRuby);
    DrawText("(1600, 0) NE CORNER", 1450, 12, 12, Palette::CrystalRuby);

    DrawCircle(0, 1200, 8.0f, Palette::CrystalRuby);
    DrawText("(0, 1200) SW CORNER", 12, 1180, 12, Palette::CrystalRuby);

    DrawCircle(1600, 1200, 8.0f, Palette::CrystalRuby);
    DrawText("(1600, 1200) SE CORNER", 1440, 1180, 12, Palette::CrystalRuby);

    // 6. Camera Follow Target Marker
    float pulse = (std::sin(m_animTime * 4.0f) + 1.0f) * 0.5f;
    float outerRadius = 16.0f + pulse * 6.0f;
    DrawCircleLines(static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y), outerRadius, Palette::MysticGlow);
    DrawCircle(static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y), 6.0f, Palette::AmberTorch);

    // Target crosshair
    DrawLine(static_cast<int>(m_targetPos.x) - 12, static_cast<int>(m_targetPos.y),
             static_cast<int>(m_targetPos.x) + 12, static_cast<int>(m_targetPos.y), Palette::AmberTorch);
    DrawLine(static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y) - 12,
             static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y) + 12, Palette::AmberTorch);

    std::string targetLabel = "TARGET (" + std::to_string(static_cast<int>(m_targetPos.x)) +
                              "," + std::to_string(static_cast<int>(m_targetPos.y)) + ")";
    DrawText(targetLabel.c_str(), static_cast<int>(m_targetPos.x) - 30, static_cast<int>(m_targetPos.y) - 28, 10, Palette::AmberTorch);
}

void TestScene::RenderUI(const DrawContext& ctx, const Camera2DSystem& camera, const Viewport& /*viewport*/) {
    // Virtual Canvas Boundary Frame (640x360)
    DrawRectangleLines(1, 1, ctx.virtualWidth - 2, ctx.virtualHeight - 2, Color{ 60, 80, 110, 120 });

    // In-Canvas HUD (Virtual Coordinates)
    DrawRectangle(8, 8, 260, 48, Color{ 14, 18, 26, 210 });
    DrawRectangleLines(8, 8, 260, 48, Color{ 45, 60, 85, 255 });

    std::string modeStr = (camera.GetMode() == CameraMode::Follow) ? "FOLLOW [Target]" : "FREE [WASD]";
    DrawText("CAMERA MODE:", 14, 14, 10, Color{ 160, 180, 205, 255 });
    DrawText(modeStr.c_str(), 105, 14, 10, (camera.GetMode() == CameraMode::Follow) ? Palette::AncientGold : Palette::StreamCyan);

    Vector2 camPos = camera.GetTarget();
    std::string posStr = "Cam: (" + std::to_string(static_cast<int>(camPos.x)) + ", " +
                         std::to_string(static_cast<int>(camPos.y)) + ")  Zoom: " +
                         std::to_string(camera.GetZoom()).substr(0, 4) + "x";
    DrawText(posStr.c_str(), 14, 28, 10, Palette::PureWhite);

    DrawText("F2: Toggle Follow | WASD: Pan | Q/E: Zoom | R: Reset", 14, 42, 9, Color{ 130, 150, 175, 255 });
}

} // namespace wildbound::graphics

