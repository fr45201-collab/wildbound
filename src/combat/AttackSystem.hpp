#pragma once

#include "CombatTypes.hpp"
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace wildbound::combat {

struct AttackDefinition {
    std::string id;
    std::string name;
    DamageType damageType = DamageType::Physical;
    float baseDamage = 20.0f;
    float range = 32.0f;
    float cooldown = 0.5f;
    float hitboxWidth = 32.0f;
    float hitboxHeight = 24.0f;
    float forwardOffset = 20.0f;
    std::string animationPrefix = "attack";
};

class AttackSystem {
public:
    AttackSystem() = default;

    void RegisterAttack(const AttackDefinition& def);
    bool LoadAttacksFromJson(const std::string& filepath);

    const AttackDefinition* GetAttack(const std::string& attackId) const;
    const std::unordered_map<std::string, AttackDefinition>& GetAllAttacks() const { return m_attacks; }

    // Cooldown management
    void Update(float dt);
    bool CanAttack(const std::string& attackId) const;
    bool ExecuteAttack(const std::string& attackId);

    float GetCooldownRemaining() const { return m_cooldownRemaining; }
    float GetCurrentAttackCooldown() const { return m_lastCooldownTotal; }
    const std::string& GetActiveAttackId() const { return m_activeAttackId; }

    void ResetCooldown() {
        m_cooldownRemaining = 0.0f;
        m_activeAttackId.clear();
    }

private:
    std::unordered_map<std::string, AttackDefinition> m_attacks;
    float m_cooldownRemaining = 0.0f;
    float m_lastCooldownTotal = 0.0f;
    std::string m_activeAttackId;
};

} // namespace wildbound::combat

