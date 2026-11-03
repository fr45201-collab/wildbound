#pragma once

#include "EngineConfig.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/Camera2D.hpp"
#include "../graphics/TestScene.hpp"
#include "../resources/AssetManager.hpp"
#include "../world/World.hpp"
#include "../world/WorldRenderer.hpp"
#include <memory>

namespace wildbound::scripting {
    class LuaManager;
}

namespace wildbound::ui {
    class DevUI;
}

namespace wildbound::core {

class Game {
public:
    explicit Game(const EngineConfig& initialConfig = EngineConfig::CreateDefault());
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    bool Initialize();
    void Run();
    void Shutdown();

    void Stop() { m_running = false; }
    bool IsRunning() const { return m_running; }

    const EngineConfig& GetConfig() const { return m_config; }
    graphics::Renderer& GetRenderer() { return *m_renderer; }
    graphics::Camera2DSystem& GetCamera() { return *m_camera; }
    resources::AssetManager& GetAssetManager() { return *m_assetManager; }
    world::World& GetWorld() { return *m_world; }
    world::WorldRenderer& GetWorldRenderer() { return *m_worldRenderer; }
    scripting::LuaManager& GetScripting() { return *m_luaManager; }

private:
    void ProcessInput(float dt);
    void FixedUpdate(float fixedDt);
    void Update(float dt);
    void Render(float dt);

    EngineConfig m_config;
    bool m_running = false;
    float m_runningTime = 0.0f;

    std::unique_ptr<graphics::Renderer> m_renderer;
    std::unique_ptr<graphics::Camera2DSystem> m_camera;
    std::unique_ptr<graphics::TestScene> m_testScene;
    std::unique_ptr<resources::AssetManager> m_assetManager;
    std::unique_ptr<world::World> m_world;
    std::unique_ptr<world::WorldRenderer> m_worldRenderer;
    std::unique_ptr<scripting::LuaManager> m_luaManager;
    std::unique_ptr<ui::DevUI> m_devUI;
};

} // namespace wildbound::core
