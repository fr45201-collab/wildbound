#pragma once

#include "MatchState.hpp"
#include "MatchConfiguration.hpp"
#include "MatchParticipant.hpp"
#include "MatchParticipantRegistry.hpp"
#include "MatchSpawner.hpp"
#include "MatchCountdown.hpp"
#include "MatchEliminationSystem.hpp"
#include "MatchStatistics.hpp"
#include "MatchResult.hpp"
#include "results/MatchResultManager.hpp"
#include "results/MatchResultModel.hpp"

#include <memory>
#include <string>
#include <vector>

namespace wildbound::entities {
    class Player;
}

namespace wildbound::world {
    class WorldManager;
    class World;
}

namespace wildbound::storm {
    class StormManager;
}

namespace wildbound::ui {
    class UIManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::gamemodes {
    class GameModeManager;
}

namespace wildbound::combat {
    struct DeathEvent;
}

namespace wildbound::match {

/**
 * Master Controller for the Wildbound Battle Royale Match Lifecycle.
 * Coordinates match state transitions, participant registrations, spawn placement,
 * pre-match countdown, active match progression, authoritative eliminations,
 * final-survivor detection, results compilation, and cleanup.
 */
class MatchManager {
public:
    MatchManager();
    ~MatchManager();

    // Lifecycle Pipeline
    bool Initialize(const std::string& configPath = "data/match/default_match.json",
                    const world::WorldManager* worldMgr = nullptr);
    bool Initialize(const MatchConfiguration& config,
                    const world::WorldManager* worldMgr = nullptr);

    bool StartCountdown();
    bool ForceStartMatch();
    void Update(float dt,
                entities::Player* player = nullptr,
                storm::StormManager* stormMgr = nullptr,
                ui::UIManager* uiMgr = nullptr,
                audio::AudioSystem* audioSys = nullptr);

    bool CompleteMatch(audio::AudioSystem* audioSys = nullptr,
                       ui::UIManager* uiMgr = nullptr,
                       storm::StormManager* stormMgr = nullptr);
    void ResetMatch(entities::Player* player = nullptr,
                    storm::StormManager* stormMgr = nullptr,
                    ui::UIManager* uiMgr = nullptr);

    // Participant Registration
    bool RegisterParticipant(const MatchParticipant& participant);
    bool RegisterLocalPlayer(entities::Player* player);
    bool UnregisterParticipant(uint32_t participantId);

    // Spawning
    bool PrepareSpawns(const world::WorldManager* worldMgr = nullptr);
    bool SpawnParticipants(entities::Player* localPlayer = nullptr);

    // Elimination & Death Events
    bool ProcessElimination(uint32_t victimParticipantId,
                            EliminationCause cause,
                            uint32_t killerParticipantId = 0);
    bool HandleCombatDeath(const combat::DeathEvent& deathEvent);

    // State Queries
    [[nodiscard]] MatchState GetState() const noexcept { return m_state; }
    [[nodiscard]] const char* GetStateString() const noexcept { return MatchStateToString(m_state); }
    [[nodiscard]] bool IsActive() const noexcept { return m_state == MatchState::Active; }
    [[nodiscard]] bool IsGameplayAllowed() const noexcept;
    [[nodiscard]] float GetMatchTime() const noexcept { return m_matchTime; }
    [[nodiscard]] const MatchConfiguration& GetConfig() const noexcept { return m_config; }
    [[nodiscard]] const MatchParticipantRegistry& GetRegistry() const noexcept { return m_registry; }
    [[nodiscard]] MatchParticipantRegistry& GetRegistry() noexcept { return m_registry; }
    [[nodiscard]] const MatchSpawner& GetSpawner() const noexcept { return m_spawner; }
    [[nodiscard]] MatchSpawner& GetSpawner() noexcept { return m_spawner; }
    [[nodiscard]] const MatchCountdown& GetCountdown() const noexcept { return m_countdown; }
    [[nodiscard]] const MatchStatistics& GetStatistics() const noexcept { return m_statistics; }
    [[nodiscard]] MatchStatistics& GetStatistics() noexcept { return m_statistics; }
    [[nodiscard]] const MatchResult& GetResult() const noexcept { return m_result; }
    [[nodiscard]] const results::MatchResultModel& GetFinalResultModel() const noexcept { return m_resultManager.GetFinalResult(); }
    [[nodiscard]] const results::MatchResultManager& GetResultManager() const noexcept { return m_resultManager; }
    [[nodiscard]] results::MatchResultManager& GetResultManager() noexcept { return m_resultManager; }
    [[nodiscard]] uint32_t GetLocalParticipantId() const noexcept { return m_localParticipantId; }

    void SetGameModeManager(gamemodes::GameModeManager* modeMgr) noexcept { m_gameModeManager = modeMgr; }
    [[nodiscard]] gamemodes::GameModeManager* GetGameModeManager() const noexcept { return m_gameModeManager; }

    // Manual / Developer Controls
    bool TransitionTo(MatchState newState);
    void SetAllowDevSingleParticipant(bool allow) noexcept { m_config.allowDevSingleParticipant = allow; }

    using ScreenChangeCallback = std::function<void(const std::string&)>;
    void SetScreenCallback(ScreenChangeCallback cb) { m_screenCallback = std::move(cb); }

private:
    void EvaluateFinalSurvivor();
    void BuildMatchResult();

    MatchState m_state = MatchState::Uninitialized;
    MatchConfiguration m_config;
    MatchParticipantRegistry m_registry;
    MatchSpawner m_spawner;
    MatchCountdown m_countdown;
    MatchEliminationSystem m_eliminationSystem;
    MatchStatistics m_statistics;
    MatchResult m_result;
    results::MatchResultManager m_resultManager;
    gamemodes::GameModeManager* m_gameModeManager = nullptr;
    ScreenChangeCallback m_screenCallback;

    float m_matchTime = 0.0f;
    uint32_t m_localParticipantId = 1;
    bool m_winnerDetermined = false;
    bool m_stormStarted = false;
};

} // namespace wildbound::match
