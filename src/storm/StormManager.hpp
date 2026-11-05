#pragma once

#include "StormTypes.hpp"
#include "SafeZone.hpp"
#include "StormDamageSystem.hpp"
#include "StormBoundaryRenderer.hpp"
#include <raylib.h>
#include <memory>
#include <string>

namespace wildbound::entities {
    class Player;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::ui {
    class HUDManager;
}

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::storm {

/**
 * Master Controller for the Wildbound Battle Royale Corruption Storm.
 * Orchestrates phase progression, smooth zone interpolation, player status detection,
 * frame-rate independent damage application, visual atmospheric rendering,
 * audio events, and HUD/minimap integration.
 */
class StormManager {
public:
    StormManager();
    ~StormManager() = default;

    bool Initialize(const std::string& configPath = "data/storm/storm_config.json",
                    Rectangle playableBounds = Rectangle{ 768.0f, 768.0f, 8704.0f, 8704.0f });
    void Reset();

    void Update(float dt, entities::Player* player = nullptr,
                audio::AudioSystem* audioSys = nullptr,
                ui::HUDManager* hudMgr = nullptr);

    // Render hooks
    void RenderWorld(const graphics::Camera2DSystem& camera, resources::AssetManager* assetMgr = nullptr);
    void RenderScreenEffects(const graphics::Camera2DSystem& camera, const entities::Player* player = nullptr);
    void RenderMinimap(Vector2 minimapCenter, float minimapRadius, Vector2 playerPos,
                       float zoom, Rectangle worldBounds);
    void RenderHUDWarning(resources::AssetManager* assetMgr, const entities::Player* player = nullptr);

    // Lifecycle & Developer Controls
    void StartStorm();
    void AdvancePhase(audio::AudioSystem* audioSys = nullptr);
    void PauseStorm() noexcept { m_isPaused = true; }
    void ResumeStorm() noexcept { m_isPaused = false; }
    void TogglePause() noexcept { m_isPaused = !m_isPaused; }
    void ForcePlayerOutside(entities::Player* player);

    // Queries
    [[nodiscard]] StormState GetState() const noexcept { return m_state; }
    [[nodiscard]] bool IsPaused() const noexcept { return m_isPaused; }
    [[nodiscard]] int GetCurrentPhaseIndex() const noexcept { return m_currentPhaseIndex; }
    [[nodiscard]] size_t GetTotalPhases() const noexcept { return m_config.phases.size(); }
    [[nodiscard]] const StormPhaseConfig* GetCurrentPhaseConfig() const noexcept;

    [[nodiscard]] const SafeZone& GetCurrentSafeZone() const noexcept { return m_currentZone; }
    [[nodiscard]] const SafeZone& GetTargetSafeZone() const noexcept { return m_targetZone; }
    [[nodiscard]] const SafeZone& GetInitialSafeZone() const noexcept { return m_initialZone; }

    [[nodiscard]] float GetPhaseTimer() const noexcept { return m_phaseTimer; }
    [[nodiscard]] float GetStateDuration() const noexcept { return m_stateDuration; }
    [[nodiscard]] PlayerStormStatus GetPlayerStatus() const noexcept { return m_playerStatus; }
    [[nodiscard]] float GetCurrentDPS() const noexcept;

    [[nodiscard]] StormDamageSystem& GetDamageSystem() noexcept { return m_damageSystem; }
    [[nodiscard]] const StormDamageSystem& GetDamageSystem() const noexcept { return m_damageSystem; }

    [[nodiscard]] StormBoundaryRenderer& GetBoundaryRenderer() noexcept { return m_renderer; }
    [[nodiscard]] const StormBoundaryRenderer& GetBoundaryRenderer() const noexcept { return m_renderer; }

    [[nodiscard]] const StormConfig& GetConfig() const noexcept { return m_config; }
    [[nodiscard]] Rectangle GetPlayableBounds() const noexcept { return m_playableBounds; }

    // Direct programmatic phase/timer adjustments (for unit testing)
    void SetCurrentSafeZone(SafeZone zone) { m_currentZone = zone; }
    void SetTargetSafeZone(SafeZone zone) { m_targetZone = zone; }
    void SetPhaseTimer(float time) { m_phaseTimer = time; }
    void SetState(StormState state) { m_state = state; }

private:
    void SetupPhase(size_t phaseIdx, audio::AudioSystem* audioSys = nullptr);
    void UpdatePlayerStatus(const entities::Player* player);
    void SyncWithHUD(ui::HUDManager* hudMgr);

    StormConfig m_config;
    Rectangle m_playableBounds{ 768.0f, 768.0f, 8704.0f, 8704.0f };

    SafeZone m_initialZone;
    SafeZone m_currentZone;
    SafeZone m_previousZone;
    SafeZone m_targetZone;

    StormState m_state = StormState::Idle;
    PlayerStormStatus m_playerStatus = PlayerStormStatus::Safe;

    bool m_isPaused = false;
    size_t m_currentPhaseIndex = 0; // 0-based index into m_config.phases
    float m_phaseTimer = 0.0f;      // Counts down to 0
    float m_stateDuration = 0.0f;   // Total duration of current Warning or Shrinking state
    uint32_t m_rngSeed = 1337;

    StormDamageSystem m_damageSystem;
    StormBoundaryRenderer m_renderer;
};

} // namespace wildbound::storm

