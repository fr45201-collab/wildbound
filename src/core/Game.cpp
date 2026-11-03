#include "Game.hpp"
#include "Logger.hpp"
#include "../scripting/LuaManager.hpp"
#include "../ui/DevUI.hpp"

#include <raylib.h>
#include <algorithm>

namespace wildbound::core {

Game::Game(const EngineConfig& initialConfig)
    : m_config(initialConfig)
    , m_renderer(std::make_unique<graphics::Renderer>())
    , m_camera(std::make_unique<graphics::Camera2DSystem>(m_config.internalWidth, m_config.internalHeight))
    , m_testScene(std::make_unique<graphics::TestScene>())
    , m_assetManager(std::make_unique<resources::AssetManager>())
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

    // 3. Initialize Renderer (Window + Virtual Canvas + Viewport)
    if (!m_renderer->Initialize(m_config)) {
        Logger::Error("ENGINE", "Failed to initialize Renderer subsystem.");
        return false;
    }

    // 4. Initialize Asset Management System
    if (!m_assetManager->Initialize("assets")) {
        Logger::Error("ENGINE", "Failed to initialize Asset Management System.");
        return false;
    }

    // Load registered spritesheet metadata and animations
    m_assetManager->LoadSpriteSheetMetadata("characters/player/player_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("ui/icons/item_icons.json");
    m_assetManager->LoadAnimationMetadata("characters/player/player_walk.json");

    // 5. Configure Camera system
    m_camera->SetVirtualDimensions(m_config.internalWidth, m_config.internalHeight);
    m_camera->SetWorldBounds(m_testScene->GetWorldBounds());
    m_camera->SetTarget(Vector2{ 800.0f, 600.0f });
    m_camera->SetZoomLimits(0.5f, 4.0f);
    m_camera->SetFollowSpeed(4.5f);
    m_camera->SetFollowTarget(m_testScene->GetTargetPosPtr());
    m_camera->SetMode(graphics::CameraMode::Follow);

    // 6. Initialize DevUI (Dear ImGui + rlImGui)
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

    while (m_running && !m_renderer->IsWindowClosed()) {
        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }

        m_runningTime += frameTime;

        ProcessInput(frameTime);

        accumulator += frameTime;
        while (accumulator >= fixedDeltaTime) {
            FixedUpdate(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }

        Update(frameTime);
        Render(frameTime);
    }

    Logger::Info("ENGINE", "Exiting main game loop.");
}

void Game::ProcessInput(float dt) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        Stop();
        return;
    }

    if (IsKeyPressed(KEY_F1)) {
        if (m_devUI) {
            m_devUI->ToggleVisible();
        }
    }

    if (IsKeyPressed(KEY_F2)) {
        if (m_camera->GetMode() == graphics::CameraMode::Follow) {
            m_camera->SetMode(graphics::CameraMode::Free);
            Logger::Info("CAMERA", "Camera mode switched to FREE (WASD to pan)");
        } else {
            m_camera->SetFollowTarget(m_testScene->GetTargetPosPtr());
            m_camera->SetMode(graphics::CameraMode::Follow);
            Logger::Info("CAMERA", "Camera mode switched to FOLLOW (Tracking animated target)");
        }
    }

    if (IsKeyPressed(KEY_R)) {
        m_camera->SetTarget(Vector2{ 800.0f, 600.0f });
        m_camera->SetZoom(1.0f);
        Logger::Info("CAMERA", "Camera reset to center (800, 600) @ 1.0x zoom");
    }

    if (m_camera->GetMode() == graphics::CameraMode::Free) {
        float zoom = m_camera->GetZoom();
        float moveSpeed = (350.0f / (zoom > 0.1f ? zoom : 1.0f)) * dt;
        Vector2 delta{ 0.0f, 0.0f };

        if (IsKeyDown(KEY_W)) delta.y -= moveSpeed;
        if (IsKeyDown(KEY_S)) delta.y += moveSpeed;
        if (IsKeyDown(KEY_A)) delta.x -= moveSpeed;
        if (IsKeyDown(KEY_D)) delta.x += moveSpeed;

        if (delta.x != 0.0f || delta.y != 0.0f) {
            m_camera->Move(delta);
        }
    }

    float zoomSpeed = 1.2f * dt;
    if (IsKeyDown(KEY_Q)) {
        m_camera->ZoomBy(-zoomSpeed);
    }
    if (IsKeyDown(KEY_E)) {
        m_camera->ZoomBy(zoomSpeed);
    }
}

void Game::FixedUpdate(float /*fixedDt*/) {
}

void Game::Update(float dt) {
    m_testScene->Update(dt);
    m_camera->Update(dt);
}

void Game::Render(float dt) {
    float fps = static_cast<float>(GetFPS());

    // 1. Begin virtual resolution frame
    m_renderer->BeginFrame();

    // 2. Render World Layer with Camera and AssetManager sprites
    m_renderer->BeginWorld(*m_camera);
    m_testScene->RenderWorld(*m_renderer, *m_camera, m_renderer->GetDrawContext(), m_assetManager.get());
    m_renderer->EndWorld(*m_camera);

    // 3. Render In-Game UI Layer
    m_renderer->BeginUI();
    m_testScene->RenderUI(m_renderer->GetDrawContext(), *m_camera, m_renderer->GetViewport(), m_assetManager.get());
    m_renderer->EndUI();

    // 4. End virtual canvas render target
    m_renderer->EndFrame();

    // 5. Present scaled canvas to window
    m_renderer->Present();

    // 6. Developer overlay (Dear ImGui with Asset Browser & Validation)
    if (m_config.debugMode && m_devUI && m_devUI->IsVisible()) {
        Vector2 mouseScreen = GetMousePosition();
        Vector2 mouseVirtual = m_renderer->GetViewport().ScreenToVirtual(mouseScreen);
        Vector2 mouseWorld = m_camera->ScreenToWorld(mouseScreen, m_renderer->GetViewport());

        m_devUI->BeginFrame();
        m_devUI->RenderDebugOverlay(
            m_config,
            fps,
            dt,
            *m_camera,
            m_renderer->GetViewport(),
            mouseScreen,
            mouseVirtual,
            mouseWorld,
            m_assetManager.get()
        );
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

    if (m_assetManager) {
        m_assetManager->Shutdown();
    }

    if (m_renderer) {
        m_renderer->Shutdown();
    }

    if (m_luaManager) {
        m_luaManager->Shutdown();
    }

    Logger::Info("ENGINE", "Engine shutdown completed cleanly.");
}

} // namespace wildbound::core
