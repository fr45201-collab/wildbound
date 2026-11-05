#pragma once

#include "Minimap.hpp"
#include "../UINotification.hpp"
#include "../UITheme.hpp"
#include <raylib.h>
#include <string>
#include <memory>

namespace wildbound::entities {
    class Player;
}

namespace wildbound::world {
    class World;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::interaction {
    struct InteractionPrompt;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::ui {

/**
 * Match Information State for HUD Presentation.
 */
struct MatchHUDInfo {
    float matchTimer = 0.0f;
    int playersAlive = 50;
    int eliminations = 0;
    std::string phaseName = "Phase 1 - In Progress";
    float stormTimer = 180.0f;
};

/**
 * Comprehensive Gameplay HUD Manager.
 * Orchestrates player vitals (health, lag bar, low-HP pulse), armor mitigation,
 * dual weapon slots (MainHand/OffHand), bow draw charge bar, ammo counter,
 * match status widget, status effects, and minimap radar.
 */
class HUDManager {
public:
    HUDManager();
    ~HUDManager() = default;

    void Reset();
    void Update(float dt, const entities::Player* player = nullptr);
    void Render(resources::AssetManager& assetMgr,
                const entities::Player* player,
                const world::World* world,
                const items::ItemManager* itemMgr,
                const interaction::InteractionPrompt* prompt = nullptr);

    // Subsystems
    [[nodiscard]] Minimap& GetMinimap() { return m_minimap; }
    [[nodiscard]] const Minimap& GetMinimap() const { return m_minimap; }

    [[nodiscard]] NotificationQueue& GetNotifications() { return m_notifications; }
    [[nodiscard]] const NotificationQueue& GetNotifications() const { return m_notifications; }

    // Match Info setters/getters
    void SetMatchInfo(const MatchHUDInfo& info) { m_matchInfo = info; }
    [[nodiscard]] MatchHUDInfo& GetMatchInfo() { return m_matchInfo; }
    [[nodiscard]] const MatchHUDInfo& GetMatchInfo() const { return m_matchInfo; }

    // HUD Toggles
    void SetHUDVisible(bool visible) { m_hudVisible = visible; }
    [[nodiscard]] bool IsHUDVisible() const { return m_hudVisible; }
    void ToggleHUDVisible() { m_hudVisible = !m_hudVisible; }

    // Vitals queries (for verification/testing)
    [[nodiscard]] float GetDisplayedHealth() const { return m_displayedHealth; }
    [[nodiscard]] float GetLagHealth() const { return m_lagHealth; }
    [[nodiscard]] bool IsLowHealthAlertActive() const { return m_isLowHealthAlert; }

private:
    void RenderVitals(const entities::Player* player);
    void RenderArmor(const entities::Player* player, const items::ItemManager* itemMgr);
    void RenderWeaponInterface(const entities::Player* player, const items::ItemManager* itemMgr);
    void RenderAmmoCounter(const entities::Player* player, const items::ItemManager* itemMgr);
    void RenderPlayerStatus(const entities::Player* player);
    void RenderMatchInfo();
    void RenderInteractionPrompt(const interaction::InteractionPrompt* prompt);

    bool m_hudVisible = true;

    // Health Vitals State
    float m_displayedHealth = 100.0f;
    float m_lagHealth = 100.0f;
    float m_maxHealth = 100.0f;
    float m_lagTimer = 0.0f;
    float m_lowHealthPulseTimer = 0.0f;
    bool m_isLowHealthAlert = false;

    // Subcomponents
    Minimap m_minimap;
    NotificationQueue m_notifications;
    MatchHUDInfo m_matchInfo;
};

} // namespace wildbound::ui

