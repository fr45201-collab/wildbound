#pragma once

#include "EngineConfig.hpp"
#include <memory>

namespace wildbound::graphics {
    class RenderManager;
}

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
    scripting::LuaManager& GetScripting() { return *m_luaManager; }

private:
    void ProcessInput();
    void FixedUpdate(float fixedDt);
    void Update(float dt);
    void Render();

    EngineConfig m_config;
    bool m_running = false;
    float m_runningTime = 0.0f;

    std::unique_ptr<graphics::RenderManager> m_renderManager;
    std::unique_ptr<scripting::LuaManager> m_luaManager;
    std::unique_ptr<ui::DevUI> m_devUI;
};

} // namespace wildbound::core

