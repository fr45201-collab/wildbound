#pragma once

#include "CombatTypes.hpp"
#include <raylib.h>
#include <functional>
#include <vector>

namespace wildbound::combat {

struct DamageEvent {
    CombatantId attackerId = 0;
    CombatantId targetId = 0;
    DamageType damageType = DamageType::Physical;
    float rawDamage = 0.0f;
    float finalDamage = 0.0f;
    bool isCritical = false;
    Vector2 hitPosition{ 0.0f, 0.0f };
    float timestamp = 0.0f;
};

struct DeathEvent {
    CombatantId victimId = 0;
    CombatantId killerId = 0;
    Vector2 deathPosition{ 0.0f, 0.0f };
    float timestamp = 0.0f;
};

struct AttackEvent {
    CombatantId attackerId = 0;
    std::string attackId;
    Vector2 direction{ 0.0f, 0.0f };
    Vector2 origin{ 0.0f, 0.0f };
    float timestamp = 0.0f;
};

class CombatEventDispatcher {
public:
    using DamageCallback = std::function<void(const DamageEvent&)>;
    using DeathCallback = std::function<void(const DeathEvent&)>;
    using AttackCallback = std::function<void(const AttackEvent&)>;

    void SubscribeDamage(DamageCallback cb) { m_damageCallbacks.push_back(std::move(cb)); }
    void SubscribeDeath(DeathCallback cb) { m_deathCallbacks.push_back(std::move(cb)); }
    void SubscribeAttack(AttackCallback cb) { m_attackCallbacks.push_back(std::move(cb)); }

    void Dispatch(const DamageEvent& evt) {
        for (const auto& cb : m_damageCallbacks) cb(evt);
    }
    void Dispatch(const DeathEvent& evt) {
        for (const auto& cb : m_deathCallbacks) cb(evt);
    }
    void Dispatch(const AttackEvent& evt) {
        for (const auto& cb : m_attackCallbacks) cb(evt);
    }

    void Clear() {
        m_damageCallbacks.clear();
        m_deathCallbacks.clear();
        m_attackCallbacks.clear();
    }

private:
    std::vector<DamageCallback> m_damageCallbacks;
    std::vector<DeathCallback> m_deathCallbacks;
    std::vector<AttackCallback> m_attackCallbacks;
};

} // namespace wildbound::combat

