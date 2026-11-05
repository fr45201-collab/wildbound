#pragma once

#include "CombatTypes.hpp"
#include "HealthComponent.hpp"
#include "AttackSystem.hpp"
#include "CombatEvents.hpp"
#include "../graphics/animation/Direction.hpp"
#include <raylib.h>
#include <cstdint>

namespace wildbound::combat {

/**
 * Shared Combatant Interface.
 * Implemented by both Player and Enemy entities to participate in the unified CombatSystem.
 */
class Combatant {
public:
    virtual ~Combatant() = default;

    virtual CombatantId GetCombatantId() const = 0;
    virtual CombatantType GetCombatantType() const = 0;
    virtual Faction GetFaction() const = 0;

    virtual Vector2 GetCombatPosition() const = 0;
    virtual Rectangle GetHurtbox() const = 0;
    virtual graphics::animation::Direction GetCombatFacing() const = 0;

    virtual bool IsAlive() const = 0;
    virtual HealthComponent& GetHealth() = 0;
    virtual const HealthComponent& GetHealth() const = 0;

    virtual AttackSystem& GetAttackSystem() = 0;
    virtual const AttackSystem& GetAttackSystem() const = 0;

    // Defense & Armor mitigation (Phase 19)
    virtual float GetDefenseRating() const { return 0.0f; }
    virtual float GetDamageReduction() const { return 0.0f; }

    // Weapon Combat Scaling & Single-Hit Target Tracking (Phase 24)
    virtual float GetEquippedWeaponDamage() const { return 0.0f; }
    virtual float GetEquippedCritChance() const { return 0.05f; }
    virtual float GetEquippedCritMultiplier() const { return 1.5f; }
    virtual bool HasHitTargetThisAttack(uint64_t targetId) const { (void)targetId; return false; }
    virtual void RecordHitTargetThisAttack(uint64_t targetId) { (void)targetId; }

    virtual void OnDamageReceived(const DamageEvent& evt) = 0;
    virtual void OnDeath(const DeathEvent& evt) = 0;
};

} // namespace wildbound::combat
