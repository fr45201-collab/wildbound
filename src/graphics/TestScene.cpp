#include "TestScene.hpp"
#include "ColorPalette.hpp"
#include "Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include <cmath>
#include <string>

namespace wildbound::graphics {

TestScene::TestScene() {
    m_targetPos = Vector2{ 800.0f, 600.0f };
}

void TestScene::Update(float dt) {
    m_animTime += dt;

    float radiusX = 280.0f;
    float radiusY = 160.0f;
    float speed = 0.6f;
    m_targetPos.x = 800.0f + std::cos(m_animTime * speed) * radiusX;
    m_targetPos.y = 600.0f + std::sin(m_animTime * speed * 1.5f) * radiusY;
}

void TestScene::RenderWorld(Renderer& renderer, const Camera2DSystem& /*camera*/, const DrawContext& /*ctx*/, resources::AssetManager* assetMgr) {
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

    // If AssetManager is present, draw item icons on the shrine
    if (assetMgr && assetMgr->HasTexture("tex_item_icons")) {
        Texture2D iconTex = assetMgr->GetTexture("tex_item_icons");
        const resources::SpriteSheet* iconSheet = assetMgr->GetSpriteSheet("sheet_item_icons");
        for (int i = 0; i < 4; ++i) {
            Rectangle srcRect = iconSheet ? iconSheet->GetFrameRect(i) : Rectangle{ i * 16.0f, 0.0f, 16.0f, 16.0f };
            renderer.DrawSprite(iconTex, srcRect, Vector2{ 752.0f + i * 24.0f, 588.0f }, Vector2{ 0.0f, 0.0f }, 1.0f);
        }
    }

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

    // Forest Grove (South-West) with Real Oak Tree Sprites if available!
    bool drewSpriteTrees = false;
    if (assetMgr && assetMgr->HasTexture("tex_oak_tree")) {
        Texture2D treeTex = assetMgr->GetTexture("tex_oak_tree");
        if (treeTex.id != 0) {
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 4; ++col) {
                    float treeX = 280.0f + col * 75.0f + (row % 2) * 35.0f;
                    float treeY = 820.0f + row * 70.0f;
                    renderer.DrawSprite(treeTex, Rectangle{ 0.0f, 0.0f, 48.0f, 64.0f },
                                       Vector2{ treeX, treeY }, Vector2{ 24.0f, 64.0f }, 1.0f);
                }
            }
            drewSpriteTrees = true;
        }
    }

    if (!drewSpriteTrees) {
        // Fallback procedural trees
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                float treeX = 300.0f + col * 70.0f + (row % 2) * 30.0f;
                float treeY = 850.0f + row * 65.0f;
                DrawRectangle(static_cast<int>(treeX) - 4, static_cast<int>(treeY), 8, 20, Palette::BarkBrown);
                DrawCircle(static_cast<int>(treeX), static_cast<int>(treeY) - 10, 22.0f, Palette::DeepMoss);
                DrawCircle(static_cast<int>(treeX), static_cast<int>(treeY) - 14, 16.0f, Palette::ForestGrass);
            }
        }
    }
    DrawText("WHISPERING GROVE", 320, 790, 12, Palette::CanopyGreen);

    // 5. World Coordinate Indicators
    DrawCircle(0, 0, 8.0f, Palette::CrystalRuby);
    DrawText("(0, 0) WORLD ORIGIN", 12, 12, 12, Palette::CrystalRuby);

    DrawCircle(1600, 0, 8.0f, Palette::CrystalRuby);
    DrawText("(1600, 0) NE CORNER", 1450, 12, 12, Palette::CrystalRuby);

    DrawCircle(0, 1200, 8.0f, Palette::CrystalRuby);
    DrawText("(0, 1200) SW CORNER", 12, 1180, 12, Palette::CrystalRuby);

    DrawCircle(1600, 1200, 8.0f, Palette::CrystalRuby);
    DrawText("(1600, 1200) SE CORNER", 1440, 1180, 12, Palette::CrystalRuby);

    // 6. Camera Follow Target: Draw Animated Sprite if available, or procedural ring
    bool drewSpritePlayer = false;
    if (assetMgr && assetMgr->HasTexture("tex_player")) {
        Texture2D playerTex = assetMgr->GetTexture("tex_player");
        const resources::SpriteSheet* sheet = assetMgr->GetSpriteSheet("sheet_player");
        const resources::AnimationClip* clip = assetMgr->GetAnimation("player_walk_down");

        if (playerTex.id != 0) {
            int frameIdx = clip ? clip->GetFrameIndexAtTime(m_animTime) : (static_cast<int>(m_animTime * 6.0f) % 4);
            Rectangle srcRect = sheet ? sheet->GetFrameRect(frameIdx) : Rectangle{ frameIdx * 32.0f, 0.0f, 32.0f, 32.0f };
            Vector2 origin = sheet ? sheet->GetFrameOrigin(frameIdx) : Vector2{ 16.0f, 32.0f };

            // Draw shadow beneath character
            DrawEllipse(static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y), 10.0f, 4.0f, Color{ 0, 0, 0, 100 });

            // Render animated sprite
            renderer.DrawSprite(playerTex, srcRect, m_targetPos, origin, 1.0f);
            drewSpritePlayer = true;
        }
    }

    if (!drewSpritePlayer) {
        float pulse = (std::sin(m_animTime * 4.0f) + 1.0f) * 0.5f;
        float outerRadius = 16.0f + pulse * 6.0f;
        DrawCircleLines(static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y), outerRadius, Palette::MysticGlow);
        DrawCircle(static_cast<int>(m_targetPos.x), static_cast<int>(m_targetPos.y), 6.0f, Palette::AmberTorch);
    }

    std::string targetLabel = "TARGET (" + std::to_string(static_cast<int>(m_targetPos.x)) +
                              "," + std::to_string(static_cast<int>(m_targetPos.y)) + ")";
    DrawText(targetLabel.c_str(), static_cast<int>(m_targetPos.x) - 30, static_cast<int>(m_targetPos.y) - 36, 10, Palette::AmberTorch);
}

void TestScene::RenderUI(const DrawContext& ctx, const Camera2DSystem& camera, const Viewport& /*viewport*/, resources::AssetManager* assetMgr) {
    DrawRectangleLines(1, 1, ctx.virtualWidth - 2, ctx.virtualHeight - 2, Color{ 60, 80, 110, 120 });

    DrawRectangle(8, 8, 290, 56, Color{ 14, 18, 26, 210 });
    DrawRectangleLines(8, 8, 290, 56, Color{ 45, 60, 85, 255 });

    std::string modeStr = (camera.GetMode() == CameraMode::Follow) ? "FOLLOW [Target]" : "FREE [WASD]";
    DrawText("CAMERA MODE:", 14, 14, 10, Color{ 160, 180, 205, 255 });
    DrawText(modeStr.c_str(), 105, 14, 10, (camera.GetMode() == CameraMode::Follow) ? Palette::AncientGold : Palette::StreamCyan);

    Vector2 camPos = camera.GetTarget();
    std::string posStr = "Cam: (" + std::to_string(static_cast<int>(camPos.x)) + ", " +
                         std::to_string(static_cast<int>(camPos.y)) + ")  Zoom: " +
                         std::to_string(camera.GetZoom()).substr(0, 4) + "x";
    DrawText(posStr.c_str(), 14, 28, 10, Palette::PureWhite);

    if (assetMgr) {
        std::string assetStr = "Assets: " + std::to_string(assetMgr->GetRegistry().GetAssetCount()) +
                               " registered | Textures: " + std::to_string(assetMgr->GetTextureManager().GetLoadedTextureCount());
        DrawText(assetStr.c_str(), 14, 40, 9, Palette::CanopyGreen);
    }

    DrawText("F2: Toggle Follow | F1: Diagnostics & Asset Browser | WASD: Pan", 14, 52, 8, Color{ 130, 150, 175, 255 });
}

} // namespace wildbound::graphics
