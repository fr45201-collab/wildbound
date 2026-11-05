#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::graphics {
    class Renderer;
    class Camera2DSystem;
    class Viewport;
}

namespace wildbound::world {
    class World;
}

namespace wildbound::world::environment {

enum class AmbientType {
    Bird,
    Butterfly
};

struct AmbientCreature {
    int id = 0;
    AmbientType type = AmbientType::Bird;
    std::string species;
    Vector2 position{ 0.0f, 0.0f };
    Vector2 velocity{ 0.0f, 0.0f };
    Vector2 targetPosition{ 0.0f, 0.0f };
    float altitude = 0.0f;
    float stateTimer = 0.0f;
    int state = 0; // 0=Flying, 1=Hovering, 2=Resting
    std::string spritesheetId;
    std::string currentClip;
    float animTimer = 0.0f;
    int currentFrame = 0;
    Rectangle wanderBounds{ 0.0f, 0.0f, 1200.0f, 1200.0f };
};

struct AnimatedEnvObject {
    int id = 0;
    std::string name;
    std::string type; // "water", "waterfall", "fire", "grass", "tree"
    Vector2 position{ 0.0f, 0.0f };
    Vector2 size{ 32.0f, 32.0f };
    std::string spritesheetId;
    std::string clipName;
    float frameDuration = 0.15f;
    int totalFrames = 4;
    float timer = 0.0f;
    int currentFrame = 0;
    float phaseOffset = 0.0f;
    int layer = 1; // 0=Ground, 1=Object, 2=Overhead
    Color tint = WHITE;
};

struct CloudInstance {
    Vector2 position{ 0.0f, 0.0f };
    Vector2 velocity{ 15.0f, 0.0f };
    float scale = 1.0f;
    float alpha = 0.6f;
    std::string textureId;
};

struct SmokeInstance {
    Vector2 position{ 0.0f, 0.0f };
    std::string spritesheetId;
    std::string clipName;
    float timer = 0.0f;
    int currentFrame = 0;
    float frameDuration = 0.14f;
    int totalFrames = 6;
};

struct AirplaneFlyoverEvent {
    bool active = false;
    Vector2 startPos{ 0.0f, 0.0f };
    Vector2 endPos{ 0.0f, 0.0f };
    Vector2 currentPos{ 0.0f, 0.0f };
    float speed = 130.0f;
    float altitude = 45.0f;
    float rotation = 0.0f;
    float timer = 0.0f;
    int currentFrame = 0;
    float frameDuration = 0.06f;
    int totalFrames = 4;
    std::string spritesheetId = "sheet_airplane_recon_001";
    std::string clipName = "airplane_recon_001_loop";
};

/**
 * Unified living-world environmental animation and atmospheric management system.
 * Governs the 4 Environmental Categories:
 * - Category A: Animated Environmental Objects (Water, Waterfalls, Fire, Grass, Trees)
 * - Category B: Ambient Entities (Birds, Butterflies)
 * - Category C: Atmospheric Effects (Clouds, Fog, Smoke)
 * - Category D: Scripted Environmental Events (Airplane Flyovers)
 */
class EnvironmentalAnimationSystem {
public:
    EnvironmentalAnimationSystem();
    ~EnvironmentalAnimationSystem();

    bool Initialize(resources::AssetManager& assetMgr);
    void PopulateFromWorld(const World& world, resources::AssetManager& assetMgr);
    void Update(float dt, const graphics::Camera2DSystem& camera, const World* world = nullptr);

    // Render passes
    void RenderWorldGround(graphics::Renderer& renderer, const graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr);
    void RenderWorldObjects(graphics::Renderer& renderer, const graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr);
    void RenderWorldOverhead(graphics::Renderer& renderer, const graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr);
    void RenderScreenAtmosphere(graphics::Renderer& renderer, const graphics::Viewport& viewport, resources::AssetManager& assetMgr);

    // Spawning & Control API
    void AddAnimatedObject(const AnimatedEnvObject& obj);
    void SpawnAmbientBird(Vector2 pos, Rectangle bounds);
    void SpawnAmbientButterfly(Vector2 pos, const std::string& species, Rectangle bounds);
    void TriggerAirplaneFlyover(Vector2 startPos, Vector2 endPos, float speed = 130.0f);

    // Controls & Queries
    bool IsWaterEnabled() const { return m_waterEnabled; }
    void SetWaterEnabled(bool e) { m_waterEnabled = e; }

    bool IsFireEnabled() const { return m_fireEnabled; }
    void SetFireEnabled(bool e) { m_fireEnabled = e; }

    bool IsWindEnabled() const { return m_windEnabled; }
    void SetWindEnabled(bool e) { m_windEnabled = e; }

    bool IsAmbientEnabled() const { return m_ambientEnabled; }
    void SetAmbientEnabled(bool e) { m_ambientEnabled = e; }

    bool IsCloudsEnabled() const { return m_cloudsEnabled; }
    void SetCloudsEnabled(bool e) { m_cloudsEnabled = e; }

    bool IsFogEnabled() const { return m_fogEnabled; }
    void SetFogEnabled(bool e) { m_fogEnabled = e; }
    float GetFogOpacity() const { return m_fogOpacity; }
    void SetFogOpacity(float op) { m_fogOpacity = op; }

    float GetGlobalPlaybackSpeed() const { return m_globalSpeed; }
    void SetGlobalPlaybackSpeed(float s) { m_globalSpeed = s > 0.0f ? s : 1.0f; }

    const AirplaneFlyoverEvent& GetAirplaneEvent() const { return m_airplaneEvent; }
    size_t GetAnimatedObjectCount() const { return m_animatedObjects.size(); }
    size_t GetAmbientCreatureCount() const { return m_ambientCreatures.size(); }
    size_t GetCloudCount() const { return m_clouds.size(); }

    void ClearAmbientCreatures() { m_ambientCreatures.clear(); }

private:
    void UpdateAnimatedObjects(float dt);
    void UpdateAmbientCreatures(float dt, const World* world);
    void UpdateAtmosphere(float dt);
    void UpdateAirplaneEvent(float dt);

    void InitDefaultObjects();
    void InitAtmosphere();

    bool m_initialized = false;
    float m_globalSpeed = 1.0f;

    // Feature toggles
    bool m_waterEnabled = true;
    bool m_fireEnabled = true;
    bool m_windEnabled = true;
    bool m_ambientEnabled = true;
    bool m_cloudsEnabled = true;
    bool m_fogEnabled = true;
    float m_fogOpacity = 0.35f;
    float m_fogScrollOffset = 0.0f;

    // Category containers
    std::vector<AnimatedEnvObject> m_animatedObjects;
    std::vector<AmbientCreature> m_ambientCreatures;
    std::vector<CloudInstance> m_clouds;
    std::vector<SmokeInstance> m_smokePlumes;
    AirplaneFlyoverEvent m_airplaneEvent;

    int m_nextEntityId = 1000;
};

} // namespace wildbound::world::environment

