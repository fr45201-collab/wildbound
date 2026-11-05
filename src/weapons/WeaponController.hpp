#pragma once

#include "WeaponData.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>

namespace wildbound::weapons {

/**
 * Controller Managing Weapon Attack Lifecycle, Bow Draw Cycles,
 * and Single-Hit-Per-Target Melee Hitbox Validation.
 */
class WeaponController {
public:
    WeaponController() = default;
    ~WeaponController() = default;

    void Update(float dt);

    // Melee Attack Lifecycle
    [[nodiscard]] bool CanInitiateAttack() const;
    bool StartMeleeAttack(float totalCooldown = 0.5f,
                          float prepTime = 0.08f,
                          float activeTime = 0.12f,
                          float recoveryTime = 0.15f);

    [[nodiscard]] WeaponAttackState GetAttackState() const { return m_state; }
    [[nodiscard]] bool IsAttacking() const {
        return m_state == WeaponAttackState::Preparation ||
               m_state == WeaponAttackState::Active ||
               m_state == WeaponAttackState::Recovery;
    }
    [[nodiscard]] bool IsHitboxActive() const { return m_state == WeaponAttackState::Active; }
    [[nodiscard]] float GetCooldownRemaining() const;

    // Single-Hit-Per-Target Prevention
    [[nodiscard]] bool HasHitTarget(uint64_t targetId) const;
    void RecordHitTarget(uint64_t targetId);
    void ClearHitTargets() { m_targetsHitThisSwing.clear(); }

    // Ranged Bow Draw Mechanics
    void StartDraw(float maxDuration = 0.45f);
    void CancelDraw();
    [[nodiscard]] float ReleaseDraw(); // Returns normalized charge [0.0, 1.0]
    [[nodiscard]] bool IsDrawing() const { return m_isDrawing; }
    [[nodiscard]] float GetDrawProgress() const;
    [[nodiscard]] bool IsFullyDrawn() const { return GetDrawProgress() >= 1.0f; }

    // Weapon Switching Validation
    [[nodiscard]] bool CanSwitchWeapon() const { return !IsAttacking() && !m_isDrawing; }

    void Reset();

private:
    WeaponAttackState m_state = WeaponAttackState::Idle;
    float m_stateTimer = 0.0f;

    float m_prepDuration = 0.08f;
    float m_activeDuration = 0.12f;
    float m_recoveryDuration = 0.15f;
    float m_cooldownDuration = 0.20f;
    float m_cooldownRemaining = 0.0f;

    // Target deduplication per swing
    std::vector<uint64_t> m_targetsHitThisSwing;

    // Bow state
    bool m_isDrawing = false;
    float m_drawTimer = 0.0f;
    float m_maxDrawDuration = 0.45f;
};

} // namespace wildbound::weapons

