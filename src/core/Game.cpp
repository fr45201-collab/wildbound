#include "Game.hpp"
#include "Logger.hpp"
#include "../graphics/RenderManager.hpp"
#include "../scripting/LuaManager.hpp"
#include "../ui/DevUI.hpp"

#include <raylib.h>
#include <algorithm>

namespace wildbound::core {

Game::Game(const EngineConfig& initialConfig)
    : m_config(initialConfig)
    , m_renderManager(std::make_unique<graphics::RenderManager>())
    , m_luaManager(std::make_unique<scripting::LuaManager>())
    , m_devUI(std::make_unique<ui::DevUI>())
{
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    Logger::Info("ENGINE", "Initialization started.");

    // 1. Initialize Lua runtime
    if (!m_luaManager->Initialize()) {
        Logger::Error("ENGINE", "Failed to initialize Lua scripting runtime.");
        return false;
    }

    // 2. Load engine configuration from Lua
    if (!m_luaManager->LoadEngineConfig("scripts/config/engine.lua", m_config)) {
        Logger::Warn("ENGINE", "Could not load engine.lua, continuing with default configuration.");
    }

    // 3. Initialize Graphics / Raylib Window
    if (!m_renderManager->Initialize(m_config)) {
        Logger::Error("ENGINE", "Failed to initialize Graphics module.");
        return false;
    }

    // 4. Initialize DevUI (Dear ImGui + rlImGui)
    if (m_config.debugMode) {
        if (!m_devUI->Initialize()) {
            Logger::Warn("ENGINE", "Failed to initialize Developer ImGui overlay.");
        }
    }

    m_running = true;
    m_runningTime = 0.0f;
    Logger::Info("ENGINE", "Initialization completed.");
    return true;
}

void Game::Run() {
    if (!m_running) {
        Logger::Error("ENGINE", "Cannot run: Engine is not initialized.");
        return;
    }

    Logger::Info("ENGINE", "Entering main game loop.");

    constexpr float fixedDeltaTime = 1.0f / 60.0f;
    float accumulator = 0.0f;

    while (m_running && !m_renderManager->IsWindowClosed()) {
        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) {
            frameTime = 0.25f; // Clamp delta time to prevent spiral of death
        }

        ProcessInput();

        // Fixed timestep simulation updates
        accumulator += frameTime;
        while (accumulator >= fixedDeltaTime) {
            FixedUpdate(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }

        // Variable frame update
        Update(frameTime);

        // Render frame
        Render();
    }

    Logger::Info("ENGINE", "Exiting main game loop.");
}

void Game::ProcessInput() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        Stop();
    }

    if (IsKeyPressed(KEY_F1)) {
        if (m_devUI) {
            m_devUI->ToggleVisible();
        }
    }
}

void Game::FixedUpdate(float /*fixedDt*/) {
    // Fixed simulation updates (reserved for future physics & deterministic systems)
}

void Game::Update(float dt) {
    m_runningTime += dt;
    // Scripting tick or engine updates will be called here
}

void Game::Render() {
    float dt = GetFrameTime();
    float fps = static_cast<float>(GetFPS());

    // 1. Draw game content onto internal virtual canvas (640x360)
    m_renderManager->BeginVirtualCanvas();
    m_renderManager->DrawInitialScreen(m_config, m_runningTime);
    m_renderManager->EndVirtualCanvas();

    // 2. Render virtual canvas onto window viewport with letterboxing
    BeginDrawing();
    m_renderManager->RenderCanvasToWindow();

    // 3. Render developer tools (Dear ImGui) on top in native window coordinates
    if (m_config.debugMode && m_devUI && m_devUI->IsVisible()) {
        m_devUI->BeginFrame();
        m_devUI->RenderDebugOverlay(m_config, fps, dt, "ONLINE (sol2 / Lua 5.4)");
        m_devUI->EndFrame();
    }

    EndDrawing();
}

void Game::Shutdown() {
    if (m_running) {
        m_running = false;
    }

    if (m_devUI) {
        m_devUI->Shutdown();
    }

    if (m_renderManager) {
        m_renderManager->Shutdown();
    }

    if (m_luaManager) {
        m_luaManager->Shutdown();
    }

    Logger::Info("ENGINE", "Engine shutdown completed cleanly.");
}

} // namespace wildbound::core

