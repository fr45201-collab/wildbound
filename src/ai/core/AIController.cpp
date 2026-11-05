#include "AIController.hpp"
#include "../../entities/Enemy.hpp"
#include "../../combat/CombatSystem.hpp"
#include "../../audio/AudioSystem.hpp"
#include "../../core/Logger.hpp"
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::ai {

AIController::AIController() = default;

void AIController::Initialize(uint32_t entityId,
                              const CreatureAIProfile& profile,
                              Vector2 spawnPos,
                              const NavigationGrid* navGrid)
{
    m_entityId = entityId;
    m_profile = profile;
    m_spawnOrigin = spawnPos;
    m_state = AIState::Idle;
    m_targetId = 0;
    m_targetPos = spawnPos;
    m_desiredVelocity = Vector2{ 0.0f, 0.0f };
    m_wantsAttack = false;

    m_decisionTimer = 0.0f;
    m_pathTimer = 0.0f;
    m_pursuitTimer = 0.0f;

    // Configure Perception
    PerceptionConfig pCfg;
    pCfg.viewDistance = profile.detectionRadius;
    pCfg.fieldOfViewDegrees = profile.fieldOfView;
    pCfg.omnidirectionalWhenAlert = profile.omnidirectionalWhenAlert;
    pCfg.requireLineOfSight = true;
    m_perception.SetConfig(pCfg);

    // Configure Target Memory
    TargetMemoryConfig tmCfg;
    tmCfg.memoryDuration = profile.pursuitDuration;
    m_targetMemory.SetConfig(tmCfg);

    // Configure Patrol
    if (!profile.defaultPatrol.points.empty()) {
        m_patrol.SetRoute(profile.defaultPatrol);
    } else {
        m_patrol.SetWanderZone(spawnPos, 80.0f);
    }

    // Configure Movement
    MovementControllerConfig mcCfg;
    mcCfg.waypointArrivalRadius = 14.0f;
    mcCfg.goalArrivalRadius = profile.attackRadius * 0.75f;
    m_movement.SetConfig(mcCfg);

    // Configure Stuck Detector
    m_stuckDetector.Reset(spawnPos);

    if (navGrid) {
        PathfindingConfig pfCfg;
        pfCfg.mode = MovementMode::EightDirectional;
        pfCfg.preventCornerCutting = true;
        pfCfg.enableSmoothing = true;
        m_pathfinding = std::make_unique<Pathfinding>(*navGrid, pfCfg);
    }
}

void AIController::SetState(AIState state) {
    m_state = state;
    if (state == AIState::Idle || state == AIState::Dead) {
        m_desiredVelocity = Vector2{ 0.0f, 0.0f };
        m_movement.ClearPath();
    }
}

void AIController::ForceTarget(uint32_t targetId, Vector2 targetPos) {
    m_targetId = targetId;
    m_targetPos = targetPos;
    m_targetMemory.RecordTarget(targetId, targetPos, 2.0f);
    SetState(AIState::Chase);
}

void AIController::OnDamageTaken(uint32_t attackerId, float damage, audio::AudioSystem* audioSys) {
    (void)damage;
    if (attackerId != 0 && (m_targetId == 0 || m_state == AIState::Idle || m_state == AIState::Patrol)) {
        m_targetId = attackerId;
        SetState(AIState::Chase);

        if (audioSys) {
            std::string alertEvent = "creature." + m_profile.archetype + ".alert";
            audioSys->DispatchEvent(alertEvent);
        }
    }
}

void AIController::OnDeath() {
    m_state = AIState::Dead;
    m_desiredVelocity = Vector2{ 0.0f, 0.0f };
    m_targetId = 0;
    m_movement.ClearPath();
}

void AIController::SetPatrolRoute(const PatrolRoute& route) {
    m_patrol.SetRoute(route);
}

void AIController::UpdatePathToTarget(Vector2 currentPos, Vector2 targetPos, const NavigationGrid& navGrid) {
    if (!m_pathfinding) {
        PathfindingConfig pfCfg;
        pfCfg.mode = MovementMode::EightDirectional;
        pfCfg.preventCornerCutting = true;
        pfCfg.enableSmoothing = true;
        m_pathfinding = std::make_unique<Pathfinding>(navGrid, pfCfg);
    }

    PathResult path = m_pathfinding->FindPath(currentPos, targetPos);
    if (path.IsValid()) {
        m_movement.SetPath(path);
        m_stuckDetector.OnPathReplanned();
    }
}

void AIController::ExecuteDecision(Vector2 currentPos,
                                   float hpPercent,
                                   const std::vector<TargetCandidate>& candidates,
                                   const NavigationGrid& navGrid,
                                   combat::CombatSystem& combatSys,
                                   entities::Enemy* owner,
                                   audio::AudioSystem* audioSys)
{
    if (m_state == AIState::Dead) return;

    // 1. Health check: Retreat if low HP and archetype allows retreat
    if (m_profile.canRetreat && hpPercent <= m_profile.retreatHealthPercent && m_targetId != 0) {
        if (m_state != AIState::Retreat) {
            SetState(AIState::Retreat);
        }
        return;
    }

    // 2. Perception & Target Evaluation
    bool isAlert = (m_state == AIState::Alert || m_state == AIState::Chase || m_state == AIState::Attack);
    uint32_t bestCandidateId = 0;
    Vector2 bestCandPos = m_targetPos;
    bool foundVisibleTarget = false;

    for (const auto& cand : candidates) {
        if (m_perception.CanDetectTarget(currentPos, m_desiredFacing, cand.position, isAlert, navGrid)) {
            bestCandidateId = cand.id;
            bestCandPos = cand.position;
            foundVisibleTarget = true;
            break;
        }
    }

    if (foundVisibleTarget) {
        if (m_targetId != bestCandidateId && m_targetId == 0) {
            // Newly acquired target: trigger alert sound
            if (audioSys) {
                audioSys->DispatchEvent("creature." + m_profile.archetype + ".alert");
            }
        }
        m_targetId = bestCandidateId;
        m_targetPos = bestCandPos;
        m_targetMemory.RecordTarget(m_targetId, m_targetPos);
        m_pursuitTimer = 0.0f;

        float dx = m_targetPos.x - currentPos.x;
        float dy = m_targetPos.y - currentPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= m_profile.attackRadius) {
            SetState(AIState::Attack);
            m_desiredVelocity = Vector2{ 0.0f, 0.0f };
            m_wantsAttack = true;

            // Face target
            if (std::abs(dx) > std::abs(dy)) {
                m_desiredFacing = (dx > 0.0f) ? graphics::animation::Direction::Right : graphics::animation::Direction::Left;
            } else {
                m_desiredFacing = (dy > 0.0f) ? graphics::animation::Direction::Down : graphics::animation::Direction::Up;
            }

            if (owner) {
                combatSys.ProcessAttack(owner, owner->GetConfig().combat.defaultAttack, m_desiredFacing);
            }
        } else {
            SetState(AIState::Chase);
            m_wantsAttack = false;
        }
    } else {
        // Target is not visible in current perception check
        if (m_targetMemory.HasActiveMemory()) {
            // Target was seen recently: switch to Investigate
            if (m_state == AIState::Chase || m_state == AIState::Attack) {
                SetState(AIState::Investigate);
                m_investigate.StartInvestigation(m_targetMemory.GetLastKnownPosition(), m_profile.investigateDuration);
                UpdatePathToTarget(currentPos, m_targetMemory.GetLastKnownPosition(), navGrid);
            }
        } else {
            // No target and memory expired: Return or Patrol
            m_targetId = 0;
            float dSpawnX = currentPos.x - m_spawnOrigin.x;
            float dSpawnY = currentPos.y - m_spawnOrigin.y;
            float distToSpawn = std::sqrt(dSpawnX * dSpawnX + dSpawnY * dSpawnY);

            if (distToSpawn > 120.0f) {
                SetState(AIState::Return);
            } else {
                SetState(AIState::Patrol);
            }
        }
    }
}

void AIController::Update(float dt,
                          Vector2 currentPos,
                          float currentHp,
                          float maxHp,
                          const std::vector<TargetCandidate>& candidates,
                          const std::vector<AvoidanceEntity>& nearbyEntities,
                          const NavigationGrid& navGrid,
                          combat::CombatSystem& combatSys,
                          entities::Enemy* owner,
                          audio::AudioSystem* audioSys)
{
    if (m_state == AIState::Dead) {
        m_desiredVelocity = Vector2{ 0.0f, 0.0f };
        return;
    }

    // 1. Update Target Memory
    m_targetMemory.Update(dt);

    // 2. Periodic Tactical AI Decision Evaluation
    m_decisionTimer -= dt;
    if (m_decisionTimer <= 0.0f) {
        m_decisionTimer = m_profile.decisionInterval;
        float hpPct = (maxHp > 0.0f) ? (currentHp / maxHp) : 1.0f;
        ExecuteDecision(currentPos, hpPct, candidates, navGrid, combatSys, owner, audioSys);
    }

    // 3. Compute Separation Avoidance Force
    Vector2 avoidanceForce = m_avoidance.ComputeAvoidanceForce(m_entityId, currentPos, nearbyEntities);

    // 4. State-Specific Path & Movement Execution
    float moveSpeed = (owner ? owner->GetStats().walkSpeed : 60.0f);
    if (m_state == AIState::Chase || m_state == AIState::Retreat) {
        moveSpeed = (owner ? owner->GetStats().runSpeed : 110.0f);
    }

    switch (m_state) {
        case AIState::Idle:
            m_desiredVelocity = Vector2{ 0.0f, 0.0f };
            break;

        case AIState::Patrol: {
            bool targetChanged = m_patrol.Update(currentPos, dt);
            if (targetChanged || !m_movement.HasActivePath()) {
                UpdatePathToTarget(currentPos, m_patrol.GetCurrentTargetPosition(), navGrid);
            }

            if (m_patrol.IsWaiting()) {
                m_desiredVelocity = Vector2{ 0.0f, 0.0f };
            } else {
                m_desiredVelocity = m_movement.Update(currentPos, moveSpeed, dt, avoidanceForce);
                m_desiredFacing = m_movement.GetDesiredFacing();
            }
            break;
        }

        case AIState::Chase: {
            m_pursuitTimer += dt;
            m_pathTimer -= dt;

            // Throttled path recalculation
            if (m_pathTimer <= 0.0f || !m_movement.HasActivePath()) {
                m_pathTimer = m_profile.pathfindingInterval;
                UpdatePathToTarget(currentPos, m_targetPos, navGrid);
            }

            m_desiredVelocity = m_movement.Update(currentPos, moveSpeed, dt, avoidanceForce);
            m_desiredFacing = m_movement.GetDesiredFacing();

            // Check if pursuit duration expired without seeing target
            if (m_pursuitTimer >= m_profile.pursuitDuration) {
                SetState(AIState::Investigate);
                m_investigate.StartInvestigation(m_targetPos, m_profile.investigateDuration);
            }
            break;
        }

        case AIState::Attack:
            m_desiredVelocity = Vector2{ 0.0f, 0.0f };
            break;

        case AIState::Retreat: {
            // Flee direction away from target position
            float dx = currentPos.x - m_targetPos.x;
            float dy = currentPos.y - m_targetPos.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 0.001f) {
                Vector2 fleeDir{ dx / dist, dy / dist };
                Vector2 fleeGoal{ currentPos.x + fleeDir.x * 120.0f, currentPos.y + fleeDir.y * 120.0f };
                fleeGoal = navGrid.GetNearestWalkablePosition(fleeGoal, 64.0f);

                m_pathTimer -= dt;
                if (m_pathTimer <= 0.0f || !m_movement.HasActivePath()) {
                    m_pathTimer = m_profile.pathfindingInterval;
                    UpdatePathToTarget(currentPos, fleeGoal, navGrid);
                }

                m_desiredVelocity = m_movement.Update(currentPos, moveSpeed, dt, avoidanceForce);
                m_desiredFacing = m_movement.GetDesiredFacing();
            } else {
                m_desiredVelocity = Vector2{ 0.0f, 0.0f };
            }
            break;
        }

        case AIState::Investigate: {
            m_investigate.Update(currentPos, dt);
            if (m_investigate.IsComplete()) {
                SetState(AIState::Return);
            } else if (!m_investigate.HasArrived()) {
                m_desiredVelocity = m_movement.Update(currentPos, moveSpeed, dt, avoidanceForce);
                m_desiredFacing = m_movement.GetDesiredFacing();
            } else {
                m_desiredVelocity = Vector2{ 0.0f, 0.0f };
                m_desiredFacing = m_investigate.GetSearchFacing();
            }
            break;
        }

        case AIState::Return: {
            m_pathTimer -= dt;
            if (m_pathTimer <= 0.0f || !m_movement.HasActivePath()) {
                m_pathTimer = 1.0f;
                UpdatePathToTarget(currentPos, m_spawnOrigin, navGrid);
            }

            m_desiredVelocity = m_movement.Update(currentPos, moveSpeed, dt, avoidanceForce);
            m_desiredFacing = m_movement.GetDesiredFacing();

            float dSpawnX = currentPos.x - m_spawnOrigin.x;
            float dSpawnY = currentPos.y - m_spawnOrigin.y;
            if (std::sqrt(dSpawnX * dSpawnX + dSpawnY * dSpawnY) <= 24.0f) {
                SetState(AIState::Patrol);
            }
            break;
        }

        default:
            m_desiredVelocity = Vector2{ 0.0f, 0.0f };
            break;
    }

    // 5. Stuck Detection & Automatic Replanning
    if (m_stuckDetector.Update(currentPos, m_desiredVelocity, dt)) {
        if (!m_stuckDetector.HasExceededMaxRetries()) {
            if (m_state == AIState::Chase) {
                UpdatePathToTarget(currentPos, m_targetPos, navGrid);
            } else if (m_state == AIState::Patrol) {
                UpdatePathToTarget(currentPos, m_patrol.GetCurrentTargetPosition(), navGrid);
            }
        } else {
            // Exceeded retries, temporarily pause or return to spawn
            SetState(AIState::Return);
            m_stuckDetector.Reset(currentPos);
        }
    }
}

} // namespace wildbound::ai

