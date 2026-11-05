#include "StormManager.hpp"
#include "../entities/Player.hpp"
#include "../audio/AudioSystem.hpp"
#include "../ui/hud/HUDManager.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"
#include <iomanip>
#include <sstream>

using wildbound::core::Logger;

namespace wildbound::storm {

StormManager::StormManager() = default;

bool StormManager::Initialize(const std::string& configPath, Rectangle playableBounds) {
    m_playableBounds = playableBounds;
    m_config = StormConfig::LoadFromJson(configPath, m_playableBounds);

    Reset();
    Logger::Info("STORM", "Initialized StormManager: [" + m_config.name + "] with " +
                 std::to_string(m_config.phases.size()) + " phases. Initial radius: " +
                 std::to_string(m_initialZone.radius));
    return true;
}

void StormManager::Reset() {
    m_state = StormState::Idle;
    m_isPaused = false;
    m_currentPhaseIndex = 0;
    m_phaseTimer = 0.0f;
    m_stateDuration = 0.0f;
    m_rngSeed = 1337;

    // Center and radius validation
    m_initialZone.radius = std::clamp(m_config.initialRadius, 100.0f, std::min(m_playableBounds.width, m_playableBounds.height) * 0.5f);
    m_initialZone.center = SafeZoneHelper::ClampCircleCenterToRect(m_config.initialCenter, m_initialZone.radius, m_playableBounds);

    m_currentZone = m_initialZone;
    m_previousZone = m_initialZone;
    m_targetZone = m_initialZone;

    m_playerStatus = PlayerStormStatus::Safe;
    m_damageSystem.Reset();
    m_renderer.Reset();
}

void StormManager::StartStorm() {
    if (m_config.phases.empty()) {
        m_state = StormState::Completed;
        return;
    }
    SetupPhase(0);
}

void StormManager::SetupPhase(size_t phaseIdx, audio::AudioSystem* audioSys) {
    if (phaseIdx >= m_config.phases.size()) {
        m_state = StormState::Completed;
        return;
    }

    m_currentPhaseIndex = phaseIdx;
    const auto& phase = m_config.phases[m_currentPhaseIndex];

    float targetRadius = std::max(10.0f, m_initialZone.radius * phase.targetRadiusRatio);
    Vector2 nextCenter = SafeZoneHelper::CalculateNextCenter(m_currentZone, targetRadius, m_playableBounds,
                                                             m_config.centerDriftMaxRatio, m_rngSeed++);

    m_targetZone.center = nextCenter;
    m_targetZone.radius = targetRadius;
    m_previousZone = m_currentZone;

    m_state = StormState::Warning;
    m_stateDuration = std::max(0.1f, phase.warningDuration);
    m_phaseTimer = m_stateDuration;

    if (audioSys && !phase.audioEvent.empty()) {
        audioSys->PlaySound(phase.audioEvent, 0.9f);
    }

    Logger::Info("STORM", "Starting Phase " + std::to_string(phase.phaseIndex) +
                 " [Warning: " + std::to_string(m_stateDuration) + "s, Target R: " +
                 std::to_string(targetRadius) + " at (" + std::to_string(nextCenter.x) +
                 "," + std::to_string(nextCenter.y) + ")]");
}

void StormManager::AdvancePhase(audio::AudioSystem* audioSys) {
    if (m_state == StormState::Idle) {
        StartStorm();
    } else if (m_state == StormState::Warning) {
        // Fast-forward warning into shrinking
        const auto& phase = m_config.phases[m_currentPhaseIndex];
        m_state = StormState::Shrinking;
        m_stateDuration = std::max(0.1f, phase.shrinkDuration);
        m_phaseTimer = m_stateDuration;
        if (audioSys) {
            audioSys->PlaySound("sfx_storm_shrink", 0.85f);
        }
    } else if (m_state == StormState::Shrinking) {
        // Snap to target and advance
        m_currentZone = m_targetZone;
        if (m_currentPhaseIndex + 1 < m_config.phases.size()) {
            SetupPhase(m_currentPhaseIndex + 1, audioSys);
        } else {
            m_state = StormState::Completed;
        }
    }
}

void StormManager::ForcePlayerOutside(entities::Player* player) {
    if (!player) return;
    // Teleport player 300 units outside current safe zone
    Vector2 pCenter = m_currentZone.center;
    float dist = m_currentZone.radius + 300.0f;
    player->Spawn(Vector2{ pCenter.x + dist, pCenter.y });
}

void StormManager::Update(float dt, entities::Player* player,
                          audio::AudioSystem* audioSys, ui::HUDManager* hudMgr) {
    if (m_isPaused) {
        // Timers do not progress when paused
        return;
    }

    if (m_state == StormState::Warning) {
        m_phaseTimer -= dt;
        if (m_phaseTimer <= 0.0f) {
            // Transition from Warning to Shrinking
            const auto& phase = m_config.phases[m_currentPhaseIndex];
            m_state = StormState::Shrinking;
            m_stateDuration = std::max(0.1f, phase.shrinkDuration);
            m_phaseTimer = m_stateDuration;

            if (audioSys) {
                audioSys->PlaySound("sfx_storm_shrink", 0.85f);
            }
            Logger::Info("STORM", "Phase " + std::to_string(phase.phaseIndex) +
                         " Shrink Started! Duration: " + std::to_string(m_stateDuration) + "s");
        }
    } else if (m_state == StormState::Shrinking) {
        m_phaseTimer -= dt;
        float t = 1.0f - (std::max(0.0f, m_phaseTimer) / m_stateDuration);
        m_currentZone = SafeZone::Lerp(m_previousZone, m_targetZone, t);

        if (m_phaseTimer <= 0.0f) {
            m_currentZone = m_targetZone;
            if (m_currentPhaseIndex + 1 < m_config.phases.size()) {
                SetupPhase(m_currentPhaseIndex + 1, audioSys);
            } else {
                m_state = StormState::Completed;
                Logger::Info("STORM", "Final Storm Phase completed.");
            }
        }
    }

    // Damage & Rendering updates
    float dps = GetCurrentDPS();
    m_damageSystem.Update(dt, m_currentZone, dps, player, audioSys);
    m_renderer.Update(dt, m_currentZone, player);

    UpdatePlayerStatus(player);
    SyncWithHUD(hudMgr);
}

void StormManager::UpdatePlayerStatus(const entities::Player* player) {
    if (!player) {
        m_playerStatus = PlayerStormStatus::Safe;
        return;
    }

    Vector2 pos = player->GetPosition();
    if (!m_currentZone.Contains(pos)) {
        m_playerStatus = PlayerStormStatus::Outside;
    } else if (m_currentPhaseIndex >= m_config.phases.size() - 1) {
        m_playerStatus = PlayerStormStatus::FinalZone;
    } else if (m_state == StormState::Shrinking) {
        m_playerStatus = PlayerStormStatus::Shrinking;
    } else if (m_state == StormState::Warning && m_phaseTimer <= 15.0f) {
        m_playerStatus = PlayerStormStatus::Approaching;
    } else {
        m_playerStatus = PlayerStormStatus::Safe;
    }
}

void StormManager::SyncWithHUD(ui::HUDManager* hudMgr) {
    if (!hudMgr) return;

    auto& matchInfo = hudMgr->GetMatchInfo();
    matchInfo.stormTimer = std::max(0.0f, m_phaseTimer);

    const auto* phase = GetCurrentPhaseConfig();
    if (phase) {
        if (m_state == StormState::Warning) {
            matchInfo.phaseName = phase->name + " (Wait)";
        } else if (m_state == StormState::Shrinking) {
            matchInfo.phaseName = phase->name + " (Shrink)";
        } else if (m_state == StormState::Completed) {
            matchInfo.phaseName = "Final Zone Closed";
        }
    }
}

const StormPhaseConfig* StormManager::GetCurrentPhaseConfig() const noexcept {
    if (m_currentPhaseIndex < m_config.phases.size()) {
        return &m_config.phases[m_currentPhaseIndex];
    }
    return nullptr;
}

float StormManager::GetCurrentDPS() const noexcept {
    if (m_state == StormState::Idle) return 0.0f;
    const auto* phase = GetCurrentPhaseConfig();
    return phase ? phase->damagePerSecond : 1.0f;
}

void StormManager::RenderWorld(const graphics::Camera2DSystem& camera, resources::AssetManager* assetMgr) {
    const auto* phase = GetCurrentPhaseConfig();
    float intensity = phase ? phase->visualIntensity : 0.5f;
    bool isShrinking = (m_state == StormState::Shrinking);

    m_renderer.RenderWorld(m_currentZone, m_targetZone, isShrinking, intensity, camera, assetMgr);
}

void StormManager::RenderScreenEffects(const graphics::Camera2DSystem& camera, const entities::Player* player) {
    const auto* phase = GetCurrentPhaseConfig();
    float intensity = phase ? phase->visualIntensity : 0.5f;
    m_renderer.RenderScreenEffects(m_currentZone, player, camera, intensity);
}

void StormManager::RenderMinimap(Vector2 minimapCenter, float minimapRadius, Vector2 playerPos,
                                float zoom, Rectangle worldBounds) {
    bool isShrinking = (m_state == StormState::Shrinking);
    m_renderer.RenderMinimap(m_currentZone, m_targetZone, isShrinking, minimapCenter, minimapRadius,
                             playerPos, zoom, worldBounds);
}

void StormManager::RenderHUDWarning(resources::AssetManager* assetMgr, const entities::Player* player) {
    if (m_state == StormState::Idle) return;

    // Render prominent warning banner at top-center of screen (coordinates in 640x360 virtual space)
    int minutes = static_cast<int>(m_phaseTimer) / 60;
    int seconds = static_cast<int>(m_phaseTimer) % 60;
    char timerBuf[16];
    snprintf(timerBuf, sizeof(timerBuf), "%02d:%02d", minutes, seconds);

    Rectangle bannerRect{ 200.0f, 10.0f, 240.0f, 28.0f };

    if (m_playerStatus == PlayerStormStatus::Outside) {
        // State D: Outside safe zone!
        DrawRectangleRec(bannerRect, Color{ 80, 10, 20, 230 });
        DrawRectangleLinesEx(bannerRect, 1.5f, RED);
        DrawText("YOU ARE OUTSIDE THE SAFE ZONE", 212, 14, 10, WHITE);
        DrawText(TextFormat("TAKING %d DPS!", static_cast<int>(GetCurrentDPS())), 285, 25, 10, YELLOW);
    } else if (m_playerStatus == PlayerStormStatus::FinalZone) {
        // State E: Final Zone
        DrawRectangleRec(bannerRect, Color{ 60, 10, 40, 220 });
        DrawRectangleLinesEx(bannerRect, 1.5f, PURPLE);
        DrawText("FINAL SAFE ZONE COLLAPSING", 225, 14, 10, WHITE);
        DrawText(timerBuf, 305, 25, 10, YELLOW);
    } else if (m_state == StormState::Shrinking) {
        // State C: Zone shrinking
        DrawRectangleRec(bannerRect, Color{ 40, 20, 60, 210 });
        DrawRectangleLinesEx(bannerRect, 1.5f, ORANGE);
        DrawText("SAFE ZONE IS SHRINKING", 245, 14, 10, WHITE);
        DrawText(timerBuf, 305, 25, 10, YELLOW);
    } else if (m_state == StormState::Warning) {
        // State B: Storm approaching
        DrawRectangleRec(bannerRect, Color{ 20, 30, 50, 200 });
        DrawRectangleLinesEx(bannerRect, 1.0f, Color{ 100, 200, 255, 200 });
        DrawText("CORRUPTION STORM APPROACHING", 220, 14, 10, WHITE);
        DrawText(timerBuf, 305, 25, 10, Color{ 120, 240, 255, 255 });
    }
}

} // namespace wildbound::storm

