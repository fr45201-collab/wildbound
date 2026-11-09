#pragma once

#include "EngineConfig.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/Camera2D.hpp"
#include "../graphics/TestScene.hpp"
#include "../resources/AssetManager.hpp"
#include "../world/World.hpp"
#include "../world/WorldRenderer.hpp"
#include "../world/EnemySpawner.hpp"
#include "../world/BuildingManager.hpp"
#include "../world/BiomeManager.hpp"
#include "../world/WorldManager.hpp"
#include "../weapons/WeaponManager.hpp"
#include "../items/ItemManager.hpp"
#include "../world/environment/EnvironmentalAnimationSystem.hpp"
#include "../entities/Player.hpp"
#include "../combat/CombatSystem.hpp"
#include "../physics/CollisionManager.hpp"
#include "../physics/CollisionDebugRenderer.hpp"
#include <memory>

namespace wildbound::scripting {
    class LuaManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::input {
    class InputManager;
}

namespace wildbound::interaction {
    class InteractionManager;
}

namespace wildbound::ui {
    class DevUI;
    class UIManager;
}

namespace wildbound::customization {
    class CharacterCustomizationManager;
}

namespace wildbound::cosmetics {
    class CosmeticsStoreManager;
}

namespace wildbound::ai {
    class AIManager;
}

namespace wildbound::storm {
    class StormManager;
}

namespace wildbound::match {
    class MatchManager;
}

namespace wildbound::gamemodes {
    class GameModeManager;
}

namespace wildbound::network {
    class NetworkManager;
}

namespace wildbound::network::http {
    class BackendClient;
}

namespace wildbound::network::matchmaking {
    class MatchmakingConnector;
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
    entities::Player& GetPlayer() { return *m_player; }
    combat::CombatSystem& GetCombatSystem() { return *m_combatSystem; }
    world::EnemySpawner& GetEnemySpawner() { return *m_enemySpawner; }
    world::BuildingManager& GetBuildingManager() { return *m_buildingManager; }
    const world::BuildingManager& GetBuildingManager() const { return *m_buildingManager; }
    world::BiomeManager& GetBiomeManager() { return *m_biomeManager; }
    const world::BiomeManager& GetBiomeManager() const { return *m_biomeManager; }
    world::WorldManager& GetWorldManager() { return *m_worldManager; }
    const world::WorldManager& GetWorldManager() const { return *m_worldManager; }
    weapons::WeaponManager& GetWeaponManager() { return *m_weaponManager; }
    const weapons::WeaponManager& GetWeaponManager() const { return *m_weaponManager; }
    items::ItemManager& GetItemManager() { return *m_itemManager; }
    const items::ItemManager& GetItemManager() const { return *m_itemManager; }
    audio::AudioSystem& GetAudioSystem() { return *m_audioSystem; }
    const audio::AudioSystem& GetAudioSystem() const { return *m_audioSystem; }
    ui::UIManager& GetUIManager() { return *m_uiManager; }
    const ui::UIManager& GetUIManager() const { return *m_uiManager; }
    world::environment::EnvironmentalAnimationSystem& GetEnvAnimationSystem() { return *m_envAnimationSystem; }
    physics::CollisionManager& GetCollisionManager() { return *m_collisionManager; }
    const physics::CollisionManager& GetCollisionManager() const { return *m_collisionManager; }
    input::InputManager& GetInputManager() { return *m_inputManager; }
    const input::InputManager& GetInputManager() const { return *m_inputManager; }
    interaction::InteractionManager& GetInteractionManager() { return *m_interactionManager; }
    const interaction::InteractionManager& GetInteractionManager() const { return *m_interactionManager; }
    scripting::LuaManager& GetScripting() { return *m_luaManager; }
    customization::CharacterCustomizationManager& GetCustomizationManager() { return *m_customizationManager; }
    const customization::CharacterCustomizationManager& GetCustomizationManager() const { return *m_customizationManager; }
    cosmetics::CosmeticsStoreManager& GetCosmeticsStoreManager() { return *m_cosmeticsStoreManager; }
    const cosmetics::CosmeticsStoreManager& GetCosmeticsStoreManager() const { return *m_cosmeticsStoreManager; }
    ai::AIManager& GetAIManager() { return *m_aiManager; }
    const ai::AIManager& GetAIManager() const { return *m_aiManager; }
    storm::StormManager& GetStormManager() { return *m_stormManager; }
    const storm::StormManager& GetStormManager() const { return *m_stormManager; }
    match::MatchManager& GetMatchManager() { return *m_matchManager; }
    const match::MatchManager& GetMatchManager() const { return *m_matchManager; }
    gamemodes::GameModeManager& GetGameModeManager() { return *m_gameModeManager; }
    const gamemodes::GameModeManager& GetGameModeManager() const { return *m_gameModeManager; }
    network::NetworkManager* GetNetworkManager() noexcept { return m_networkManager.get(); }
    const network::NetworkManager* GetNetworkManager() const noexcept { return m_networkManager.get(); }
    network::http::BackendClient* GetBackendClient() noexcept { return m_backendClient.get(); }
    const network::http::BackendClient* GetBackendClient() const noexcept { return m_backendClient.get(); }
    network::matchmaking::MatchmakingConnector* GetMatchmakingConnector() noexcept { return m_matchmakingConnector.get(); }
    const network::matchmaking::MatchmakingConnector* GetMatchmakingConnector() const noexcept { return m_matchmakingConnector.get(); }

    void StartSelectedGameMode();

    void ToggleCombatDebug() { m_showCombatDebug = !m_showCombatDebug; }
    bool IsCombatDebug() const { return m_showCombatDebug; }

private:
    void ProcessInput(float dt);
    void FixedUpdate(float fixedDt);
    void Update(float dt);
    void Render(float dt);

    EngineConfig m_config;
    bool m_running = false;
    bool m_isShutdown = false;
    float m_runningTime = 0.0f;

    std::unique_ptr<graphics::Renderer> m_renderer;
    std::unique_ptr<graphics::Camera2DSystem> m_camera;
    std::unique_ptr<graphics::TestScene> m_testScene;
    std::unique_ptr<resources::AssetManager> m_assetManager;
    std::unique_ptr<world::World> m_world;
    std::unique_ptr<world::WorldRenderer> m_worldRenderer;
    std::unique_ptr<entities::Player> m_player;
    std::unique_ptr<combat::CombatSystem> m_combatSystem;
    std::unique_ptr<world::EnemySpawner> m_enemySpawner;
    std::unique_ptr<world::BuildingManager> m_buildingManager;
    std::unique_ptr<world::BiomeManager> m_biomeManager;
    std::unique_ptr<world::WorldManager> m_worldManager;
    std::unique_ptr<weapons::WeaponManager> m_weaponManager;
    std::unique_ptr<items::ItemManager> m_itemManager;
    std::unique_ptr<audio::AudioSystem> m_audioSystem;
    std::unique_ptr<ui::UIManager> m_uiManager;
    std::unique_ptr<world::environment::EnvironmentalAnimationSystem> m_envAnimationSystem;
    std::unique_ptr<physics::CollisionManager> m_collisionManager;
    std::unique_ptr<physics::CollisionDebugRenderer> m_collisionDebugRenderer;
    std::unique_ptr<input::InputManager> m_inputManager;
    std::unique_ptr<interaction::InteractionManager> m_interactionManager;
    std::unique_ptr<ai::AIManager> m_aiManager;
    std::unique_ptr<storm::StormManager> m_stormManager;
    std::unique_ptr<match::MatchManager> m_matchManager;
    std::unique_ptr<gamemodes::GameModeManager> m_gameModeManager;
    std::unique_ptr<network::NetworkManager> m_networkManager;
    std::unique_ptr<network::http::BackendClient> m_backendClient;
    std::unique_ptr<network::matchmaking::MatchmakingConnector> m_matchmakingConnector;
    std::unique_ptr<customization::CharacterCustomizationManager> m_customizationManager;
    std::unique_ptr<cosmetics::CosmeticsStoreManager> m_cosmeticsStoreManager;
    std::unique_ptr<scripting::LuaManager> m_luaManager;
    std::unique_ptr<ui::DevUI> m_devUI;
    bool m_showCombatDebug = false;

    // Persistent scratch vector for rendering to eliminate per-frame allocations
    std::vector<entities::Entity*> m_cachedRenderEntities;
};

} // namespace wildbound::core
