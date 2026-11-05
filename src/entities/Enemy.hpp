#pragma once

#include "Character.hpp"
#include "EnemyConfig.hpp"
#include "../combat/Combatant.hpp"
#include "../combat/HealthComponent.hpp"
#include "../combat/AttackSystem.hpp"
#include <string>

namespace wildbound::world {
    class World;
}

namespace wildbound::combat {
    class CombatSystem;
}

namespace wildbound::scripting {
    class LuaManager;
}

namespace wildbound::entities {

enum class AIState {
    Idle,
    Patrol,
    DetectTarget,
    Chase,
    Attack,
    Retreat,
    Dead,
    Investigate,
    Alert,
    Defend,
    Search,
    Return
};

inline std::string AIStateToString(AIState state) {
    switch (state) {
        case AIState::Idle:         return "IDLE";
        case AIState::Patrol:       return "PATROL";
        case AIState::DetectTarget: return "DETECT_TARGET";
        case AIState::Chase:        return "CHASE";
        case AIState::Attack:       return "ATTACK";
        case AIState::Retreat:      return "RETREAT";
        case AIState::Dead:         return "DEAD";
        case AIState::Investigate:  return "INVESTIGATE";
        case AIState::Alert:        return "ALERT";
        case AIState::Defend:       return "DEFEND";
        case AIState::Search:       return "SEARCH";
        case AIState::Return:       return "RETURN";
        default:                    return "UNKNOWN";
    }
}

inline AIState StringToAIState(const std::string& str) {
    if (str == "PATROL") return AIState::Patrol;
    if (str == "DETECT_TARGET") return AIState::DetectTarget;
    if (str == "CHASE") return AIState::Chase;
    if (str == "ATTACK") return AIState::Attack;
    if (str == "RETREAT") return AIState::Retreat;
    if (str == "DEAD") return AIState::Dead;
    if (str == "INVESTIGATE") return AIState::Investigate;
    if (str == "ALERT") return AIState::Alert;
    if (str == "DEFEND") return AIState::Defend;
    if (str == "SEARCH") return AIState::Search;
    if (str == "RETURN") return AIState::Return;
    return AIState::Idle;
}

class Enemy : public Character, public combat::Combatant {
public:
    Enemy(uint32_t id, const std::string& name = "Enemy", const std::string& type = "goblin");
    ~Enemy() override = default;

    void ApplyConfig(const EnemyConfig& cfg);
    void Spawn(Vector2 spawnPos);

    void Update(float dt) override;
    void UpdateWithWorld(float dt, const world::World& world, combat::CombatSystem& combatSys,
                         const resources::AssetManager& assetMgr, scripting::LuaManager* luaMgr = nullptr);

    // Collision Resolution Helper
    void ResolveMovement(Vector2 delta, const world::World& world);

    // Combatant Implementation
    combat::CombatantId GetCombatantId() const override { return GetId(); }
    combat::CombatantType GetCombatantType() const override { return combat::CombatantType::Enemy; }
    combat::Faction GetFaction() const override { return m_config.faction; }

    Vector2 GetCombatPosition() const override { return GetPosition(); }
    Rectangle GetHurtbox() const override { return GetWorldBounds(); }
    graphics::animation::Direction GetCombatFacing() const override { return GetFacing(); }

    bool IsAlive() const override { return m_health.IsAlive() && !m_pendingDestruction; }
    combat::HealthComponent& GetHealth() override { return m_health; }
    const combat::HealthComponent& GetHealth() const override { return m_health; }

    combat::AttackSystem& GetAttackSystem() override { return m_attackSystem; }
    const combat::AttackSystem& GetAttackSystem() const override { return m_attackSystem; }

    void OnDamageReceived(const combat::DamageEvent& evt) override;
    void OnDeath(const combat::DeathEvent& evt) override;

    // AI & State Queries
    AIState GetAIState() const { return m_aiState; }
    void SetAIState(AIState state) { m_aiState = state; }

    combat::CombatantId GetTargetId() const { return m_targetId; }
    void SetTargetId(combat::CombatantId id) { m_targetId = id; }

    const EnemyConfig& GetConfig() const { return m_config; }
    Vector2 GetSpawnOrigin() const { return m_spawnOrigin; }

    bool IsPendingDestruction() const { return m_pendingDestruction; }

private:
    void ExecuteAIDecision(combat::CombatSystem& combatSys, scripting::LuaManager* luaMgr);

    EnemyConfig m_config;
    combat::HealthComponent m_health{ 50.0f, 50.0f };
    combat::AttackSystem m_attackSystem;

    AIState m_aiState = AIState::Idle;
    combat::CombatantId m_targetId = 0;
    Vector2 m_spawnOrigin{ 0.0f, 0.0f };
    Vector2 m_patrolTarget{ 0.0f, 0.0f };

    float m_aiDecisionTimer = 0.0f;
    float m_patrolWaitTimer = 0.0f;
    float m_deathTimer = 0.0f;
    bool m_pendingDestruction = false;
};

} // namespace wildbound::entities

