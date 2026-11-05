#pragma once

#include "CombatTypes.hpp"
#include "CombatEvents.hpp"
#include <memory>

namespace wildbound::combat {

class Combatant;

struct DamageRequest {
    CombatantId sourceId = 0;
    CombatantId targetId = 0;
    DamageType damageType = DamageType::Physical;
    float baseDamage = 0.0f;
    float damageMultiplier = 1.0f;
    float criticalChance = 0.0f; // 0.0 to 1.0
    float criticalMultiplier = 1.5f;
    Vector2 impactPoint{ 0.0f, 0.0f };
};

struct DamageCalculationResult {
    bool valid = false;
    float rawDamage = 0.0f;
    float finalDamage = 0.0f;
    bool isCritical = false;
    DamageType damageType = DamageType::Physical;
};

class DamageSystem {
public:
    DamageSystem() = default;

    // Centralized calculation
    DamageCalculationResult CalculateDamage(const DamageRequest& request, const Combatant* target = nullptr) const;

    // Apply calculated damage to a target combatant
    bool ApplyDamage(const DamageRequest& request, Combatant* target, CombatEventDispatcher* dispatcher = nullptr);
};

} // namespace wildbound::combat

