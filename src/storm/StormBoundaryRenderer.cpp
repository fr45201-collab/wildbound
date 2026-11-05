#include "StormBoundaryRenderer.hpp"
#include "../graphics/Camera2D.hpp"
#include "../resources/AssetManager.hpp"
#include "../entities/Player.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::storm {

StormBoundaryRenderer::StormBoundaryRenderer() {
    m_particles.reserve(128);
}

void StormBoundaryRenderer::Reset() {
    m_animTime = 0.0f;
    m_particleSpawnTimer = 0.0f;
    m_particles.clear();
}

void StormBoundaryRenderer::Update(float dt, const SafeZone& currentZone, const entities::Player* player) {
    m_animTime += dt;

    if (!m_renderParticles) {
        m_particles.clear();
        return;
    }

    // Update existing particles
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            // Fade alpha based on remaining life
            float alphaRatio = it->life / it->maxLife;
            it->color.a = static_cast<unsigned char>(std::clamp(alphaRatio * 200.0f, 0.0f, 255.0f));
            ++it;
        }
    }

    // Spawn new particles around player if near or outside boundary
    if (player && m_particles.size() < 96) {
        m_particleSpawnTimer += dt;
        if (m_particleSpawnTimer >= 0.08f) {
            m_particleSpawnTimer = 0.0f;
            SpawnParticlesAroundPlayer(currentZone, player->GetPosition());
        }
    }
}

void StormBoundaryRenderer::SpawnParticlesAroundPlayer(const SafeZone& zone, Vector2 playerPos) {
    float dist = zone.DistanceTo(playerPos);
    // If player is outside zone or within 300px of the boundary
    if (dist >= zone.radius - 300.0f) {
        float angle = static_cast<float>(rand() % 360) * (3.14159265f / 180.0f);
        float spawnRadius = static_cast<float>(rand() % 280) + 20.0f;
        Vector2 spawnPos{
            playerPos.x + std::cos(angle) * spawnRadius,
            playerPos.y + std::sin(angle) * spawnRadius
        };

        // If outside the zone, spawn corruption motes
        if (!zone.Contains(spawnPos)) {
            StormParticle p;
            p.position = spawnPos;
            p.velocity = Vector2{
                static_cast<float>((rand() % 60) - 30),
                static_cast<float>((rand() % 60) - 45) // Drift upwards
            };
            p.maxLife = 1.0f + static_cast<float>(rand() % 100) / 100.0f;
            p.life = p.maxLife;
            p.size = 2.0f + static_cast<float>(rand() % 5);
            // Arcane purple/magenta glow
            p.color = Color{
                static_cast<unsigned char>(180 + rand() % 75),
                static_cast<unsigned char>(40 + rand() % 80),
                static_cast<unsigned char>(220 + rand() % 35),
                200
            };
            m_particles.push_back(p);
        }
    }
}

void StormBoundaryRenderer::RenderWorld(const SafeZone& currentZone, const SafeZone& targetZone,
                                        bool isShrinking, float visualIntensity,
                                        const graphics::Camera2DSystem& camera,
                                        resources::AssetManager* assetMgr) {
    // 1. Camera-culled Atmospheric Outside Overlay
    if (m_renderOverlay) {
        // Visible world bounds from camera
        Vector2 topLeft = camera.VirtualToWorld(Vector2{ 0.0f, 0.0f });
        Vector2 bottomRight = camera.VirtualToWorld(Vector2{ 640.0f, 360.0f });
        Rectangle visibleWorld{
            topLeft.x,
            topLeft.y,
            bottomRight.x - topLeft.x,
            bottomRight.y - topLeft.y
        };

        // If the visible world area overlaps the outside region, apply atmospheric tint
        // Sample 4 corners and center of visible area: if any is outside, render atmospheric haze
        bool cameraSeesStorm = !currentZone.Contains(Vector2{ visibleWorld.x, visibleWorld.y }) ||
                               !currentZone.Contains(Vector2{ visibleWorld.x + visibleWorld.width, visibleWorld.y }) ||
                               !currentZone.Contains(Vector2{ visibleWorld.x, visibleWorld.y + visibleWorld.height }) ||
                               !currentZone.Contains(Vector2{ visibleWorld.x + visibleWorld.width, visibleWorld.y + visibleWorld.height });

        if (cameraSeesStorm) {
            float pulse = 0.85f + 0.15f * std::sin(m_animTime * 2.5f);
            unsigned char alpha = static_cast<unsigned char>(std::clamp(visualIntensity * 65.0f * pulse, 20.0f, 130.0f));
            Color stormHazeColor{ 75, 15, 110, alpha };
            DrawRectangleRec(visibleWorld, stormHazeColor);
        }
    }

    // 2. Target Safe Zone (Dashed / Preview Ring if currently shrinking)
    if (isShrinking && m_renderBoundary && targetZone.radius > 0.0f) {
        // Subtle dotted or dashed target ring
        Color targetRingColor{ 240, 200, 60, 160 }; // Amber/yellow preview
        DrawCircleLines(static_cast<int>(targetZone.center.x), static_cast<int>(targetZone.center.y),
                        targetZone.radius, targetRingColor);
    }

    // 3. Current Safe Zone Boundary (Animated Multi-Ring Energy Wall)
    if (m_renderBoundary && currentZone.radius > 0.0f) {
        int cx = static_cast<int>(currentZone.center.x);
        int cy = static_cast<int>(currentZone.center.y);
        float r = currentZone.radius;

        float pulse = std::sin(m_animTime * 4.0f) * 2.0f;
        float shimmer = std::cos(m_animTime * 6.0f) * 1.5f;

        // Outer violet energy halo
        Color outerColor{ 180, 50, 240, 140 };
        DrawCircleLines(cx, cy, r + 4.0f + pulse, outerColor);
        DrawCircleLines(cx, cy, r + 2.0f + shimmer, outerColor);

        // Core cyan/arcane energy barrier
        Color coreColor{ 100, 240, 255, 230 };
        DrawCircleLines(cx, cy, r, coreColor);
        DrawCircleLines(cx, cy, std::max(0.0f, r - 1.0f), coreColor);

        // Inner soft glow
        Color innerColor{ 60, 160, 255, 160 };
        DrawCircleLines(cx, cy, std::max(0.0f, r - 3.0f + pulse), innerColor);
    }

    // 4. Render Floating Corruption Particles
    if (m_renderParticles) {
        for (const auto& p : m_particles) {
            DrawCircleV(p.position, p.size, p.color);
        }
    }
}

void StormBoundaryRenderer::RenderScreenEffects(const SafeZone& currentZone, const entities::Player* player,
                                                const graphics::Camera2DSystem& camera, float visualIntensity) {
    if (!player) return;

    Vector2 playerPos = player->GetPosition();

    // If player is outside safe zone, render screen-edge hazard vignette & directional pointer
    if (!currentZone.Contains(playerPos)) {
        float pulse = 0.5f + 0.5f * std::sin(m_animTime * 5.0f);
        unsigned char vignetteAlpha = static_cast<unsigned char>(std::clamp(visualIntensity * 90.0f * (0.7f + 0.3f * pulse), 30.0f, 150.0f));

        // Screen vignette border (4 screen edges in virtual resolution 640x360)
        Color vignetteColor{ 140, 10, 40, vignetteAlpha };
        DrawRectangle(0, 0, 640, 12, vignetteColor);
        DrawRectangle(0, 348, 640, 12, vignetteColor);
        DrawRectangle(0, 12, 12, 336, vignetteColor);
        DrawRectangle(628, 12, 12, 336, vignetteColor);

        // Directional vector pointing toward nearest safe point
        Vector2 safePoint = currentZone.GetClosestSafePoint(playerPos);
        float dx = safePoint.x - playerPos.x;
        float dy = safePoint.y - playerPos.y;
        float len = std::sqrt(dx * dx + dy * dy);

        if (len > 0.001f) {
            float ndx = dx / len;
            float ndy = dy / len;

            // Center of screen
            Vector2 screenCenter{ 320.0f, 180.0f };
            // Position arrow 50px away from center in direction of safe zone
            Vector2 arrowPos{ screenCenter.x + ndx * 50.0f, screenCenter.y + ndy * 50.0f };

            // Render arrow pointing toward safe zone
            Vector2 tip = Vector2{ arrowPos.x + ndx * 12.0f, arrowPos.y + ndy * 12.0f };
            Vector2 left = Vector2{ arrowPos.x - ndy * 8.0f - ndx * 6.0f, arrowPos.y + ndx * 8.0f - ndy * 6.0f };
            Vector2 right = Vector2{ arrowPos.x + ndy * 8.0f - ndx * 6.0f, arrowPos.y - ndx * 8.0f - ndy * 6.0f };

            Color arrowColor{ 80, 240, 180, static_cast<unsigned char>(180 + 75 * pulse) };
            DrawTriangle(tip, left, right, arrowColor);
        }
    }
}

void StormBoundaryRenderer::RenderMinimap(const SafeZone& currentZone, const SafeZone& targetZone,
                                          bool isShrinking, Vector2 minimapCenter, float minimapRadius,
                                          Vector2 playerPos, float zoom, Rectangle worldBounds) {
    if (minimapRadius <= 0.0f || worldBounds.width <= 0.0f) return;

    // Minimap world scale factor:
    // In Minimap.cpp, the view diameter corresponds to a fraction of the world modulated by zoom:
    // float viewWorldRadius = (worldBounds.width * 0.25f) / zoom;
    // float scale = minimapRadius / viewWorldRadius;
    float viewWorldRadius = (worldBounds.width * 0.25f) / std::max(0.1f, zoom);
    float scale = minimapRadius / viewWorldRadius;

    // 1. Current Safe Zone Circle on Minimap
    Vector2 offset{
        (currentZone.center.x - playerPos.x) * scale,
        (currentZone.center.y - playerPos.y) * scale
    };
    Vector2 miniCircleCenter{ minimapCenter.x + offset.x, minimapCenter.y + offset.y };
    float miniCircleRadius = currentZone.radius * scale;

    // Draw safe zone boundary on minimap
    Color safeRingColor{ 80, 220, 255, 230 };
    DrawCircleLines(static_cast<int>(miniCircleCenter.x), static_cast<int>(miniCircleCenter.y),
                    miniCircleRadius, safeRingColor);

    // 2. Target Safe Zone on Minimap (if shrinking)
    if (isShrinking && targetZone.radius > 0.0f) {
        Vector2 targetOffset{
            (targetZone.center.x - playerPos.x) * scale,
            (targetZone.center.y - playerPos.y) * scale
        };
        Vector2 miniTargetCenter{ minimapCenter.x + targetOffset.x, minimapCenter.y + targetOffset.y };
        float miniTargetRadius = targetZone.radius * scale;

        Color targetRingColor{ 255, 210, 60, 190 }; // Amber/yellow dashed preview
        DrawCircleLines(static_cast<int>(miniTargetCenter.x), static_cast<int>(miniTargetCenter.y),
                        miniTargetRadius, targetRingColor);
    }
}

} // namespace wildbound::storm

