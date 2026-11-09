#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "UITheme.hpp"
#include "UINotification.hpp"
#include "hud/HUDManager.hpp"
#include "menus/SettingsMenu.hpp"
#include "menus/PauseMenu.hpp"
#include "menus/MainMenu.hpp"
#include "menus/ModeSelectMenu.hpp"
#include "menus/AuthMenu.hpp"
#include "menus/MultiplayerMenu.hpp"
#include "inventory/InventoryUI.hpp"

namespace wildbound::network::matchmaking {
    class MatchmakingConnector;
}

namespace wildbound::gamemodes {
    class GameModeManager;
}

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::interaction {
    struct InteractionPrompt;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::input {
    class InputManager;
}

namespace wildbound::world {
    class World;
}

namespace wildbound::customization {
    class CharacterCustomizationManager;
}

namespace wildbound::cosmetics {
    class CosmeticsStoreManager;
}

namespace wildbound::match {
    class MatchManager;
}

namespace wildbound::ui {

enum class UIScreen {
    None = 0,         // Pure gameplay HUD
    MainMenu,
    ModeSelect,
    CharacterSelect,
    Wardrobe,
    Store,
    Inventory,
    Settings,
    PauseMenu,
    MatchResults,
    GameOver,
    Auth,
    Multiplayer
};

struct UIButton {
    Rectangle bounds;
    std::string text;
    std::string iconId;
    std::string style = "primary"; // primary, secondary, destructive, confirm, tab
    bool hovered = false;
    bool pressed = false;
    bool selected = false;
    bool disabled = false;
};

/**
 * Player-Facing Game UI Management System.
 * Renders custom pixel-art UI screens and components within the 640x360 virtual canvas,
 * integrated with raylib, the Asset Management System, and AudioSystem.
 */
class UIManager {
public:
    UIManager();
    ~UIManager();

    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    bool Initialize(resources::AssetManager& assetMgr, audio::AudioSystem* audioSys = nullptr);
    void Shutdown();

    void Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed, bool mouseLeftDown, const input::InputManager* inputMgr = nullptr);
    void Update(float dt, const input::InputManager* inputMgr);
    void Render(graphics::Renderer& renderer,
                resources::AssetManager& assetMgr,
                entities::Player* player = nullptr,
                const items::ItemManager* itemMgr = nullptr,
                const interaction::InteractionPrompt* prompt = nullptr,
                world::World* world = nullptr,
                customization::CharacterCustomizationManager* customMgr = nullptr,
                cosmetics::CosmeticsStoreManager* storeMgr = nullptr);

    void SetInteractionPrompt(const interaction::InteractionPrompt* prompt) { m_interactionPrompt = prompt; }
    [[nodiscard]] const interaction::InteractionPrompt* GetInteractionPrompt() const { return m_interactionPrompt; }

    void SetScreen(UIScreen screen);
    [[nodiscard]] UIScreen GetCurrentScreen() const { return m_currentScreen; }
    [[nodiscard]] UIScreen GetPreviousScreen() const { return m_previousScreen; }
    [[nodiscard]] bool IsScreenActive() const { return m_currentScreen != UIScreen::None; }

    // Subsystem Getters
    [[nodiscard]] HUDManager& GetHUDManager() { return m_hudManager; }
    [[nodiscard]] const HUDManager& GetHUDManager() const { return m_hudManager; }
    [[nodiscard]] SettingsMenu& GetSettingsMenu() { return m_settingsMenu; }
    [[nodiscard]] const SettingsMenu& GetSettingsMenu() const { return m_settingsMenu; }
    [[nodiscard]] PauseMenu& GetPauseMenu() { return m_pauseMenu; }
    [[nodiscard]] const PauseMenu& GetPauseMenu() const { return m_pauseMenu; }
    [[nodiscard]] MainMenu& GetMainMenu() { return m_mainMenu; }
    [[nodiscard]] const MainMenu& GetMainMenu() const { return m_mainMenu; }
    [[nodiscard]] InventoryUI& GetInventoryUI() { return m_inventoryUI; }
    [[nodiscard]] const InventoryUI& GetInventoryUI() const { return m_inventoryUI; }
    [[nodiscard]] ModeSelectMenu& GetModeSelectMenu() { return m_modeSelectMenu; }
    [[nodiscard]] const ModeSelectMenu& GetModeSelectMenu() const { return m_modeSelectMenu; }
    [[nodiscard]] AuthMenu& GetAuthMenu() { return m_authMenu; }
    [[nodiscard]] const AuthMenu& GetAuthMenu() const { return m_authMenu; }
    [[nodiscard]] MultiplayerMenu& GetMultiplayerMenu() noexcept { return m_multiplayerMenu; }
    [[nodiscard]] const MultiplayerMenu& GetMultiplayerMenu() const noexcept { return m_multiplayerMenu; }

    void SetBackendClient(network::http::BackendClient* client);
    void OpenAuthMenu(AuthMenuMode mode = AuthMenuMode::Login);
    void SetMatchmakingConnector(network::matchmaking::MatchmakingConnector* connector);
    void OpenMultiplayerMenu();

    void SetGameModeManager(gamemodes::GameModeManager* modeMgr) noexcept { m_gameModeManager = modeMgr; }
    [[nodiscard]] gamemodes::GameModeManager* GetGameModeManager() const noexcept { return m_gameModeManager; }

    using MatchStartCallback = std::function<void()>;
    void SetMatchStartCallback(MatchStartCallback cb) { m_matchStartCallback = std::move(cb); }

    // Reusable UI Component Primitives
    void DrawPanel(Rectangle bounds, const std::string& style = "main");
    bool DrawButton(Rectangle bounds, const std::string& text, const std::string& style = "primary", bool selected = false);
    void DrawProgressBar(Rectangle bounds, float progress, Color fillColor, Color bgColor = Color{ 15, 18, 25, 255 });
    void DrawIcon(const std::string& iconId, Vector2 pos, float size = 24.0f, Color tint = WHITE);

    // Audio integration
    void SetAudioSystem(audio::AudioSystem* audioSys) { m_audioSystem = audioSys; }
    void SetMatchManager(match::MatchManager* matchMgr) { m_matchManager = matchMgr; }

private:
    void RenderMainMenu(graphics::Renderer& renderer, resources::AssetManager& assetMgr);
    void RenderCharacterSelect(graphics::Renderer& renderer, resources::AssetManager& assetMgr, entities::Player* player, customization::CharacterCustomizationManager* customMgr = nullptr);
    void RenderWardrobe(graphics::Renderer& renderer, resources::AssetManager& assetMgr, entities::Player* player, customization::CharacterCustomizationManager* customMgr = nullptr, cosmetics::CosmeticsStoreManager* storeMgr = nullptr);
    void RenderStore(graphics::Renderer& renderer, resources::AssetManager& assetMgr, entities::Player* player, customization::CharacterCustomizationManager* customMgr = nullptr, cosmetics::CosmeticsStoreManager* storeMgr = nullptr);
    void RenderInventory(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const items::ItemManager* itemMgr, entities::Player* player = nullptr, world::World* world = nullptr);
    void RenderSettings(graphics::Renderer& renderer, resources::AssetManager& assetMgr, input::InputManager* inputMgr = nullptr);
    void RenderPauseMenu(graphics::Renderer& renderer, resources::AssetManager& assetMgr);
    void RenderGameplayHUD(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const entities::Player* player, const interaction::InteractionPrompt* prompt = nullptr);
    void RenderMatchResults(graphics::Renderer& renderer, resources::AssetManager& assetMgr);
    void RenderGameOver(graphics::Renderer& renderer, resources::AssetManager& assetMgr);

    bool CheckHover(Rectangle r) const;

    bool m_initialized = false;
    UIScreen m_currentScreen = UIScreen::None;
    UIScreen m_previousScreen = UIScreen::MainMenu;

    audio::AudioSystem* m_audioSystem = nullptr;

    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed = false;
    bool m_mouseDown = false;

    // Subsystem instances
    HUDManager m_hudManager;
    SettingsMenu m_settingsMenu;
    PauseMenu m_pauseMenu;
    MainMenu m_mainMenu;
    ModeSelectMenu m_modeSelectMenu;
    AuthMenu m_authMenu;
    MultiplayerMenu m_multiplayerMenu;
    InventoryUI m_inventoryUI;

    network::matchmaking::MatchmakingConnector* m_matchmakingConnector{ nullptr };

    // Submenu state
    int m_wardrobeCategoryTab = 0; // 0=Hair, 1=Headwear, 2=Helmet, 3=Face, 4=Chest, 5=Legs, 6=Feet, 7=Back, 8=Skin, 9=Loadouts
    int m_wardrobeScrollOffset = 0;
    bool m_wardrobePreviewInitialized = false;
    int m_inventoryCategoryTab = 0; // 0=All, 1=Collectibles, 2=Consumables, 3=Materials, 4=Resources, 5=Special
    std::string m_selectedInventoryItemId;
    int m_selectedInventorySlot = -1;
    int m_selectedCharacterIndex = 0;

    // Results screen navigation & tabs (Phase 33)
    int m_resultsSelectedButton = 0; // 0 = PLAY AGAIN, 1 = MAIN MENU
    int m_resultsActiveTab = 0;      // 0 = Summary & Performance, 1 = Leaderboard
    float m_resultsEntranceTimer = 0.0f;

    // Cached gameplay system references
    entities::Player* m_lastPlayer = nullptr;
    const items::ItemManager* m_lastItemMgr = nullptr;
    world::World* m_lastWorld = nullptr;

    // Hover sound tracking
    std::string m_lastHoveredId;
    const interaction::InteractionPrompt* m_interactionPrompt = nullptr;
    match::MatchManager* m_matchManager = nullptr;
    gamemodes::GameModeManager* m_gameModeManager = nullptr;
    MatchStartCallback m_matchStartCallback;
};

} // namespace wildbound::ui

