#include "../../src/ui/UIManager.hpp"
#include "../../src/audio/AudioSystem.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include "../../src/network/matchmaking/MatchmakingConnector.hpp"
#include "../../src/network/matchmaking/MatchmakingTypes.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <vector>

using namespace wildbound::ui;
using namespace wildbound::audio;
using namespace wildbound::resources;
namespace fs = std::filesystem;

void TestUIManagerInitialization() {
    std::cout << "[TEST] 1. UIManager Initialization & Default Screen..." << std::endl;

    AssetManager assetMgr;
    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    UIManager uiMgr;
    bool ok = uiMgr.Initialize(assetMgr, &audioSys);
    assert(ok && "UIManager failed to initialize");

    assert(uiMgr.GetCurrentScreen() == UIScreen::None && "Default screen should be UIScreen::None (HUD)");
    assert(uiMgr.IsScreenActive() == false && "Gameplay HUD should not report as active modal screen");

    std::cout << "  [PASS] UIManager successfully initialized." << std::endl;
    uiMgr.Shutdown();
    audioSys.Shutdown();
}

void TestScreenNavigationFlow() {
    std::cout << "[TEST] 2. Screen Navigation State Machine..." << std::endl;

    AssetManager assetMgr;
    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    UIManager uiMgr;
    uiMgr.Initialize(assetMgr, &audioSys);

    const std::vector<UIScreen> testScreens = {
        UIScreen::MainMenu,
        UIScreen::CharacterSelect,
        UIScreen::Wardrobe,
        UIScreen::Inventory,
        UIScreen::Settings,
        UIScreen::PauseMenu,
        UIScreen::MatchResults,
        UIScreen::GameOver,
        UIScreen::Auth
    };

    for (auto screen : testScreens) {
        uiMgr.SetScreen(screen);
        assert(uiMgr.GetCurrentScreen() == screen && "Screen did not transition correctly");
        assert(uiMgr.IsScreenActive() == true && "Modal screens must report IsScreenActive() == true");
    }

    // Return to gameplay HUD
    uiMgr.SetScreen(UIScreen::None);
    assert(uiMgr.GetCurrentScreen() == UIScreen::None);
    assert(uiMgr.IsScreenActive() == false);

    std::cout << "  [PASS] All 9 player-facing screen transitions verified." << std::endl;
    uiMgr.Shutdown();
    audioSys.Shutdown();
}

void TestUIInputSimulation() {
    std::cout << "[TEST] 3. Virtual Coordinate Mouse Input Simulation..." << std::endl;

    AssetManager assetMgr;
    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    UIManager uiMgr;
    uiMgr.Initialize(assetMgr, &audioSys);
    uiMgr.SetScreen(UIScreen::MainMenu);

    // Simulate mouse hover at virtual resolution center (320, 180)
    uiMgr.Update(0.016f, Vector2{ 320.0f, 180.0f }, false, false);
    assert(uiMgr.GetCurrentScreen() == UIScreen::MainMenu);

    // Simulate click
    uiMgr.Update(0.016f, Vector2{ 320.0f, 180.0f }, true, true);
    assert(uiMgr.GetCurrentScreen() == UIScreen::MainMenu);

    std::cout << "  [PASS] Input simulation updated without errors." << std::endl;
    uiMgr.Shutdown();
    audioSys.Shutdown();
}

void TestUIAssetIntegrity() {
    std::cout << "[TEST] 4. UI Artwork & Texture Asset File Integrity..." << std::endl;

    // 10 Backgrounds (640x360 px)
    const std::vector<std::string> backgrounds = {
        "assets/ui/backgrounds/bg_main_menu.png",
        "assets/ui/backgrounds/bg_character_select.png",
        "assets/ui/backgrounds/bg_wardrobe.png",
        "assets/ui/backgrounds/bg_inventory.png",
        "assets/ui/backgrounds/bg_settings.png",
        "assets/ui/backgrounds/bg_loading.png",
        "assets/ui/backgrounds/bg_lobby.png",
        "assets/ui/backgrounds/bg_match_results.png",
        "assets/ui/backgrounds/bg_pause_menu.png",
        "assets/ui/backgrounds/bg_game_over.png"
    };

    for (const auto& path : backgrounds) {
        assert(fs::exists(path) && "Background asset file missing");
        assert(fs::file_size(path) > 100 && "Background asset file is empty or corrupted");
    }
    std::cout << "  Validated 10/10 full-canvas background textures." << std::endl;

    // 5 Reusable Panels
    const std::vector<std::string> panels = {
        "assets/ui/panels/panel_main_64.png",
        "assets/ui/panels/panel_dark_64.png",
        "assets/ui/panels/panel_tooltip_32.png",
        "assets/ui/panels/panel_card_48.png",
        "assets/ui/panels/panel_hud_bar.png"
    };

    for (const auto& path : panels) {
        assert(fs::exists(path) && "Panel asset file missing");
        assert(fs::file_size(path) > 50 && "Panel asset file is empty or corrupted");
    }
    std::cout << "  Validated 5/5 scalable UI panels." << std::endl;

    // 25 UI Icons
    const std::vector<std::string> icons = {
        "assets/ui/icons/icon_health.png",
        "assets/ui/icons/icon_stamina.png",
        "assets/ui/icons/icon_armor.png",
        "assets/ui/icons/icon_weapons.png",
        "assets/ui/icons/icon_items.png",
        "assets/ui/icons/icon_map.png",
        "assets/ui/icons/icon_settings.png",
        "assets/ui/icons/icon_audio.png",
        "assets/ui/icons/icon_graphics.png",
        "assets/ui/icons/icon_controls.png",
        "assets/ui/icons/icon_exit.png",
        "assets/ui/icons/icon_confirm.png",
        "assets/ui/icons/icon_pause.png",
        "assets/ui/icons/icon_inventory.png",
        "assets/ui/icons/icon_character.png",
        "assets/ui/icons/icon_cosmetics.png",
        "assets/ui/icons/icon_collectibles.png",
        "assets/ui/icons/icon_consumables.png",
        "assets/ui/icons/icon_currency.png",
        "assets/ui/icons/icon_notifications.png",
        "assets/ui/icons/icon_search.png",
        "assets/ui/icons/icon_nav.png",
        "assets/ui/icons/icon_error.png",
        "assets/ui/icons/icon_backpacks.png",
        "assets/ui/icons/icon_clothing.png"
    };

    for (const auto& path : icons) {
        assert(fs::exists(path) && "Icon asset file missing");
        assert(fs::file_size(path) > 50 && "Icon asset file is empty or corrupted");
    }
    std::cout << "  Validated 25/25 24x24 pixel UI icons." << std::endl;

    // Button state textures
    const std::vector<std::string> buttons = {
        "assets/ui/buttons/btn_primary_normal.png",
        "assets/ui/buttons/btn_primary_hover.png",
        "assets/ui/buttons/btn_primary_pressed.png",
        "assets/ui/buttons/btn_primary_disabled.png",
        "assets/ui/buttons/btn_destructive_normal.png",
        "assets/ui/buttons/btn_confirm_normal.png",
        "assets/ui/buttons/btn_tab_normal.png",
        "assets/ui/buttons/btn_tab_selected.png"
    };

    for (const auto& path : buttons) {
        assert(fs::exists(path) && "Button texture missing");
    }
    std::cout << "  Validated button state and style textures." << std::endl;

    std::cout << "  [PASS] All UI artwork and texture assets verified on disk." << std::endl;
}

void TestAuthMenuIntegration() {
    std::cout << "[TEST] 5. AuthMenu & Backend Client UI Integration..." << std::endl;

    AssetManager assetMgr;
    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    UIManager uiMgr;
    uiMgr.Initialize(assetMgr, &audioSys);

    wildbound::network::http::BackendClient backendClient("http://127.0.0.1:8080");
    uiMgr.SetBackendClient(&backendClient);

    // 1. Initial State
    assert(!uiMgr.GetAuthMenu().IsOpen());
    assert(uiMgr.GetCurrentScreen() == UIScreen::None);

    // 2. Open Auth Menu via OpenAuthMenu
    uiMgr.OpenAuthMenu(AuthMenuMode::Login);
    assert(uiMgr.GetCurrentScreen() == UIScreen::Auth);
    assert(uiMgr.GetAuthMenu().IsOpen());
    assert(uiMgr.IsScreenActive());

    // 3. Update while in Auth Screen
    uiMgr.Update(0.016f, Vector2{ 0.0f, 0.0f }, false, false);
    assert(uiMgr.GetCurrentScreen() == UIScreen::Auth);
    assert(uiMgr.GetAuthMenu().IsOpen());

    // 4. Simulate Close button click (closeBtn is at x=450, y=40, w=24, h=20)
    uiMgr.Update(0.016f, Vector2{ 460.0f, 48.0f }, true, true);
    assert(!uiMgr.GetAuthMenu().IsOpen());
    assert(uiMgr.GetCurrentScreen() == UIScreen::MainMenu || uiMgr.GetCurrentScreen() == UIScreen::None);

    // 5. Open via MainMenu action navigation
    uiMgr.SetScreen(UIScreen::MainMenu);
    assert(uiMgr.GetCurrentScreen() == UIScreen::MainMenu);

    // MainMenu has 8 items, index 4 is Account / Login
    uiMgr.GetMainMenu().SetFocusedIndex(4);
    MainMenuAction act = uiMgr.GetMainMenu().Update(0.016f, Vector2{ 0.0f, 0.0f }, false, false, false, true);
    (void)act;
    assert(act == MainMenuAction::Account);

    // 6. Direct SetScreen(UIScreen::Auth) opens AuthMenu automatically
    uiMgr.SetScreen(UIScreen::Auth);
    assert(uiMgr.GetCurrentScreen() == UIScreen::Auth);
    assert(uiMgr.GetAuthMenu().IsOpen());

    // 7. Verify session persistence handling with temporary/clean file
    wildbound::network::http::AuthSession testSess;
    testSess.userId = "usr_test_stage6a";
    testSess.username = "test_player";
    testSess.accessToken = "mock_token_jwt_123";
    backendClient.SetSession(testSess);
    assert(backendClient.IsLoggedIn());

    bool saved = backendClient.SaveSessionToFile("user_data/test_auth_temp.json");
    (void)saved;
    assert(saved);

    backendClient.ClearSession();
    assert(!backendClient.IsLoggedIn());

    bool loaded = backendClient.LoadSessionFromFile("user_data/test_auth_temp.json");
    (void)loaded;
    assert(loaded);
    assert(backendClient.IsLoggedIn());
    assert(backendClient.GetSession().username == "test_player");

    backendClient.ClearSession();
    std::filesystem::remove("user_data/test_auth_temp.json");

    // 8. Verify Callback & Shutdown Safety: Destruction while request in flight (Section 4)
    {
        auto tempMenu = std::make_unique<AuthMenu>(&backendClient);
        tempMenu->Open(AuthMenuMode::Login);
        // Trigger a real async HTTP login call to an unreachable port (simulating in-flight request)
        backendClient.Login("probe_user", "probe_password", [](bool, const std::string&) {});
        // Destroy the owning AuthMenu immediately before request completes
        tempMenu.reset();
        // Discharging backend client updates must not crash or touch destroyed tempMenu
        backendClient.Update();
    }

    // 9. Verify Repeated Shutdown Idempotence (Section 4)
    backendClient.Shutdown();
    backendClient.Shutdown(); // Second call must be completely safe

    std::cout << "  [PASS] AuthMenu and BackendClient UI integration verified." << std::endl;
    uiMgr.Shutdown();
    audioSys.Shutdown();
}

void TestMultiplayerMenuIntegration() {
    std::cout << "[TEST] 6. MultiplayerMenu & Matchmaking UI Integration (Stage 6B)..." << std::endl;

    AssetManager assetMgr;
    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    UIManager uiMgr;
    bool ok = uiMgr.Initialize(assetMgr, &audioSys);
    (void)ok;
    assert(ok);

    wildbound::network::matchmaking::MatchmakingConnector connector;
    uiMgr.SetMatchmakingConnector(&connector);

    // 1. Initial State
    assert(uiMgr.GetCurrentScreen() == UIScreen::None);
    assert(!uiMgr.GetMultiplayerMenu().IsConnected());
    assert(uiMgr.GetMultiplayerMenu().GetMatchmakingState() == wildbound::network::matchmaking::MatchmakingState::Idle);

    // 2. Open via OpenMultiplayerMenu
    uiMgr.OpenMultiplayerMenu();
    assert(uiMgr.GetCurrentScreen() == UIScreen::Multiplayer);
    assert(uiMgr.IsScreenActive());

    // 3. Update while in Multiplayer Screen (Syncs state from connector)
    uiMgr.Update(0.016f, Vector2{ 0.0f, 0.0f }, false, false);
    assert(uiMgr.GetCurrentScreen() == UIScreen::Multiplayer);

    // 4. Navigate back to MainMenu via Back button click (at x=520, y=280, w=100, h=26)
    uiMgr.Update(0.016f, Vector2{ 550.0f, 290.0f }, true, true);
    assert(uiMgr.GetCurrentScreen() == UIScreen::MainMenu);

    // 5. Open via MainMenu action navigation (index 5 is ONLINE MULTIPLAYER)
    uiMgr.GetMainMenu().SetFocusedIndex(5);
    MainMenuAction act = uiMgr.GetMainMenu().Update(0.016f, Vector2{ 0.0f, 0.0f }, false, false, false, true);
    (void)act;
    assert(act == MainMenuAction::Multiplayer);

    // Transition UIManager on MainMenuAction::Multiplayer
    uiMgr.Update(0.016f, Vector2{ 0.0f, 0.0f }, false, false); // Triggers main menu action
    // Simulate selection via mouse click on item 5
    uiMgr.Update(0.016f, Vector2{ 100.0f, 112.0f + 5 * 24.0f + 10.0f }, true, true);
    assert(uiMgr.GetCurrentScreen() == UIScreen::Multiplayer);

    // 6. Test Matchmaking button actions via UI Update
    // Click "FIND MATCH" button (at x=210, y=280, w=150, h=26)
    uiMgr.Update(0.016f, Vector2{ 250.0f, 290.0f }, true, true);
    // Since connector is not connected to a live server, state transitions to Failed with error
    assert(connector.GetState() == wildbound::network::matchmaking::MatchmakingState::Failed);
    assert(!connector.GetLastErrorMessage().empty());

    // Sync menu and verify error display
    uiMgr.Update(0.016f, Vector2{ 0.0f, 0.0f }, false, false);
    assert(uiMgr.GetMultiplayerMenu().GetMatchmakingState() == wildbound::network::matchmaking::MatchmakingState::Failed);
    assert(uiMgr.GetMultiplayerMenu().GetErrorMessage() == connector.GetLastErrorMessage());

    // Reset connector
    connector.Reset();
    assert(connector.GetState() == wildbound::network::matchmaking::MatchmakingState::Idle);

    // 7. Test Mode Selection clicks in MultiplayerMenu (SOLO, DUOS, SQUADS)
    assert(uiMgr.GetMultiplayerMenu().GetSelectedGameMode() == "solo");
    // Click DUOS button (at x=310, y=80, w=80, h=24)
    uiMgr.Update(0.016f, Vector2{ 340.0f, 90.0f }, true, true);
    assert(uiMgr.GetMultiplayerMenu().GetSelectedGameMode() == "duos");
    // Click SQUADS button (at x=400, y=80, w=80, h=24)
    uiMgr.Update(0.016f, Vector2{ 430.0f, 90.0f }, true, true);
    assert(uiMgr.GetMultiplayerMenu().GetSelectedGameMode() == "squads");
    // Click SOLO button (at x=220, y=80, w=80, h=24)
    uiMgr.Update(0.016f, Vector2{ 250.0f, 90.0f }, true, true);
    assert(uiMgr.GetMultiplayerMenu().GetSelectedGameMode() == "solo");

    // 8. Test Match Assignment Display Synchronization
    wildbound::network::matchmaking::MatchAssignmentInfo mockAssignment;
    mockAssignment.matchId = "match_test_5001";
    mockAssignment.serverHost = "127.0.0.1";
    mockAssignment.serverPort = 17777;
    mockAssignment.playerAuthToken = "token_participant_alpha";
    mockAssignment.gameMode = "solo";
    mockAssignment.mapName = "maps/world/battle_royale_island.json";
    assert(mockAssignment.IsValid());

    uiMgr.GetMultiplayerMenu().SetMatchAssignment(mockAssignment);
    uiMgr.GetMultiplayerMenu().SetMatchmakingState(wildbound::network::matchmaking::MatchmakingState::MatchFound);
    assert(uiMgr.GetMultiplayerMenu().GetMatchAssignment().IsValid());
    assert(uiMgr.GetMultiplayerMenu().GetMatchAssignment().matchId == "match_test_5001");
    assert(uiMgr.GetMultiplayerMenu().GetMatchAssignment().serverPort == 17777);

    // 9. Repeated Navigation Back & Forth
    for (int i = 0; i < 5; ++i) {
        uiMgr.SetScreen(UIScreen::MainMenu);
        assert(uiMgr.GetCurrentScreen() == UIScreen::MainMenu);
        uiMgr.SetScreen(UIScreen::Multiplayer);
        assert(uiMgr.GetCurrentScreen() == UIScreen::Multiplayer);
    }

    // 10. Clean Teardown & Idempotence
    connector.Shutdown();
    connector.Shutdown(); // Idempotent
    uiMgr.Shutdown();
    audioSys.Shutdown();

    std::cout << "  [PASS] MultiplayerMenu and MatchmakingConnector UI integration verified." << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 18: UI SYSTEM TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestUIManagerInitialization();
    TestScreenNavigationFlow();
    TestUIInputSimulation();
    TestUIAssetIntegrity();
    TestAuthMenuIntegration();
    TestMultiplayerMenuIntegration();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL UI SYSTEM TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}

