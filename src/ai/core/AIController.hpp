#pragma once

#include "AITypes.hpp"
#include "AIConfiguration.hpp"
#include "../navigation/NavigationGrid.hpp"
#include "../navigation/Pathfinding.hpp"
#include "../movement/AIMovementController.hpp"
#include "../movement/ObstacleAvoidance.hpp"
#include "../movement/StuckDetector.hpp"
#include "../targeting/PerceptionSystem.hpp"
#include "../targeting/TargetMemory.hpp"
#include "../targeting/TargetSelector.hpp"
#include "../behaviours/PatrolBehaviour.hpp"
#include "../behaviours/InvestigateBehaviour.hpp"
#include <raylib.h>
#include <vector>
#include <memory>

namespace wildbound::entities {
    class Enemy;
}

namespace wildbound::combat {
    class CombatSystem;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::ai {

class AIController {
public:
    AIController();
    ~AIController() = default;

    void Initialize(uint32_t entityId,
                    const CreatureAIProfile& profile,
                    Vector2 spawnPos,
                    const NavigationGrid* navGrid);

    void Update(float dt,
                Vector2 currentPos,
                float currentHp,
                float maxHp,
                const std::vector<TargetCandidate>& candidates,
                const std::vector<AvoidanceEntity>& nearbyEntities,
                const NavigationGrid& navGrid,
                combat::CombatSystem& combatSys,
                entities::Enemy* owner,
                audio::AudioSystem* audioSys = nullptr);

    // Queries
    uint32_t GetEntityId() const noexcept { return m_entityId; }
    AIState GetState() const noexcept { return m_state; }
    Vector2 GetDesiredVelocity() const noexcept { return m_desiredVelocity; }
    graphics::animation::Direction GetDesiredFacing() const noexcept { return m_desiredFacing; }
    bool WantsAttack() const noexcept { return m_wantsAttack; }
    uint32_t GetTargetId() const noexcept { return m_targetId; }
    Vector2 GetTargetPosition() const noexcept { return m_targetPos; }
    Vector2 GetSpawnOrigin() const noexcept { return m_spawnOrigin; }

    const CreatureAIProfile& GetProfile() const noexcept { return m_profile; }
    const AIMovementController& GetMovementController() const noexcept { return m_movement; }
    AIMovementController& GetMovementController() noexcept { return m_movement; }
    const TargetMemory& GetTargetMemory() const noexcept { return m_targetMemory; }
    const StuckDetector& GetStuckDetector() const noexcept { return m_stuckDetector; }
    const PatrolBehaviour& GetPatrolBehaviour() const noexcept { return m_patrol; }

    // Manual overrides & event triggers
    void SetState(AIState state);
    void ForceTarget(uint32_t targetId, Vector2 targetPos);
    void OnDamageTaken(uint32_t attackerId, float damage, audio::AudioSystem* audioSys = nullptr);
    void OnDeath();
    void SetPatrolRoute(const PatrolRoute& route);

private:
    void ExecuteDecision(Vector2 currentPos,
                         float hpPercent,
                         const std::vector<TargetCandidate>& candidates,
                         const NavigationGrid& navGrid,
                         combat::CombatSystem& combatSys,
                         entities::Enemy* owner,
                         audio::AudioSystem* audioSys);

    void UpdatePathToTarget(Vector2 currentPos, Vector2 targetPos, const NavigationGrid& navGrid);

    uint32_t m_entityId = 0;
    CreatureAIProfile m_profile;
    Vector2 m_spawnOrigin{ 0.0f, 0.0f };
    AIState m_state = AIState::Idle;

    uint32_t m_targetId = 0;
    Vector2 m_targetPos{ 0.0f, 0.0f };

    Vector2 m_desiredVelocity{ 0.0f, 0.0f };
    graphics::animation::Direction m_desiredFacing = graphics::animation::Direction::Down;
    bool m_wantsAttack = false;

    float m_decisionTimer = 0.0f;
    float m_pathTimer = 0.0f;
    float m_pursuitTimer = 0.0f;

    // Subsystems
    PerceptionSystem m_perception;
    TargetMemory m_targetMemory;
    TargetSelector m_targetSelector;
    AIMovementController m_movement;
    ObstacleAvoidance m_avoidance;
    StuckDetector m_stuckDetector;
    PatrolBehaviour m_patrol;
    InvestigateBehaviour m_investigate;

    std::unique_ptr<Pathfinding> m_pathfinding;
};

} // namespace wildbound::ai

