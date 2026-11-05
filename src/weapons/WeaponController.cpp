#include "WeaponController.hpp"

namespace wildbound::weapons {

void WeaponController::Update(float dt) {
    // 1. Process Cooldown Timer
    if (m_cooldownRemaining > 0.0f) {
        m_cooldownRemaining -= dt;
        if (m_cooldownRemaining < 0.0f) {
            m_cooldownRemaining = 0.0f;
        }
    }

    // 2. Process Bow Draw
    if (m_isDrawing) {
        m_drawTimer += dt;
        if (m_drawTimer > m_maxDrawDuration) {
            m_drawTimer = m_maxDrawDuration;
        }
    }

    // 3. Process Melee Attack State Machine
    if (m_state == WeaponAttackState::Idle) {
        return;
    }

    m_stateTimer += dt;

    switch (m_state) {
        case WeaponAttackState::Preparation:
            if (m_stateTimer >= m_prepDuration) {
                m_state = WeaponAttackState::Active;
                m_stateTimer = 0.0f;
                m_targetsHitThisSwing.clear(); // Ready to detect hits
            }
            break;

        case WeaponAttackState::Active:
            if (m_stateTimer >= m_activeDuration) {
                m_state = WeaponAttackState::Recovery;
                m_stateTimer = 0.0f;
            }
            break;

        case WeaponAttackState::Recovery:
            if (m_stateTimer >= m_recoveryDuration) {
                if (m_cooldownRemaining > 0.0f) {
                    m_state = WeaponAttackState::Cooldown;
                } else {
                    m_state = WeaponAttackState::Idle;
                }
                m_stateTimer = 0.0f;
            }
            break;

        case WeaponAttackState::Cooldown:
            if (m_cooldownRemaining <= 0.0f) {
                m_state = WeaponAttackState::Idle;
                m_stateTimer = 0.0f;
            }
            break;

        default:
            break;
    }
}

bool WeaponController::CanInitiateAttack() const {
    return m_state == WeaponAttackState::Idle && m_cooldownRemaining <= 0.0f && !m_isDrawing;
}

bool WeaponController::StartMeleeAttack(float totalCooldown,
                                       float prepTime,
                                       float activeTime,
                                       float recoveryTime)
{
    if (!CanInitiateAttack()) {
        return false;
    }

    m_prepDuration = prepTime;
    m_activeDuration = activeTime;
    m_recoveryDuration = recoveryTime;
    m_cooldownRemaining = totalCooldown;

    m_state = WeaponAttackState::Preparation;
    m_stateTimer = 0.0f;
    m_targetsHitThisSwing.clear();
    return true;
}

float WeaponController::GetCooldownRemaining() const {
    return m_cooldownRemaining;
}

bool WeaponController::HasHitTarget(uint64_t targetId) const {
    return std::find(m_targetsHitThisSwing.begin(), m_targetsHitThisSwing.end(), targetId) != m_targetsHitThisSwing.end();
}

void WeaponController::RecordHitTarget(uint64_t targetId) {
    if (!HasHitTarget(targetId)) {
        m_targetsHitThisSwing.push_back(targetId);
    }
}

void WeaponController::StartDraw(float maxDuration) {
    if (m_state != WeaponAttackState::Idle || m_cooldownRemaining > 0.0f) {
        return;
    }
    m_isDrawing = true;
    m_drawTimer = 0.0f;
    m_maxDrawDuration = (maxDuration > 0.05f) ? maxDuration : 0.45f;
}

void WeaponController::CancelDraw() {
    m_isDrawing = false;
    m_drawTimer = 0.0f;
}

float WeaponController::ReleaseDraw() {
    if (!m_isDrawing) {
        return 0.0f;
    }

    float charge = GetDrawProgress();
    m_isDrawing = false;
    m_drawTimer = 0.0f;
    return charge;
}

float WeaponController::GetDrawProgress() const {
    if (!m_isDrawing || m_maxDrawDuration <= 0.0f) {
        return 0.0f;
    }
    float p = m_drawTimer / m_maxDrawDuration;
    return (p > 1.0f) ? 1.0f : p;
}

void WeaponController::Reset() {
    m_state = WeaponAttackState::Idle;
    m_stateTimer = 0.0f;
    m_cooldownRemaining = 0.0f;
    m_targetsHitThisSwing.clear();
    m_isDrawing = false;
    m_drawTimer = 0.0f;
}

} // namespace wildbound::weapons

