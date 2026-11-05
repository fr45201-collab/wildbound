#include "DamageSystem.hpp"
#include "Combatant.hpp"
#include <algorithm>

namespace wildbound::combat {

DamageCalculationResult DamageSystem::CalculateDamage(const DamageRequest& request, const Combatant* target) const {
    DamageCalculationResult res;
    res.damageType = request.damageType;

    if (request.baseDamage <= 0.0f) {
        res.valid = false;
        return res;
    }

    if (target && !target->IsAlive()) {
        res.valid = false;
        return res;
    }

    res.valid = true;
    res.rawDamage = request.baseDamage * std::max(0.0f, request.damageMultiplier);

    // Critical hit check
    if (request.criticalChance > 0.0f && request.criticalMultiplier > 1.0f) {
        // Deterministic or probability check (can be expanded)
        res.isCritical = (request.criticalChance >= 1.0f);
        if (res.isCritical) {
            res.rawDamage *= request.criticalMultiplier;
        }
    }

    // Armor defense mitigation (Phase 19)
    if (target) {
        float reduction = target->GetDamageReduction();
        if (reduction > 0.0f) {
            res.rawDamage *= std::clamp(1.0f - reduction, 0.05f, 1.0f);
        }
    }

    res.finalDamage = std::max(0.0f, res.rawDamage);
    return res;
}

bool DamageSystem::ApplyDamage(const DamageRequest& request, Combatant* target, CombatEventDispatcher* dispatcher) {
    if (!target || !target->IsAlive()) {
        return false;
    }

    DamageCalculationResult calc = CalculateDamage(request, target);
    if (!calc.valid || calc.finalDamage <= 0.0f) {
        return false;
    }

    // Apply to target's health
    bool applied = target->GetHealth().ApplyDamage(calc.finalDamage);
    if (!applied) {
        return false;
    }

    // Construct damage event
    DamageEvent dmgEvt;
    dmgEvt.attackerId = request.sourceId;
    dmgEvt.targetId = target->GetCombatantId();
    dmgEvt.damageType = request.damageType;
    dmgEvt.rawDamage = calc.rawDamage;
    dmgEvt.finalDamage = calc.finalDamage;
    dmgEvt.isCritical = calc.isCritical;
    dmgEvt.hitPosition = request.impactPoint;
    dmgEvt.timestamp = static_cast<float>(GetTime());

    target->OnDamageReceived(dmgEvt);

    if (dispatcher) {
        dispatcher->Dispatch(dmgEvt);
    }

    // Check if damage caused death
    if (target->GetHealth().IsDead()) {
        DeathEvent deathEvt;
        deathEvt.victimId = target->GetCombatantId();
        deathEvt.killerId = request.sourceId;
        deathEvt.deathPosition = target->GetCombatPosition();
        deathEvt.timestamp = static_cast<float>(GetTime());

        target->OnDeath(deathEvt);

        if (dispatcher) {
            dispatcher->Dispatch(deathEvt);
        }
    }

    return true;
}

} // namespace wildbound::combat

