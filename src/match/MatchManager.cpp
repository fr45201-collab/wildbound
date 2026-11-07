#include "MatchManager.hpp"
#include "../entities/Player.hpp"
#include "../world/WorldManager.hpp"
#include "../storm/StormManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../combat/CombatEvents.hpp"
#include "../core/Logger.hpp"
#include "../gamemodes/GameModeManager.hpp"

#include <algorithm>

namespace wildbound::match {

MatchManager::MatchManager() = default;
MatchManager::~MatchManager() = default;

bool MatchManager::TransitionTo(MatchState newState) {
    if (!IsValidStateTransition(m_state, newState)) {
        core::Logger::Warn("MATCH", "Rejected invalid state transition from [" +
                                    std::string(MatchStateToString(m_state)) + "] to [" +
                                    std::string(MatchStateToString(newState)) + "].");
        return false;
    }

    core::Logger::Info("MATCH", "State transition: [" +
                                std::string(MatchStateToString(m_state)) + "] -> [" +
                                std::string(MatchStateToString(newState)) + "].");

    m_state = newState;
    return true;
}

bool MatchManager::Initialize(const std::string& configPath, const world::WorldManager* worldMgr) {
    MatchConfiguration cfg;
    if (!cfg.LoadFromFile(configPath)) {
        core::Logger::Warn("MATCH", "Could not load config from " + configPath + ", falling back to default match configuration.");
        cfg = MatchConfiguration::CreateDefault();
    }
    return Initialize(cfg, worldMgr);
}

bool MatchManager::Initialize(const MatchConfiguration& config, const world::WorldManager* worldMgr) {
    if (m_state != MatchState::Uninitialized) {
        core::Logger::Warn("MATCH", "Cannot initialize match: match already in state [" +
                                    std::string(MatchStateToString(m_state)) + "]. Resetting first.");
        ResetMatch();
    }

    if (!TransitionTo(MatchState::Initializing)) {
        return false;
    }

    std::vector<std::string> configErrors;
    if (!config.Validate(configErrors)) {
        for (const auto& err : configErrors) {
            core::Logger::Error("MATCH_INIT", "Configuration validation error: " + err);
        }
        TransitionTo(MatchState::Failed);
        return false;
    }

    m_config = config;
    m_matchTime = 0.0f;
    m_winnerDetermined = false;
    m_stormStarted = false;

    m_registry.ResetRegistry();
    m_statistics.Reset();
    m_result.Reset();
    m_resultManager.Reset();

    if (m_gameModeManager) {
        m_gameModeManager->Reset();
    }
    m_countdown.Reset();

    if (!PrepareSpawns(worldMgr)) {
        core::Logger::Warn("MATCH_INIT", "WorldManager spawn points could not be loaded; using default boundary spawns.");
    }

    core::Logger::Info("MATCH", "Initialized match [" + m_config.matchId + "] on world [" + m_config.worldId + "].");
    return TransitionTo(MatchState::WaitingForPlayers);
}

bool MatchManager::PrepareSpawns(const world::WorldManager* worldMgr) {
    Rectangle playableBounds{ 960.0f, 960.0f, 8320.0f, 8320.0f };
    if (worldMgr) {
        playableBounds = worldMgr->GetPlayableBoundary();
        if (m_spawner.InitializeSpawnPoints(*worldMgr, playableBounds)) {
            core::Logger::Info("MATCH", "Loaded " + std::to_string(m_spawner.GetTotalSpawnCount()) + " spawn points from WorldManager.");
            return true;
        }
    }

    // Default fallback spawn coordinates if worldMgr was unavailable
    m_spawner.AddCustomSpawnPoint(Vector2{ 4160.0f, 5200.0f }, "Default_Forest");
    m_spawner.AddCustomSpawnPoint(Vector2{ 4208.0f, 5152.0f }, "Default_Forest");
    m_spawner.AddCustomSpawnPoint(Vector2{ 5000.0f, 5000.0f }, "Default_Center");
    m_spawner.AddCustomSpawnPoint(Vector2{ 3500.0f, 3500.0f }, "Default_NorthWest");
    return true;
}

bool MatchManager::RegisterParticipant(const MatchParticipant& participant) {
    if (m_state != MatchState::WaitingForPlayers && m_state != MatchState::Initializing) {
        core::Logger::Warn("MATCH", "Cannot register participant in state [" + std::string(MatchStateToString(m_state)) + "].");
        return false;
    }

    if (!m_registry.RegisterParticipant(participant)) {
        core::Logger::Warn("MATCH", "Failed to register participant with ID: " + std::to_string(participant.participantId));
        return false;
    }

    core::Logger::Info("MATCH", "Registered participant [" + participant.displayName + "] (ID: " +
                                std::to_string(participant.participantId) + ", Type: " +
                                std::string(ParticipantTypeToString(participant.type)) + "). Total: " +
                                std::to_string(m_registry.GetTotalCount()));
    return true;
}

bool MatchManager::RegisterLocalPlayer(entities::Player* player) {
    if (!player) return false;

    MatchParticipant localP;
    localP.participantId = m_localParticipantId;
    localP.type = ParticipantType::LocalHuman;
    localP.displayName = player->GetName().empty() ? "Player" : player->GetName();
    localP.entityId = player->GetId();
    localP.status = ParticipantStatus::Registered;
    localP.spawnPosition = player->GetPosition();

    return RegisterParticipant(localP);
}

bool MatchManager::UnregisterParticipant(uint32_t participantId) {
    if (m_state != MatchState::WaitingForPlayers && m_state != MatchState::Initializing) {
        return false;
    }
    return m_registry.UnregisterParticipant(participantId);
}

bool MatchManager::SpawnParticipants(entities::Player* localPlayer) {
    auto& participants = m_registry.GetAllParticipants();
    if (participants.empty()) {
        core::Logger::Error("MATCH_SPAWN", "No participants registered to spawn.");
        return false;
    }

    bool spawnSuccess = false;
    if (m_gameModeManager && m_gameModeManager->GetActiveMode().teamsEnabled) {
        spawnSuccess = m_spawner.AssignTeamSpawnPoints(participants);
    } else {
        spawnSuccess = m_spawner.AssignSpawnPoints(participants, SpawnSelectionStrategy::RegionalDistributed);
    }

    if (!spawnSuccess) {
        core::Logger::Error("MATCH_SPAWN", "Insufficient spawn points available for " + std::to_string(participants.size()) + " participants.");
        return false;
    }

    for (auto& p : participants) {
        p.status = ParticipantStatus::Alive;
        p.survivalDuration = 0.0f;

        if (p.type == ParticipantType::LocalHuman && localPlayer) {
            localPlayer->Spawn(p.spawnPosition);
            localPlayer->GetHealth().Reset();
        }
    }

    core::Logger::Info("MATCH_SPAWN", "Successfully assigned and spawned " + std::to_string(participants.size()) + " participants.");
    return true;
}

bool MatchManager::StartCountdown() {
    if (m_state != MatchState::WaitingForPlayers) {
        core::Logger::Warn("MATCH", "Cannot start countdown from state [" + std::string(MatchStateToString(m_state)) + "].");
        return false;
    }

    size_t count = m_registry.GetTotalCount();
    if (count < m_config.minimumParticipants && !m_config.allowDevSingleParticipant) {
        core::Logger::Error("MATCH", "Minimum participants required is " + std::to_string(m_config.minimumParticipants) +
                                     ", but only " + std::to_string(count) + " registered. Aborting match start.");
        TransitionTo(MatchState::Failed);
        return false;
    }

    if (!SpawnParticipants(nullptr)) {
        core::Logger::Error("MATCH", "Participant spawning failed. Aborting match start.");
        TransitionTo(MatchState::Failed);
        return false;
    }

    if (!TransitionTo(MatchState::Countdown)) {
        return false;
    }

    m_countdown.Start(m_config.countdownSeconds);
    core::Logger::Info("MATCH", "Countdown started: " + std::to_string(m_config.countdownSeconds) + " seconds.");
    return true;
}

bool MatchManager::ForceStartMatch() {
    if (m_state == MatchState::WaitingForPlayers) {
        if (!SpawnParticipants(nullptr)) return false;
    }

    if (m_state == MatchState::WaitingForPlayers || m_state == MatchState::Countdown) {
        m_countdown.Reset();
        if (m_state == MatchState::WaitingForPlayers) {
            TransitionTo(MatchState::Countdown);
        }
        if (TransitionTo(MatchState::Starting)) {
            return TransitionTo(MatchState::Active);
        }
    }
    return false;
}

bool MatchManager::IsGameplayAllowed() const noexcept {
    return (m_state == MatchState::Active);
}

void MatchManager::Update(float dt,
                          entities::Player* player,
                          storm::StormManager* stormMgr,
                          ui::UIManager* uiMgr,
                          audio::AudioSystem* audioSys) {
    (void)uiMgr;

    if (m_state == MatchState::Countdown) {
        m_countdown.Update(dt);
        if (m_countdown.IsFinished()) {
            if (TransitionTo(MatchState::Starting)) {
                if (stormMgr) {
                    stormMgr->Reset();
                }
                if (TransitionTo(MatchState::Active)) {
                    if (stormMgr) {
                        stormMgr->StartStorm();
                        m_stormStarted = true;
                    }
                    if (audioSys) {
                        audioSys->DispatchEvent("Combat.Start");
                    }
                }
            }
        }
    } else if (m_state == MatchState::Active) {
        m_matchTime += dt;
        m_statistics.matchDuration = m_matchTime;

        if (!m_stormStarted && stormMgr) {
            stormMgr->StartStorm();
            m_stormStarted = true;
        }

        // Update survival time for alive participants
        for (auto& p : m_registry.GetAllParticipants()) {
            if (p.IsAlive()) {
                p.survivalDuration = m_matchTime;
            }
        }

        // Authoritative death check for local player
        if (player && player->GetHealth().IsDead()) {
            MatchParticipant* localP = m_registry.FindParticipant(m_localParticipantId);
            if (localP && localP->IsAlive()) {
                // If storm was damaging, attribute to storm; otherwise combat/environment
                EliminationCause cause = EliminationCause::Combat;
                if (stormMgr && stormMgr->GetDamageSystem().IsPlayerDamagedThisTick()) {
                    cause = EliminationCause::CorruptionStorm;
                }
                ProcessElimination(m_localParticipantId, cause, 0);
            }
        }

        EvaluateFinalSurvivor();

        if (m_winnerDetermined) {
            if (TransitionTo(MatchState::Finishing)) {
                CompleteMatch(audioSys, uiMgr, stormMgr);
            }
        }
    }
}

bool MatchManager::ProcessElimination(uint32_t victimParticipantId,
                                      EliminationCause cause,
                                      uint32_t killerParticipantId) {
    if (m_state != MatchState::Active && m_state != MatchState::Finishing) {
        core::Logger::Warn("MATCH", "Elimination received outside of active match state.");
    }

    bool processed = m_eliminationSystem.ProcessElimination(
        victimParticipantId, cause, killerParticipantId, m_matchTime, m_registry, m_statistics
    );

    if (processed) {
        if (const auto* victim = m_registry.FindParticipant(victimParticipantId)) {
            m_resultManager.RecordEliminationEvent(*victim, victim->eliminationInfo, m_matchTime);
        }

        core::Logger::Info("MATCH", "Participant " + std::to_string(victimParticipantId) +
                                    " eliminated by cause [" + std::string(EliminationCauseToString(cause)) +
                                    "]. Remaining alive: " + std::to_string(m_registry.GetAliveCount()));
    }

    return processed;
}

bool MatchManager::HandleCombatDeath(const combat::DeathEvent& deathEvent) {
    MatchParticipant* victim = m_registry.FindParticipantByEntityId(deathEvent.victimId);
    if (!victim) {
        return false;
    }

    uint32_t killerParticipantId = 0;
    if (deathEvent.killerId != 0) {
        if (const auto* killer = m_registry.FindParticipantByEntityId(deathEvent.killerId)) {
            killerParticipantId = killer->participantId;
        }
    }

    return ProcessElimination(victim->participantId, EliminationCause::Combat, killerParticipantId);
}

void MatchManager::EvaluateFinalSurvivor() {
    if (m_winnerDetermined || m_state != MatchState::Active) {
        return;
    }

    results::MatchOutcome outcome = results::MatchOutcome::InProgress;
    uint32_t winnerId = 0;
    std::string winnerName;
    bool evaluated = false;

    if (m_gameModeManager) {
        evaluated = m_gameModeManager->EvaluateVictory(m_registry, true, outcome, winnerId, winnerName);
    } else {
        evaluated = m_resultManager.GetVictoryEvaluator().Evaluate(m_registry, true, outcome, winnerId, winnerName);
    }

    if (evaluated) {
        m_winnerDetermined = true;
        m_statistics.winnerParticipantId = winnerId;
        m_statistics.winnerDisplayName = winnerName;

        if (outcome == results::MatchOutcome::Victory) {
            core::Logger::Info("MATCH", "Final survivor detected! Winner: [" + winnerName +
                                        "] (Participant ID: " + std::to_string(winnerId) + ")");
        } else if (outcome == results::MatchOutcome::Draw) {
            core::Logger::Info("MATCH", "All participants eliminated simultaneously. Result: Draw.");
        }

        m_resultManager.GetResultCalculator().CalculatePlacements(m_registry, winnerId, outcome);
    }
}

void MatchManager::BuildMatchResult() {
    m_result.Reset();
    m_result.hasResult = true;
    m_result.matchDuration = m_matchTime;
    m_result.totalParticipants = static_cast<uint32_t>(m_registry.GetTotalCount());
    m_result.winnerParticipantId = m_statistics.winnerParticipantId;
    m_result.winnerDisplayName = m_statistics.winnerDisplayName;

    const MatchParticipant* localP = m_registry.FindParticipant(m_localParticipantId);
    if (localP) {
        m_result.isVictory = localP->isWinner;
        m_result.localPlayerPlacement = localP->placement;
        m_result.localPlayerKills = localP->kills;
        m_result.localPlayerDamageDealt = localP->damageDealt;
        m_result.localPlayerDamageReceived = localP->damageReceived;
        m_result.localPlayerSurvivalDuration = localP->survivalDuration;
        m_result.localPlayerEliminationCause = localP->eliminationInfo.cause;
        m_result.killerDisplayName = localP->eliminationInfo.killerName;
    }

    m_result.leaderboard = m_registry.GetAllParticipants();
    std::sort(m_result.leaderboard.begin(), m_result.leaderboard.end(), [](const MatchParticipant& a, const MatchParticipant& b) {
        return a.placement < b.placement;
    });
}

bool MatchManager::CompleteMatch(audio::AudioSystem* audioSys,
                                ui::UIManager* uiMgr,
                                storm::StormManager* stormMgr) {
    if (m_state != MatchState::Finishing && m_state != MatchState::Active) {
        core::Logger::Warn("MATCH", "Cannot complete match in state [" + std::string(MatchStateToString(m_state)) + "].");
        return false;
    }

    if (m_state == MatchState::Active) {
        TransitionTo(MatchState::Finishing);
    }

    m_resultManager.FinalizeMatchResult(m_config.matchId, m_matchTime, m_localParticipantId, m_registry, m_statistics, true);
    BuildMatchResult();

    if (!TransitionTo(MatchState::Completed)) {
        return false;
    }

    if (stormMgr) {
        stormMgr->PauseStorm();
    }

    core::Logger::Info("MATCH", "Match completed successfully in " + std::to_string(m_matchTime) +
                                "s. Local result: " + (m_result.isVictory ? "VICTORY" : "DEFEAT") +
                                " (Rank #" + std::to_string(m_result.localPlayerPlacement) + ")");

    (void)uiMgr;
    if (m_screenCallback && m_config.showResultsScreen) {
        m_screenCallback("MatchResults");
    }

    if (audioSys) {
        if (m_result.isVictory) {
            audioSys->DispatchEvent("Music.PlayVictory");
        } else {
            audioSys->DispatchEvent("Music.PlayDefeat");
        }
    }

    return true;
}

void MatchManager::ResetMatch(entities::Player* player,
                              storm::StormManager* stormMgr,
                              ui::UIManager* uiMgr) {
    (void)uiMgr;
    core::Logger::Info("MATCH", "Resetting match resources and state.");

    if (m_state != MatchState::Uninitialized) {
        // Go through Cleanup
        if (m_state != MatchState::Cleanup) {
            TransitionTo(MatchState::Cleanup);
        }
        TransitionTo(MatchState::Uninitialized);
    }

    m_matchTime = 0.0f;
    m_winnerDetermined = false;
    m_stormStarted = false;

    m_registry.ResetRegistry();
    m_spawner.ResetSpawns();
    m_countdown.Reset();
    m_statistics.Reset();
    m_result.Reset();
    m_resultManager.Reset();

    if (m_gameModeManager) {
        m_gameModeManager->Reset();
    }

    if (stormMgr) {
        stormMgr->Reset();
    }

    if (player) {
        player->GetHealth().Reset();
        player->GetInventory().Clear();
    }

    if (m_screenCallback) {
        m_screenCallback("MainMenu");
    }
}

} // namespace wildbound::match
