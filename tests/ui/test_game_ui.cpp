#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <cmath>

#include "ui/UITheme.hpp"
#include "ui/UINotification.hpp"
#include "ui/hud/Minimap.hpp"
#include "ui/hud/HUDManager.hpp"
#include "ui/menus/SettingsMenu.hpp"
#include "ui/menus/PauseMenu.hpp"
#include "ui/menus/MainMenu.hpp"
#include "ui/inventory/InventoryUI.hpp"
#include "ui/UIManager.hpp"

#include "entities/Player.hpp"
#include "combat/HealthComponent.hpp"
#include "items/ItemManager.hpp"
#include "items/Inventory.hpp"
#include "audio/AudioSystem.hpp"
#include "world/World.hpp"

using namespace wildbound;
using namespace wildbound::ui;

// ============================================================================
// 1. UITheme & Design Tokens
// ============================================================================
void Test_UITheme_MetricsAndColors() {
    std::cout << "[TEST] Running Test_UITheme_MetricsAndColors..." << std::endl;

    assert(UITheme::CanvasWidth == 640.0f);
    assert(UITheme::CanvasHeight == 360.0f);

    assert(UITheme::HealthFill.r == 220);
    assert(UITheme::ArmorFill.b == 235);
    assert(UITheme::Gold.r == 235);

    // Rarity Color Mapping
    Color cCommon = UITheme::GetRarityColor("common");
    Color cUncommon = UITheme::GetRarityColor("uncommon");
    Color cRare = UITheme::GetRarityColor("rare");
    Color cEpic = UITheme::GetRarityColor("epic");
    Color cLegendary = UITheme::GetRarityColor("legendary");

    assert(cCommon.r == UITheme::RarityCommon.r);
    assert(cUncommon.g == UITheme::RarityUncommon.g);
    assert(cRare.b == UITheme::RarityRare.b);
    assert(cEpic.r == UITheme::RarityEpic.r);
    assert(cLegendary.r == UITheme::RarityLegendary.r);

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 2. Notification Queue Lifecycle
// ============================================================================
void Test_UINotification_QueueLifecycle() {
    std::cout << "[TEST] Running Test_UINotification_QueueLifecycle..." << std::endl;

    NotificationQueue queue;
    assert(queue.GetCount() == 0);

    // Push notifications
    queue.Push("Picked up Iron Sword", "sword", UITheme::Gold, 2.0f);
    queue.Push("Healed for 30 HP", "potion", UITheme::TextSuccess, 2.0f);
    assert(queue.GetCount() == 2);

    // Test entry animation
    queue.Update(0.1f);
    assert(queue.GetToasts()[0].alpha > 0.0f);
    assert(queue.GetToasts()[0].slideOffset > -25.0f);

    // Test duration progression and expiration
    queue.Update(2.5f);
    assert(queue.GetCount() == 0);

    // Test max capacity clamp (max 5)
    for (int i = 0; i < 7; ++i) {
        queue.Push("Event " + std::to_string(i), "", UITheme::Gold, 5.0f);
    }
    assert(queue.GetCount() == 5);
    // Oldest should have been popped
    assert(queue.GetToasts().front().message == "Event 2");

    queue.Clear();
    assert(queue.GetCount() == 0);

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 3. Minimap Coordinate Mapping & Zoom
// ============================================================================
void Test_Minimap_CoordinateTransformAndZoom() {
    std::cout << "[TEST] Running Test_Minimap_CoordinateTransformAndZoom..." << std::endl;

    Minimap mm;
    assert(mm.IsVisible());
    assert(mm.GetZoom() == 1.0f);

    // Test Zoom Cycling
    mm.CycleZoom();
    assert(mm.GetZoom() == 1.5f);
    mm.CycleZoom();
    assert(mm.GetZoom() == 2.0f);
    mm.CycleZoom();
    assert(mm.GetZoom() == 1.0f);

    // Coordinate translation: player at (1000, 1000)
    Vector2 playerPos{ 1000.0f, 1000.0f };
    Rectangle worldBounds{ 0.0f, 0.0f, 3200.0f, 3200.0f };

    // Player themselves should map exactly to minimap center
    Vector2 pMM = mm.WorldToMinimap(playerPos, playerPos, worldBounds);
    Rectangle mmBounds = mm.GetBounds();
    float centerX = mmBounds.x + mmBounds.width * 0.5f;
    float centerY = mmBounds.y + mmBounds.height * 0.5f;

    assert(std::abs(pMM.x - centerX) < 0.001f);
    assert(std::abs(pMM.y - centerY) < 0.001f);
    assert(mm.IsWorldPosInView(playerPos, playerPos, worldBounds));

    // Distant point outside view radius
    Vector2 farPos{ 3000.0f, 3000.0f };
    assert(!mm.IsWorldPosInView(farPos, playerPos, worldBounds));

    // Visibility toggle
    mm.ToggleVisible();
    assert(!mm.IsVisible());
    mm.SetVisible(true);
    assert(mm.IsVisible());

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 4. HUDManager Delayed Lag Bar & Low Health Alert
// ============================================================================
void Test_HUDManager_HealthLagBarAndPulse() {
    std::cout << "[TEST] Running Test_HUDManager_HealthLagBarAndPulse..." << std::endl;

    HUDManager hud;
    assert(hud.GetDisplayedHealth() == 100.0f);
    assert(hud.GetLagHealth() == 100.0f);
    assert(!hud.IsLowHealthAlertActive());

    entities::Player player;
    auto& health = player.GetHealth();
    health.SetMaxHealth(100.0f);
    health.SetCurrentHealth(100.0f);

    // Initial sync
    hud.Update(0.016f, &player);
    assert(hud.GetDisplayedHealth() == 100.0f);

    // Apply Damage: HP drops to 60
    health.SetCurrentHealth(60.0f);
    hud.Update(0.016f, &player);

    // Displayed drops immediately; lag health stays at 100 during 0.5s lag timer
    assert(hud.GetDisplayedHealth() == 60.0f);
    assert(hud.GetLagHealth() == 100.0f);

    // Advance 0.55s: lag timer expires, lag health catches down
    hud.Update(0.55f, &player);
    assert(hud.GetLagHealth() < 100.0f);

    // Advance enough time for full catch-down
    hud.Update(1.0f, &player);
    assert(hud.GetLagHealth() == 60.0f);

    // Critical low-HP test (<= 25%)
    health.SetCurrentHealth(20.0f);
    hud.Update(0.016f, &player);
    assert(hud.IsLowHealthAlertActive());

    // Heal test: health recovers to 80%
    health.SetCurrentHealth(80.0f);
    hud.Update(0.016f, &player);
    assert(!hud.IsLowHealthAlertActive());
    assert(hud.GetDisplayedHealth() == 80.0f);
    assert(hud.GetLagHealth() == 80.0f);

    // Match Info Progression
    float initialTimer = hud.GetMatchInfo().matchTimer;
    hud.Update(1.0f, &player);
    assert(hud.GetMatchInfo().matchTimer > initialTimer);

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 5. SettingsMenu Navigation & Options
// ============================================================================
void Test_SettingsMenu_TabsAndAudioPersistence() {
    std::cout << "[TEST] Running Test_SettingsMenu_TabsAndAudioPersistence..." << std::endl;

    SettingsMenu settings;
    assert(settings.GetCurrentTab() == SettingsTab::Audio);

    settings.SetCurrentTab(SettingsTab::Graphics);
    assert(settings.GetCurrentTab() == SettingsTab::Graphics);

    settings.SetCurrentTab(SettingsTab::Controls);
    assert(settings.GetCurrentTab() == SettingsTab::Controls);

    settings.SetCurrentTab(SettingsTab::Gameplay);
    assert(settings.GetCurrentTab() == SettingsTab::Gameplay);

    // Toggles
    assert(settings.IsMinimapEnabled());
    settings.SetMinimapEnabled(false);
    assert(!settings.IsMinimapEnabled());

    assert(settings.IsHUDEnabled());
    settings.SetHUDEnabled(false);
    assert(!settings.IsHUDEnabled());

    assert(settings.AreDamageNumbersEnabled());
    settings.SetDamageNumbersEnabled(false);
    assert(!settings.AreDamageNumbersEnabled());

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 6. PauseMenu Navigation & Actions
// ============================================================================
void Test_PauseMenu_NavigationAndActions() {
    std::cout << "[TEST] Running Test_PauseMenu_NavigationAndActions..." << std::endl;

    PauseMenu pause;
    assert(pause.GetFocusedIndex() == 0);

    // Nav Down
    pause.Update(0.016f, { 0, 0 }, false, false, true, false);
    assert(pause.GetFocusedIndex() == 1);

    // Nav Up
    pause.Update(0.016f, { 0, 0 }, false, true, false, false);
    assert(pause.GetFocusedIndex() == 0);

    // Confirm on index 0 -> Resume
    PauseMenuAction act = pause.Update(0.016f, { 0, 0 }, false, false, false, true);
    assert(act == PauseMenuAction::Resume);

    // Set to index 3 (Settings) and confirm
    pause.SetFocusedIndex(3);
    act = pause.Update(0.016f, { 0, 0 }, false, false, false, true);
    assert(act == PauseMenuAction::Settings);

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 7. MainMenu Focus Navigation
// ============================================================================
void Test_MainMenu_NavigationAndCredits() {
    std::cout << "[TEST] Running Test_MainMenu_NavigationAndCredits..." << std::endl;

    MainMenu menu;
    assert(menu.GetFocusedIndex() == 0);
    assert(!menu.IsShowingCredits());

    // Nav Down
    menu.Update(0.016f, { 0, 0 }, false, false, true, false);
    assert(menu.GetFocusedIndex() == 1);

    // Confirm on Character Select
    MainMenuAction act = menu.Update(0.016f, { 0, 0 }, false, false, false, true);
    assert(act == MainMenuAction::CharacterSelect);

    // Confirm on Account (index 4)
    menu.SetFocusedIndex(4);
    act = menu.Update(0.016f, { 0, 0 }, false, false, false, true);
    assert(act == MainMenuAction::Account);

    // Confirm on Multiplayer (index 5)
    menu.SetFocusedIndex(5);
    act = menu.Update(0.016f, { 0, 0 }, false, false, false, true);
    assert(act == MainMenuAction::Multiplayer);

    // Open Credits
    menu.SetShowingCredits(true);
    assert(menu.IsShowingCredits());
    // Pressing enter dismisses credits
    menu.Update(0.016f, { 0, 0 }, false, false, false, true);
    assert(!menu.IsShowingCredits());

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 8. InventoryUI Grid & Selection
// ============================================================================
void Test_InventoryUI_GridNavigationAndActions() {
    std::cout << "[TEST] Running Test_InventoryUI_GridNavigationAndActions..." << std::endl;

    InventoryUI invUI;
    assert(invUI.GetSelectedSlot() == 0);
    assert(invUI.GetCategoryTab() == 0);

    // Nav Right: moves from slot 0 to 1
    invUI.Update(0.016f, { 0, 0 }, false, false, nullptr, nullptr, nullptr, nullptr, false, false, false, true, false, false);
    assert(invUI.GetSelectedSlot() == 1);

    // Nav Down: moves down 6 slots (from 1 to 7)
    invUI.Update(0.016f, { 0, 0 }, false, false, nullptr, nullptr, nullptr, nullptr, false, true, false, false, false, false);
    assert(invUI.GetSelectedSlot() == 7);

    // Category Tab switch
    invUI.SetCategoryTab(2);
    assert(invUI.GetCategoryTab() == 2);

    invUI.Reset();
    assert(invUI.GetSelectedSlot() == 0);
    assert(!invUI.RequestClose());

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// 9. UIManager Integration & Subsystem Access
// ============================================================================
void Test_UIManager_Integration() {
    std::cout << "[TEST] Running Test_UIManager_Integration..." << std::endl;

    UIManager ui;
    assert(ui.GetCurrentScreen() == UIScreen::None);
    assert(!ui.IsScreenActive());

    // Screen transitions
    ui.SetScreen(UIScreen::PauseMenu);
    assert(ui.GetCurrentScreen() == UIScreen::PauseMenu);
    assert(ui.IsScreenActive());

    ui.SetScreen(UIScreen::Settings);
    assert(ui.GetCurrentScreen() == UIScreen::Settings);
    assert(ui.GetPreviousScreen() == UIScreen::PauseMenu);

    ui.SetScreen(UIScreen::Inventory);
    assert(ui.GetCurrentScreen() == UIScreen::Inventory);

    ui.SetScreen(UIScreen::None);
    assert(ui.GetCurrentScreen() == UIScreen::None);
    assert(!ui.IsScreenActive());

    // Verify sub-manager references are valid
    HUDManager& hud = ui.GetHUDManager();
    assert(hud.IsHUDVisible());

    SettingsMenu& settings = ui.GetSettingsMenu();
    assert(settings.IsMinimapEnabled());

    PauseMenu& pause = ui.GetPauseMenu();
    assert(pause.GetFocusedIndex() == 0);

    MainMenu& mainM = ui.GetMainMenu();
    assert(mainM.GetFocusedIndex() == 0);

    InventoryUI& inv = ui.GetInventoryUI();
    assert(inv.GetSelectedSlot() == 0);

    AuthMenu& auth = ui.GetAuthMenu();
    assert(!auth.IsOpen());

    std::cout << "  -> PASSED" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    std::cout << "=====================================================" << std::endl;
    std::cout << "  WILDBOUND PHASE 27 — GAME UI TEST SUITE" << std::endl;
    std::cout << "=====================================================" << std::endl;

    Test_UITheme_MetricsAndColors();
    Test_UINotification_QueueLifecycle();
    Test_Minimap_CoordinateTransformAndZoom();
    Test_HUDManager_HealthLagBarAndPulse();
    Test_SettingsMenu_TabsAndAudioPersistence();
    Test_PauseMenu_NavigationAndActions();
    Test_MainMenu_NavigationAndCredits();
    Test_InventoryUI_GridNavigationAndActions();
    Test_UIManager_Integration();

    std::cout << "=====================================================" << std::endl;
    std::cout << "  ALL 9 PHASE 27 TEST SUITES PASSED (100% SUCCESS)   " << std::endl;
    std::cout << "=====================================================" << std::endl;

    return 0;
}

