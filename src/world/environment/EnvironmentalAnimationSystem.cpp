#include "EnvironmentalAnimationSystem.hpp"
#include "../../resources/AssetManager.hpp"
#include "../../graphics/Renderer.hpp"
#include "../../graphics/Camera2D.hpp"
#include "../../graphics/Viewport.hpp"
#include "../../world/World.hpp"
#include "../../core/Logger.hpp"

#include <cmath>
#include <random>

namespace wildbound::world::environment {

EnvironmentalAnimationSystem::EnvironmentalAnimationSystem() = default;
EnvironmentalAnimationSystem::~EnvironmentalAnimationSystem() = default;

bool EnvironmentalAnimationSystem::Initialize(resources::AssetManager& assetMgr) {
    core::Logger::Info("ENV_ANIM", "Initializing Environmental Animation System...");

    // 1. Preload Animation & Spritesheet Metadata for Category A & C & D assets
    const std::vector<std::string> sheetFiles = {
        // Water
        "environment/animations/water/water_pond_anim_spritesheet.json",
        "environment/animations/water/water_river_anim_spritesheet.json",
        "environment/animations/water/water_lake_anim_spritesheet.json",
        "environment/animations/water/water_flow_anim_spritesheet.json",
        "environment/animations/water/water_ripple_anim_spritesheet.json",
        // Waterfalls
        "environment/animations/waterfalls/waterfall_small_anim_spritesheet.json",
        "environment/animations/waterfalls/waterfall_medium_anim_spritesheet.json",
        "environment/animations/waterfalls/waterfall_large_anim_spritesheet.json",
        // Fire
        "environment/animations/fire/fire_small_anim_spritesheet.json",
        "environment/animations/fire/fire_medium_anim_spritesheet.json",
        "environment/animations/fire/fire_large_anim_spritesheet.json",
        // Grass
        "environment/animations/grass/grass_small_wind_anim_spritesheet.json",
        "environment/animations/grass/grass_tall_wind_anim_spritesheet.json",
        "environment/animations/grass/grass_dense_wind_anim_spritesheet.json",
        "environment/animations/grass/grass_flowering_wind_anim_spritesheet.json",
        // Trees
        "environment/animations/trees/tree_oak_breeze_anim_spritesheet.json",
        "environment/animations/trees/tree_pine_breeze_anim_spritesheet.json",
        "environment/animations/trees/bush_rustle_anim_spritesheet.json",
        "environment/animations/trees/leaves_falling_anim_spritesheet.json",
        // Smoke
        "environment/atmosphere/smoke/smoke_chimney_anim_spritesheet.json",
        "environment/atmosphere/smoke/smoke_campfire_anim_spritesheet.json",
        "environment/atmosphere/smoke/smoke_plume_anim_spritesheet.json",
        // Airplane
        "environment/events/airplanes/airplane_recon_001_spritesheet.json"
    };

    const std::vector<std::string> animFiles = {
        // Water
        "environment/animations/water/water_pond_anim_animations.json",
        "environment/animations/water/water_river_anim_animations.json",
        "environment/animations/water/water_lake_anim_animations.json",
        "environment/animations/water/water_flow_anim_animations.json",
        "environment/animations/water/water_ripple_anim_animations.json",
        // Waterfalls
        "environment/animations/waterfalls/waterfall_small_anim_animations.json",
        "environment/animations/waterfalls/waterfall_medium_anim_animations.json",
        "environment/animations/waterfalls/waterfall_large_anim_animations.json",
        // Fire
        "environment/animations/fire/fire_small_anim_animations.json",
        "environment/animations/fire/fire_medium_anim_animations.json",
        "environment/animations/fire/fire_large_anim_animations.json",
        // Grass
        "environment/animations/grass/grass_small_wind_anim_animations.json",
        "environment/animations/grass/grass_tall_wind_anim_animations.json",
        "environment/animations/grass/grass_dense_wind_anim_animations.json",
        "environment/animations/grass/grass_flowering_wind_anim_animations.json",
        // Trees
        "environment/animations/trees/tree_oak_breeze_anim_animations.json",
        "environment/animations/trees/tree_pine_breeze_anim_animations.json",
        "environment/animations/trees/bush_rustle_anim_animations.json",
        "environment/animations/trees/leaves_falling_anim_animations.json",
        // Smoke
        "environment/atmosphere/smoke/smoke_chimney_anim_animations.json",
        "environment/atmosphere/smoke/smoke_campfire_anim_animations.json",
        "environment/atmosphere/smoke/smoke_plume_anim_animations.json",
        // Airplane
        "environment/events/airplanes/airplane_recon_001_animations.json"
    };

    for (const auto& sf : sheetFiles) {
        assetMgr.LoadSpriteSheetMetadata(sf);
    }
    for (const auto& af : animFiles) {
        assetMgr.LoadAnimationMetadata(af);
    }

    InitDefaultObjects();
    InitAtmosphere();

    // Spawn starter ambient birds & butterflies in Forest map bounds
    Rectangle defaultBounds{ 64.0f, 64.0f, 1500.0f, 1500.0f };
    SpawnAmbientBird(Vector2{ 300.0f, 400.0f }, defaultBounds);
    SpawnAmbientBird(Vector2{ 600.0f, 750.0f }, defaultBounds);

    SpawnAmbientButterfly(Vector2{ 280.0f, 420.0f }, "forest_butterfly", defaultBounds);
    SpawnAmbientButterfly(Vector2{ 340.0f, 450.0f }, "blue_butterfly", defaultBounds);
    SpawnAmbientButterfly(Vector2{ 500.0f, 350.0f }, "golden_butterfly", defaultBounds);
    SpawnAmbientButterfly(Vector2{ 650.0f, 820.0f }, "forest_butterfly", defaultBounds);

    m_initialized = true;
    core::Logger::Info("ENV_ANIM", "Environmental Animation System initialized successfully.");
    return true;
}

void EnvironmentalAnimationSystem::PopulateFromWorld(const World& world, resources::AssetManager& /*assetMgr*/) {
    const Tilemap* tilemap = world.GetTilemap();
    if (!tilemap) return;

    m_animatedObjects.clear();
    m_smokePlumes.clear();
    m_ambientCreatures.clear();

    int objId = 1;

    for (const auto& obj : tilemap->GetObjects()) {
        const std::string& type = obj.GetType();
        Vector2 pos = obj.GetPosition();

        if (type == "waterfall_large_anim") {
            AddAnimatedObject({ objId++, obj.GetName(), "waterfall", pos, { 64.0f, 80.0f },
                                "sheet_waterfall_large_anim", "waterfall_large_anim_loop", 0.09f, 6, 0.0f, 0, 0.0f, 1 });
        } else if (type == "waterfall_medium_anim") {
            AddAnimatedObject({ objId++, obj.GetName(), "waterfall", pos, { 48.0f, 64.0f },
                                "sheet_waterfall_medium_anim", "waterfall_medium_anim_loop", 0.10f, 6, 0.0f, 0, 0.0f, 1 });
        } else if (type == "waterfall_small_anim") {
            AddAnimatedObject({ objId++, obj.GetName(), "waterfall", pos, { 32.0f, 48.0f },
                                "sheet_waterfall_small_anim", "waterfall_small_anim_loop", 0.11f, 6, 0.0f, 0, 0.0f, 1 });
        } else if (type == "fire_medium_anim" || type == "campfire" || type == "fire_campfire_medium_001") {
            AddAnimatedObject({ objId++, obj.GetName(), "fire", pos, { 24.0f, 32.0f },
                                "sheet_fire_medium_anim", "fire_medium_anim_loop", 0.09f, 6, 0.0f, 0, 0.0f, 1 });
        } else if (type == "fire_small_anim" || type == "torch") {
            AddAnimatedObject({ objId++, obj.GetName(), "fire", pos, { 16.0f, 24.0f },
                                "sheet_fire_small_anim", "fire_small_anim_loop", 0.10f, 6, 0.0f, 0, 0.0f, 1 });
        } else if (type == "smoke_chimney_anim") {
            m_smokePlumes.push_back({ pos, "sheet_smoke_chimney_anim", "smoke_chimney_anim_loop", 0.0f, 0, 0.15f, 6 });
        } else if (type == "smoke_plume_anim") {
            m_smokePlumes.push_back({ pos, "sheet_smoke_plume_anim", "smoke_plume_anim_loop", 0.0f, 0, 0.12f, 6 });
        } else if (type == "ambient_bird") {
            Rectangle bounds{ pos.x - 400.0f, pos.y - 300.0f, 800.0f, 600.0f };
            SpawnAmbientBird(pos, bounds);
        } else if (type == "ambient_butterfly") {
            Rectangle bounds{ pos.x - 300.0f, pos.y - 300.0f, 600.0f, 600.0f };
            SpawnAmbientButterfly(pos, "forest_butterfly", bounds);
        }
    }

    // Add environmental water ripples and flow patches across the major water features:
    // 1. Silver Lake Shimmer ripples (Subregion 4)
    AddAnimatedObject({ objId++, "Silver Lake Shimmer A", "water", { 850.0f, 1800.0f }, { 64.0f, 64.0f }, "sheet_water_lake_anim", "water_lake_anim_loop", 0.20f, 4, 0.0f, 0, 0.0f, 0 });
    AddAnimatedObject({ objId++, "Silver Lake Shimmer B", "water", { 1000.0f, 1920.0f }, { 64.0f, 64.0f }, "sheet_water_lake_anim", "water_lake_anim_loop", 0.20f, 4, 0.0f, 0, 0.5f, 0 });
    AddAnimatedObject({ objId++, "Silver Lake Ripple", "water", { 750.0f, 1720.0f }, { 32.0f, 32.0f }, "sheet_water_ripple_anim", "water_ripple_anim_loop", 0.15f, 4, 0.0f, 0, 0.25f, 0 });

    // 2. Serpentine River Rapids & Flow (Subregion 6 & 8)
    AddAnimatedObject({ objId++, "Riverwood Rapids", "water", { 3520.0f, 1760.0f }, { 64.0f, 32.0f }, "sheet_water_flow_anim", "water_flow_anim_loop", 0.12f, 4, 0.0f, 0, 0.0f, 0 });
    AddAnimatedObject({ objId++, "South River Flow", "water", { 2800.0f, 3000.0f }, { 96.0f, 32.0f }, "sheet_water_river_anim", "water_river_anim_loop", 0.14f, 4, 0.0f, 0, 0.3f, 0 });

    // 3. Lily Pond ripples (Subregion 3)
    AddAnimatedObject({ objId++, "Lily Pond Ripple", "water", { 4000.0f, 480.0f }, { 32.0f, 32.0f }, "sheet_water_ripple_anim", "water_ripple_anim_loop", 0.15f, 4, 0.0f, 0, 0.4f, 0 });

    // 4. Foliage swaying overlays in meadows & settlements
    AddAnimatedObject({ objId++, "Sunlit Meadow Wind Grass A", "grass", { 800.0f, 3000.0f }, { 24.0f, 32.0f }, "sheet_grass_tall_wind_anim", "grass_tall_wind_anim_loop", 0.18f, 4, 0.0f, 0, 0.0f, 0 });
    AddAnimatedObject({ objId++, "Sunlit Meadow Wind Grass B", "grass", { 1100.0f, 2800.0f }, { 24.0f, 32.0f }, "sheet_grass_flowering_wind_anim", "grass_flowering_wind_anim_loop", 0.20f, 4, 0.0f, 0, 0.4f, 0 });
    AddAnimatedObject({ objId++, "Emerald Grove Oak Breeze", "tree", { 2560.0f, 1750.0f }, { 48.0f, 48.0f }, "sheet_tree_oak_breeze_anim", "tree_oak_breeze_anim_loop", 0.22f, 4, 0.0f, 0, 0.0f, 1 });

    core::Logger::Info("ENV_ANIM", "Populated " + std::to_string(m_animatedObjects.size()) +
                       " animated objects, " + std::to_string(m_smokePlumes.size()) + " smoke plumes, and " +
                       std::to_string(m_ambientCreatures.size()) + " wildlife entities from World map.");
}

void EnvironmentalAnimationSystem::InitDefaultObjects() {
    m_animatedObjects.clear();

    // 1. Water Bodies (Layer 0: Ground)
    AddAnimatedObject({ 1, "Forest Pond Water", "water", { 480.0f, 380.0f }, { 64.0f, 64.0f }, "sheet_water_pond_anim", "water_pond_anim_loop", 0.18f, 4, 0.0f, 0, 0.0f, 0 });
    AddAnimatedObject({ 2, "River Water Current", "water", { 800.0f, 600.0f }, { 96.0f, 32.0f }, "sheet_water_river_anim", "water_river_anim_loop", 0.14f, 4, 0.0f, 0, 0.2f, 0 });
    AddAnimatedObject({ 3, "Lake Shore Ripple", "water", { 520.0f, 410.0f }, { 32.0f, 32.0f }, "sheet_water_ripple_anim", "water_ripple_anim_loop", 0.15f, 4, 0.0f, 0, 0.5f, 0 });

    // 2. Waterfalls (Layer 1: Object)
    AddAnimatedObject({ 10, "Cliff Waterfall Medium", "waterfall", { 920.0f, 480.0f }, { 48.0f, 64.0f }, "sheet_waterfall_medium_anim", "waterfall_medium_anim_loop", 0.10f, 6, 0.0f, 0, 0.0f, 1 });
    AddAnimatedObject({ 11, "Gorge Waterfall Small", "waterfall", { 980.0f, 500.0f }, { 32.0f, 48.0f }, "sheet_waterfall_small_anim", "waterfall_small_anim_loop", 0.11f, 6, 0.0f, 0, 0.3f, 1 });

    // 3. Fire Instances (Layer 1: Object)
    AddAnimatedObject({ 20, "Campfire Flame", "fire", { 260.0f, 940.0f }, { 24.0f, 32.0f }, "sheet_fire_medium_anim", "fire_medium_anim_loop", 0.09f, 6, 0.0f, 0, 0.0f, 1 });
    AddAnimatedObject({ 21, "Torch Flame Left", "fire", { 240.0f, 920.0f }, { 16.0f, 24.0f }, "sheet_fire_small_anim", "fire_small_anim_loop", 0.10f, 6, 0.0f, 0, 0.4f, 1 });
    AddAnimatedObject({ 22, "Forge Fire Large", "fire", { 380.0f, 850.0f }, { 32.0f, 48.0f }, "sheet_fire_large_anim", "fire_large_anim_loop", 0.08f, 6, 0.0f, 0, 0.2f, 1 });

    // 4. Wind-Responsive Grass (Layer 0/1)
    AddAnimatedObject({ 30, "Tall Wind Grass A", "grass", { 220.0f, 960.0f }, { 24.0f, 32.0f }, "sheet_grass_tall_wind_anim", "grass_tall_wind_anim_loop", 0.18f, 4, 0.0f, 0, 0.1f, 0 });
    AddAnimatedObject({ 31, "Tall Wind Grass B", "grass", { 240.0f, 980.0f }, { 24.0f, 32.0f }, "sheet_grass_tall_wind_anim", "grass_tall_wind_anim_loop", 0.18f, 4, 0.0f, 0, 0.7f, 0 });
    AddAnimatedObject({ 32, "Flowering Wind Grass", "grass", { 290.0f, 950.0f }, { 24.0f, 24.0f }, "sheet_grass_flowering_wind_anim", "grass_flowering_wind_anim_loop", 0.20f, 4, 0.0f, 0, 0.35f, 0 });

    // 5. Tree Foliage & Breeze Movement (Layer 1: Object overlay)
    AddAnimatedObject({ 40, "Oak Canopy Breeze Overlay", "tree", { 256.0f, 900.0f }, { 48.0f, 48.0f }, "sheet_tree_oak_breeze_anim", "tree_oak_breeze_anim_loop", 0.22f, 4, 0.0f, 0, 0.0f, 1 });
    AddAnimatedObject({ 41, "Falling Leaves Overlay", "tree", { 270.0f, 920.0f }, { 32.0f, 32.0f }, "sheet_leaves_falling_anim", "leaves_falling_anim_loop", 0.14f, 6, 0.0f, 0, 0.5f, 1 });
}

void EnvironmentalAnimationSystem::InitAtmosphere() {
    m_clouds.clear();
    m_smokePlumes.clear();

    // Overhead clouds drifting slowly East (Layer 2)
    m_clouds.push_back({ { 100.0f, 120.0f }, { 12.0f, 0.0f }, 1.0f, 0.65f, "tex_cloud_medium" });
    m_clouds.push_back({ { 450.0f, 80.0f }, { 15.0f, 0.0f }, 1.2f, 0.55f, "tex_cloud_large" });
    m_clouds.push_back({ { 750.0f, 220.0f }, { 10.0f, 0.0f }, 0.9f, 0.70f, "tex_cloud_small" });
    m_clouds.push_back({ { 300.0f, 480.0f }, { 14.0f, 0.0f }, 1.1f, 0.60f, "tex_cloud_cluster" });
    m_clouds.push_back({ { 900.0f, 600.0f }, { 11.0f, 0.0f }, 1.0f, 0.50f, "tex_cloud_thin" });

    // Smoke plumes above chimney / campfires
    m_smokePlumes.push_back({ { 260.0f, 915.0f }, "sheet_smoke_campfire_anim", "smoke_campfire_anim_loop", 0.0f, 0, 0.14f, 6 });
    m_smokePlumes.push_back({ { 420.0f, 790.0f }, "sheet_smoke_chimney_anim", "smoke_chimney_anim_loop", 0.0f, 0, 0.15f, 6 });
}

void EnvironmentalAnimationSystem::AddAnimatedObject(const AnimatedEnvObject& obj) {
    m_animatedObjects.push_back(obj);
}

void EnvironmentalAnimationSystem::SpawnAmbientBird(Vector2 pos, Rectangle bounds) {
    AmbientCreature bird;
    bird.id = m_nextEntityId++;
    bird.type = AmbientType::Bird;
    bird.species = "small_bird";
    bird.position = pos;
    bird.targetPosition = Vector2{ pos.x + 100.0f, pos.y - 50.0f };
    bird.velocity = Vector2{ 35.0f, -15.0f };
    bird.altitude = 12.0f;
    bird.state = 0; // Flying
    bird.stateTimer = 4.0f;
    bird.spritesheetId = "sheet_small_bird";
    bird.currentClip = "bird_fly";
    bird.animTimer = 0.0f;
    bird.currentFrame = 0;
    bird.wanderBounds = bounds;
    m_ambientCreatures.push_back(bird);
}

void EnvironmentalAnimationSystem::SpawnAmbientButterfly(Vector2 pos, const std::string& species, Rectangle bounds) {
    AmbientCreature bfly;
    bfly.id = m_nextEntityId++;
    bfly.type = AmbientType::Butterfly;
    bfly.species = species;
    bfly.position = pos;
    bfly.targetPosition = Vector2{ pos.x + 20.0f, pos.y + 10.0f };
    bfly.velocity = Vector2{ 15.0f, 5.0f };
    bfly.altitude = 6.0f;
    bfly.state = 0; // Flying
    bfly.stateTimer = 3.0f;
    bfly.spritesheetId = "sheet_" + species;
    bfly.currentClip = species + "_fly";
    bfly.animTimer = 0.0f;
    bfly.currentFrame = 0;
    bfly.wanderBounds = bounds;
    m_ambientCreatures.push_back(bfly);
}

void EnvironmentalAnimationSystem::TriggerAirplaneFlyover(Vector2 startPos, Vector2 endPos, float speed) {
    m_airplaneEvent.active = true;
    m_airplaneEvent.startPos = startPos;
    m_airplaneEvent.endPos = endPos;
    m_airplaneEvent.currentPos = startPos;
    m_airplaneEvent.speed = speed;
    m_airplaneEvent.altitude = 45.0f;
    m_airplaneEvent.timer = 0.0f;
    m_airplaneEvent.currentFrame = 0;

    float dx = endPos.x - startPos.x;
    float dy = endPos.y - startPos.y;
    m_airplaneEvent.rotation = std::atan2(dy, dx) * (180.0f / 3.14159265f) + 90.0f;

    core::Logger::Info("ENV_ANIM", "Triggered airplane flyover event from (" +
                       std::to_string(startPos.x) + ", " + std::to_string(startPos.y) + ") to (" +
                       std::to_string(endPos.x) + ", " + std::to_string(endPos.y) + ")");
}

void EnvironmentalAnimationSystem::Update(float dt, const graphics::Camera2DSystem& camera, const World* world) {
    if (!m_initialized) return;

    float effectiveDt = dt * m_globalSpeed;

    UpdateAnimatedObjects(effectiveDt);
    if (m_ambientEnabled) {
        UpdateAmbientCreatures(effectiveDt, world);
    }
    UpdateAtmosphere(effectiveDt);
    UpdateAirplaneEvent(effectiveDt);
}

void EnvironmentalAnimationSystem::UpdateAnimatedObjects(float dt) {
    for (auto& obj : m_animatedObjects) {
        if (!m_waterEnabled && obj.type == "water") continue;
        if (!m_waterEnabled && obj.type == "waterfall") continue;
        if (!m_fireEnabled && obj.type == "fire") continue;
        if (!m_windEnabled && (obj.type == "grass" || obj.type == "tree")) continue;

        obj.timer += dt;
        if (obj.timer >= obj.frameDuration) {
            obj.timer -= obj.frameDuration;
            obj.currentFrame = (obj.currentFrame + 1) % obj.totalFrames;
        }
    }
}

void EnvironmentalAnimationSystem::UpdateAmbientCreatures(float dt, const World* world) {
    for (auto& c : m_ambientCreatures) {
        c.stateTimer -= dt;
        c.animTimer += dt;

        // Frame timing
        float frameDur = (c.type == AmbientType::Bird) ? 0.08f : 0.12f;
        if (c.animTimer >= frameDur) {
            c.animTimer -= frameDur;
            c.currentFrame = (c.currentFrame + 1) % 4;
        }

        // State Machine
        if (c.stateTimer <= 0.0f) {
            if (c.state == 0) { // Was Flying -> maybe Rest/Hover
                c.state = (c.type == AmbientType::Bird) ? 2 : 1; // Bird perches, Butterfly hovers
                c.stateTimer = 2.0f + static_cast<float>(rand() % 3);
                if (c.type == AmbientType::Bird) {
                    c.currentClip = "bird_idle";
                    c.altitude = 0.0f;
                    c.velocity = { 0.0f, 0.0f };
                } else {
                    c.currentClip = c.species + "_resting";
                    c.altitude = 2.0f;
                    c.velocity = { 0.0f, 0.0f };
                }
            } else { // Was Resting/Hovering -> Takeoff & Fly
                c.state = 0; // Flying
                c.stateTimer = 4.0f + static_cast<float>(rand() % 5);
                c.altitude = (c.type == AmbientType::Bird) ? 14.0f : 6.0f;
                c.currentClip = (c.type == AmbientType::Bird) ? "bird_fly" : (c.species + "_fly");

                // Pick a new random target position within wander bounds
                float rx = c.wanderBounds.x + static_cast<float>(rand() % static_cast<int>(c.wanderBounds.width));
                float ry = c.wanderBounds.y + static_cast<float>(rand() % static_cast<int>(c.wanderBounds.height));
                c.targetPosition = { rx, ry };

                float dx = c.targetPosition.x - c.position.x;
                float dy = c.targetPosition.y - c.position.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist > 1.0f) {
                    float speed = (c.type == AmbientType::Bird) ? 45.0f : 20.0f;
                    c.velocity = { (dx / dist) * speed, (dy / dist) * speed };
                }
            }
        }

        // Move if flying
        if (c.state == 0) {
            c.position.x += c.velocity.x * dt;
            c.position.y += c.velocity.y * dt;

            // Keep within bounds
            if (c.position.x < c.wanderBounds.x) c.velocity.x = std::abs(c.velocity.x);
            if (c.position.x > c.wanderBounds.x + c.wanderBounds.width) c.velocity.x = -std::abs(c.velocity.x);
            if (c.position.y < c.wanderBounds.y) c.velocity.y = std::abs(c.velocity.y);
            if (c.position.y > c.wanderBounds.y + c.wanderBounds.height) c.velocity.y = -std::abs(c.velocity.y);
        }
    }
}

void EnvironmentalAnimationSystem::UpdateAtmosphere(float dt) {
    // 1. Move Clouds
    if (m_cloudsEnabled) {
        for (auto& cloud : m_clouds) {
            cloud.position.x += cloud.velocity.x * dt;
            // Wrap around world boundaries (e.g. 1600 px)
            if (cloud.position.x > 1800.0f) {
                cloud.position.x = -200.0f;
            }
        }
    }

    // 2. Animate Smoke Plumes
    for (auto& smoke : m_smokePlumes) {
        smoke.timer += dt;
        if (smoke.timer >= smoke.frameDuration) {
            smoke.timer -= smoke.frameDuration;
            smoke.currentFrame = (smoke.currentFrame + 1) % smoke.totalFrames;
        }
    }

    // 3. Scroll Fog Layer
    if (m_fogEnabled) {
        m_fogScrollOffset += 8.0f * dt;
        if (m_fogScrollOffset > 128.0f) {
            m_fogScrollOffset -= 128.0f;
        }
    }
}

void EnvironmentalAnimationSystem::UpdateAirplaneEvent(float dt) {
    if (!m_airplaneEvent.active) return;

    // Propeller animation
    m_airplaneEvent.timer += dt;
    if (m_airplaneEvent.timer >= m_airplaneEvent.frameDuration) {
        m_airplaneEvent.timer -= m_airplaneEvent.frameDuration;
        m_airplaneEvent.currentFrame = (m_airplaneEvent.currentFrame + 1) % m_airplaneEvent.totalFrames;
    }

    // Move along vector
    float dx = m_airplaneEvent.endPos.x - m_airplaneEvent.currentPos.x;
    float dy = m_airplaneEvent.endPos.y - m_airplaneEvent.currentPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    float step = m_airplaneEvent.speed * dt;
    if (dist <= step) {
        m_airplaneEvent.currentPos = m_airplaneEvent.endPos;
        m_airplaneEvent.active = false; // Event completed
        core::Logger::Info("ENV_ANIM", "Airplane flyover event completed and cleaned up.");
    } else {
        m_airplaneEvent.currentPos.x += (dx / dist) * step;
        m_airplaneEvent.currentPos.y += (dy / dist) * step;
    }
}

void EnvironmentalAnimationSystem::RenderWorldGround(graphics::Renderer& renderer, const graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr) {
    if (!m_initialized) return;

    // Render Ground-layer animated objects (Water pond, river, lake ripples)
    for (const auto& obj : m_animatedObjects) {
        if (obj.layer != 0) continue;
        if (!m_waterEnabled && obj.type == "water") continue;

        const auto* sheet = assetMgr.GetSpriteSheet(obj.spritesheetId);
        if (!sheet) continue;

        Texture2D tex = assetMgr.GetTexture(sheet->GetTextureId());
        if (tex.id == 0) continue;

        Rectangle srcRect = sheet->GetFrameRect(obj.currentFrame);
        Rectangle destRect{ obj.position.x, obj.position.y, obj.size.x, obj.size.y };
        renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, obj.tint);
    }
}

void EnvironmentalAnimationSystem::RenderWorldObjects(graphics::Renderer& renderer, const graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr) {
    if (!m_initialized) return;

    // 1. Render Object-layer animated world objects (Waterfalls, Fire, Grass, Tree sway)
    for (const auto& obj : m_animatedObjects) {
        if (obj.layer != 1) continue;
        if (!m_fireEnabled && obj.type == "fire") continue;
        if (!m_waterEnabled && obj.type == "waterfall") continue;
        if (!m_windEnabled && (obj.type == "grass" || obj.type == "tree")) continue;

        const auto* sheet = assetMgr.GetSpriteSheet(obj.spritesheetId);
        if (!sheet) continue;

        Texture2D tex = assetMgr.GetTexture(sheet->GetTextureId());
        if (tex.id == 0) continue;

        Rectangle srcRect = sheet->GetFrameRect(obj.currentFrame);
        Rectangle destRect{ obj.position.x, obj.position.y, obj.size.x, obj.size.y };
        renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, obj.tint);
    }

    // 2. Render Ambient Creatures (Birds & Butterflies)
    if (m_ambientEnabled) {
        for (const auto& c : m_ambientCreatures) {
            const auto* sheet = assetMgr.GetSpriteSheet(c.spritesheetId);
            if (!sheet) continue;

            Texture2D tex = assetMgr.GetTexture(sheet->GetTextureId());
            if (tex.id == 0) continue;

            float fw = static_cast<float>(sheet->GetFrameWidth());
            float fh = static_cast<float>(sheet->GetFrameHeight());
            Rectangle srcRect = sheet->GetFrameRect(c.currentFrame);

            // Ground shadow
            if (c.altitude > 1.0f) {
                DrawEllipse(static_cast<int>(c.position.x), static_cast<int>(c.position.y),
                            c.type == AmbientType::Bird ? 5.0f : 3.0f,
                            c.type == AmbientType::Bird ? 3.0f : 2.0f,
                            Color{ 20, 25, 35, 100 });
            }

            // Creature sprite with elevation offset
            Rectangle destRect{
                c.position.x - fw * 0.5f,
                c.position.y - fh * 0.5f - c.altitude,
                fw,
                fh
            };
            renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }
}

void EnvironmentalAnimationSystem::RenderWorldOverhead(graphics::Renderer& renderer, const graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr) {
    if (!m_initialized) return;

    // 1. Smoke Plumes
    for (const auto& smoke : m_smokePlumes) {
        const auto* sheet = assetMgr.GetSpriteSheet(smoke.spritesheetId);
        if (!sheet) continue;

        Texture2D tex = assetMgr.GetTexture(sheet->GetTextureId());
        if (tex.id == 0) continue;

        float fw = static_cast<float>(sheet->GetFrameWidth());
        float fh = static_cast<float>(sheet->GetFrameHeight());
        Rectangle srcRect = sheet->GetFrameRect(smoke.currentFrame);
        Rectangle destRect{ smoke.position.x - fw * 0.5f, smoke.position.y - fh, fw, fh };
        renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    }

    // 2. Overhead Clouds
    if (m_cloudsEnabled) {
        for (const auto& cloud : m_clouds) {
            Texture2D tex = assetMgr.GetTexture(cloud.textureId);
            if (tex.id == 0) continue;

            float w = static_cast<float>(tex.width) * cloud.scale;
            float h = static_cast<float>(tex.height) * cloud.scale;
            Rectangle srcRect{ 0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height) };
            Rectangle destRect{ cloud.position.x, cloud.position.y, w, h };
            Color cloudTint{ 255, 255, 255, static_cast<unsigned char>(cloud.alpha * 255.0f) };
            renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, cloudTint);
        }
    }

    // 3. Airplane Flyover Event
    if (m_airplaneEvent.active) {
        const auto* sheet = assetMgr.GetSpriteSheet(m_airplaneEvent.spritesheetId);
        if (sheet) {
            Texture2D tex = assetMgr.GetTexture(sheet->GetTextureId());
            if (tex.id != 0) {
                float fw = static_cast<float>(sheet->GetFrameWidth());
                float fh = static_cast<float>(sheet->GetFrameHeight());
                Rectangle srcRect = sheet->GetFrameRect(m_airplaneEvent.currentFrame);

                // Ground shadow
                Rectangle shadowDest{
                    m_airplaneEvent.currentPos.x - fw * 0.5f + 25.0f,
                    m_airplaneEvent.currentPos.y - fh * 0.5f + 25.0f,
                    fw,
                    fh
                };
                renderer.DrawSprite(tex, srcRect, shadowDest, Vector2{ fw * 0.5f, fh * 0.5f },
                                    m_airplaneEvent.rotation, Color{ 15, 20, 30, 90 });

                // Aircraft sprite at flight altitude
                Rectangle airDest{
                    m_airplaneEvent.currentPos.x - fw * 0.5f,
                    m_airplaneEvent.currentPos.y - fh * 0.5f - m_airplaneEvent.altitude,
                    fw,
                    fh
                };
                renderer.DrawSprite(tex, srcRect, airDest, Vector2{ fw * 0.5f, fh * 0.5f },
                                    m_airplaneEvent.rotation, WHITE);
            }
        }
    }
}

void EnvironmentalAnimationSystem::RenderScreenAtmosphere(graphics::Renderer& renderer, const graphics::Viewport& viewport, resources::AssetManager& assetMgr) {
    if (!m_initialized || !m_fogEnabled || m_fogOpacity <= 0.01f) return;

    Texture2D fogTex = assetMgr.GetTexture("tex_fog_light_layer");
    if (fogTex.id == 0) return;

    // Tile scrolling fog across screen
    float vw = static_cast<float>(viewport.GetVirtualWidth());
    float vh = static_cast<float>(viewport.GetVirtualHeight());

    Color fogTint{ 240, 245, 255, static_cast<unsigned char>(m_fogOpacity * 255.0f) };

    int tw = fogTex.width;
    int th = fogTex.height;

    for (float x = -m_fogScrollOffset; x < vw + tw; x += tw) {
        for (float y = 0.0f; y < vh + th; y += th) {
            Rectangle srcRect{ 0.0f, 0.0f, static_cast<float>(tw), static_cast<float>(th) };
            Rectangle destRect{ x, y, static_cast<float>(tw), static_cast<float>(th) };
            renderer.DrawSprite(fogTex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, fogTint);
        }
    }
}

} // namespace wildbound::world::environment

