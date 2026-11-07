#include "CombatSystem.hpp"
#include "../core/Logger.hpp"
#include "../audio/AudioSystem.hpp"
#include "../entities/Enemy.hpp"
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::combat {

CombatSystem::CombatSystem() = default;

bool CombatSystem::Initialize(const std::string& rulesPath, const std::string& attacksPath) {
    m_rules = CombatRules::LoadFromFile(rulesPath);
    bool attacksLoaded = m_sharedAttacks.LoadAttacksFromJson(attacksPath);

    // Subscribe audio feedback to combat dispatcher
    m_dispatcher.SubscribeDamage([this](const DamageEvent& evt) {
        if (!m_audioSystem) return;
        auto* target = GetCombatant(evt.targetId);
        if (!target) return;

        if (target->GetCombatantType() == CombatantType::Player) {
            m_audioSystem->DispatchEvent("combat.player.hurt");
        } else {
            auto* enemy = dynamic_cast<entities::Enemy*>(target);
            if (enemy) {
                m_audioSystem->DispatchEventSpatial("creature." + enemy->GetConfig().id + ".hurt", evt.hitPosition);
            } else {
                m_audioSystem->DispatchEventSpatial("combat.enemy.hurt", evt.hitPosition);
            }
        }

        if (evt.isCritical) {
            m_audioSystem->DispatchEventSpatial("combat.critical_hit", evt.hitPosition);
        } else {
            m_audioSystem->DispatchEventSpatial("combat.impact", evt.hitPosition);
        }
    });

    m_dispatcher.SubscribeDeath([this](const DeathEvent& evt) {
        if (!m_audioSystem) return;
        auto* victim = GetCombatant(evt.victimId);
        if (!victim) return;

        if (victim->GetCombatantType() == CombatantType::Player) {
            m_audioSystem->DispatchEvent("combat.player.death");
        } else {
            auto* enemy = dynamic_cast<entities::Enemy*>(victim);
            if (enemy) {
                m_audioSystem->DispatchEventSpatial("creature." + enemy->GetConfig().id + ".death", evt.deathPosition);
            } else {
                m_audioSystem->DispatchEventSpatial("combat.enemy.death", evt.deathPosition);
            }
        }
    });

    Logger::Info("COMBAT_SYS", "CombatSystem initialized with audio subscribers. Shared attacks loaded: " +
                 std::to_string(m_sharedAttacks.GetAllAttacks().size()));
    return attacksLoaded;
}

void CombatSystem::Update(float dt, audio::AudioSystem* audioSys) {
    m_audioSystem = audioSys;

    // 1. Update active combatant cooldowns & health regeneration
    for (auto& [id, combatant] : m_combatants) {
        if (combatant && combatant->IsAlive()) {
            combatant->GetAttackSystem().Update(dt);
            combatant->GetHealth().Update(dt);
        }
    }

    // 2. Update active projectiles (Phase 19)
    m_projectiles.Update(dt, m_combatants, m_rules, m_damageSystem, &m_dispatcher, audioSys);

    // 3. Decay debug hit indicators
    for (auto it = m_debugHits.begin(); it != m_debugHits.end(); ) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f) {
            it = m_debugHits.erase(it);
        } else {
            ++it;
        }
    }
}

void CombatSystem::RegisterCombatant(Combatant* combatant) {
    if (!combatant) return;
    m_combatants[combatant->GetCombatantId()] = combatant;
}

void CombatSystem::UnregisterCombatant(CombatantId id) {
    m_combatants.erase(id);
    m_deathHandler.Remove(id);
}

Combatant* CombatSystem::GetCombatant(CombatantId id) const {
    auto it = m_combatants.find(id);
    if (it != m_combatants.end()) {
        return it->second;
    }
    return nullptr;
}

bool CombatSystem::ProcessAttack(Combatant* attacker, const std::string& attackId, graphics::animation::Direction facing) {
    if (!attacker || !attacker->IsAlive()) {
        return false;
    }

    // Find attack definition in attacker's AttackSystem or fallback to shared attacks
    const AttackDefinition* attackDef = attacker->GetAttackSystem().GetAttack(attackId);
    if (!attackDef) {
        attackDef = m_sharedAttacks.GetAttack(attackId);
    }

    if (!attackDef) {
        Logger::Warn("COMBAT_SYS", "Unknown attack ID requested: " + attackId);
        return false;
    }

    // Check cooldown
    if (!attacker->GetAttackSystem().CanAttack(attackId)) {
        return false;
    }

    // Execute cooldown on attacker
    attacker->GetAttackSystem().ExecuteAttack(attackId);

    Vector2 attackerPos = attacker->GetCombatPosition();

    // Compute directional attack hitbox
    Rectangle attackHitbox = HitDetection::ComputeAttackHitbox(attackerPos, facing, *attackDef);

    // Compute effective base damage factoring in equipped weapon and rarity scaling
    float effectiveDamage = attackDef->baseDamage;
    float weaponDmg = attacker->GetEquippedWeaponDamage();
    if (weaponDmg > 0.0f) {
        effectiveDamage = weaponDmg;
    }

    // Trigger attack sound
    if (m_audioSystem) {
        if (attacker->GetCombatantType() == CombatantType::Player) {
            if (attackId == "shoot" || attackId.find("bow") != std::string::npos) {
                m_audioSystem->DispatchEventSpatial("weapon.bow.release", attackerPos);
            } else {
                m_audioSystem->DispatchEventSpatial("weapon.sword.swing", attackerPos);
            }
        } else {
            auto* enemy = dynamic_cast<entities::Enemy*>(attacker);
            if (enemy) {
                m_audioSystem->DispatchEventSpatial("creature." + enemy->GetConfig().id + ".attack", attackerPos);
            } else {
                m_audioSystem->DispatchEventSpatial("weapon.sword.swing", attackerPos);
            }
        }
    }

    // Dispatch attack initiation event
    AttackEvent atkEvt;
    atkEvt.attackerId = attacker->GetCombatantId();
    atkEvt.attackId = attackId;
    atkEvt.direction = DirectionToVector(facing);
    atkEvt.origin = attackerPos;
    atkEvt.timestamp = static_cast<float>(GetTime());
    m_dispatcher.Dispatch(atkEvt);

    bool anyHit = false;

    // Hit detection against all registered combatants
    for (auto& [targetId, target] : m_combatants) {
        if (!target || target == attacker || !target->IsAlive()) {
            continue;
        }

        // Faction and team friendly-fire check
        if (!m_rules.CanTarget(attacker->GetCombatantId(), targetId, attacker->GetFaction(), target->GetFaction())) {
            continue;
        }

        // Single-hit-per-target prevention during this attack
        if (attacker->HasHitTargetThisAttack(targetId)) {
            continue;
        }

        Rectangle targetHurtbox = target->GetHurtbox();

        if (HitDetection::Intersects(attackHitbox, targetHurtbox)) {
            Vector2 impact = HitDetection::ComputeImpactPoint(attackHitbox, targetHurtbox);

            DamageRequest dmgReq;
            dmgReq.sourceId = attacker->GetCombatantId();
            dmgReq.targetId = targetId;
            dmgReq.damageType = attackDef->damageType;
            dmgReq.baseDamage = effectiveDamage;
            dmgReq.damageMultiplier = 1.0f;
            dmgReq.impactPoint = impact;

            attacker->RecordHitTargetThisAttack(targetId);

            m_damageSystem.ApplyDamage(dmgReq, target, &m_dispatcher);

            HitDebugEntry debugEntry;
            debugEntry.attackHitbox = attackHitbox;
            debugEntry.targetHurtbox = targetHurtbox;
            debugEntry.hitPoint = impact;
            debugEntry.didHit = true;
            debugEntry.lifetime = 0.5f;
            m_debugHits.push_back(debugEntry);

            anyHit = true;
        }
    }

    if (!anyHit) {
        // Record attack swing in debug history even on miss
        HitDebugEntry debugEntry;
        debugEntry.attackHitbox = attackHitbox;
        debugEntry.didHit = false;
        debugEntry.lifetime = 0.35f;
        m_debugHits.push_back(debugEntry);
    }

    return true;
}

std::vector<Combatant*> CombatSystem::QueryTargetsInRadius(Vector2 center, float radius, Faction friendlyFaction) const {
    std::vector<Combatant*> targets;
    float radSq = radius * radius;

    for (const auto& [id, c] : m_combatants) {
        if (!c || !c->IsAlive()) continue;

        if (m_rules.CanTarget(friendlyFaction, c->GetFaction())) {
            Vector2 pos = c->GetCombatPosition();
            float dx = pos.x - center.x;
            float dy = pos.y - center.y;
            if (dx * dx + dy * dy <= radSq) {
                targets.push_back(c);
            }
        }
    }
    return targets;
}

Combatant* CombatSystem::FindClosestTarget(Vector2 center, float radius, Faction friendlyFaction) const {
    auto targets = QueryTargetsInRadius(center, radius, friendlyFaction);
    Combatant* closest = nullptr;
    float minDistSq = radius * radius;

    for (auto* t : targets) {
        Vector2 pos = t->GetCombatPosition();
        float dx = pos.x - center.x;
        float dy = pos.y - center.y;
        float distSq = dx * dx + dy * dy;
        if (distSq < minDistSq) {
            minDistSq = distSq;
            closest = t;
        }
    }
    return closest;
}

void CombatSystem::RenderDebug() const {
    for (const auto& entry : m_debugHits) {
        if (entry.didHit) {
            DrawRectangleLinesEx(entry.attackHitbox, 1.5f, RED);
            DrawRectangleLinesEx(entry.targetHurtbox, 1.5f, GREEN);
            DrawCircle(static_cast<int>(entry.hitPoint.x), static_cast<int>(entry.hitPoint.y), 4.0f, YELLOW);
        } else {
            DrawRectangleLinesEx(entry.attackHitbox, 1.0f, ORANGE);
        }
    }
}

} // namespace wildbound::combat

