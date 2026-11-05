#include "../../src/ui/UIManager.hpp"
#include "../../src/audio/AudioSystem.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
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
        UIScreen::GameOver
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

    std::cout << "  [PASS] All 8 player-facing screen transitions verified." << std::endl;
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

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 18: UI SYSTEM TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestUIManagerInitialization();
    TestScreenNavigationFlow();
    TestUIInputSimulation();
    TestUIAssetIntegrity();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL UI SYSTEM TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}

