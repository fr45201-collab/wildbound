#pragma once

#include "CombatTypes.hpp"
#include "Combatant.hpp"
#include "CombatRules.hpp"
#include "CombatEvents.hpp"
#include "DamageSystem.hpp"
#include "AttackSystem.hpp"
#include "HitDetection.hpp"
#include "DeathHandler.hpp"
#include "ProjectileSystem.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::combat {

class CombatSystem {
public:
    CombatSystem();
    ~CombatSystem() = default;

    bool Initialize(const std::string& rulesPath = "data/combat/combat_rules.json",
                    const std::string& attacksPath = "data/combat/attacks.json");
    void Update(float dt, audio::AudioSystem* audioSys = nullptr);

    // Combatant Management
    void RegisterCombatant(Combatant* combatant);
    void UnregisterCombatant(CombatantId id);
    Combatant* GetCombatant(CombatantId id) const;
    const std::unordered_map<CombatantId, Combatant*>& GetAllCombatants() const { return m_combatants; }

    // Centralized Attack Execution
    // Validates attacker state, cooldown, computes directional hitbox, tests hurtboxes, applies damage
    bool ProcessAttack(Combatant* attacker, const std::string& attackId, graphics::animation::Direction facing);

    // Spatial Queries for AI & targeting
    std::vector<Combatant*> QueryTargetsInRadius(Vector2 center, float radius, Faction friendlyFaction) const;
    Combatant* FindClosestTarget(Vector2 center, float radius, Faction friendlyFaction) const;

    // Subsystem Access
    CombatRules& GetRules() { return m_rules; }
    const CombatRules& GetRules() const { return m_rules; }

    DamageSystem& GetDamageSystem() { return m_damageSystem; }
    DeathHandler& GetDeathHandler() { return m_deathHandler; }
    CombatEventDispatcher& GetEvents() { return m_dispatcher; }
    const AttackSystem& GetSharedAttacks() const { return m_sharedAttacks; }
    ProjectileSystem& GetProjectileSystem() { return m_projectiles; }
    const ProjectileSystem& GetProjectileSystem() const { return m_projectiles; }

    // Debug Visualization
    const std::vector<HitDebugEntry>& GetDebugHits() const { return m_debugHits; }
    void ClearDebugHits() { m_debugHits.clear(); }
    void RenderDebug() const;

private:
    std::unordered_map<CombatantId, Combatant*> m_combatants;
    CombatRules m_rules;
    DamageSystem m_damageSystem;
    DeathHandler m_deathHandler;
    CombatEventDispatcher m_dispatcher;
    AttackSystem m_sharedAttacks;
    ProjectileSystem m_projectiles;

    std::vector<HitDebugEntry> m_debugHits;
    audio::AudioSystem* m_audioSystem = nullptr;
};

} // namespace wildbound::combat

