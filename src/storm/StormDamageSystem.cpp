#include "StormDamageSystem.hpp"
#include "../entities/Player.hpp"
#include "../combat/HealthComponent.hpp"
#include "../audio/AudioSystem.hpp"
#include <algorithm>

namespace wildbound::storm {

void StormDamageSystem::Reset() {
    m_damageAccumulator = 0.0f;
    m_totalDamageDealt = 0.0f;
    m_audioCooldown = 0.0f;
    m_damagedThisTick = false;
}

bool StormDamageSystem::Update(float dt, const SafeZone& safeZone, float dps, entities::Player* player,
                               audio::AudioSystem* audioSys) {
    if (!player) {
        m_damagedThisTick = false;
        return false;
    }

    if (!player->IsAlive()) {
        m_damagedThisTick = false;
        m_damageAccumulator = 0.0f;
        return false;
    }

    return Update(dt, safeZone, dps, player->GetPosition(), player->GetHealth(), audioSys);
}

bool StormDamageSystem::Update(float dt, const SafeZone& safeZone, float dps, Vector2 playerPos,
                               combat::HealthComponent& health, audio::AudioSystem* audioSys) {
    m_damagedThisTick = false;

    if (m_audioCooldown > 0.0f) {
        m_audioCooldown = std::max(0.0f, m_audioCooldown - dt);
    }

    if (!health.IsAlive() || dps <= 0.0f || dt <= 0.0f) {
        m_damageAccumulator = 0.0f;
        return false;
    }

    // Player is inside the safe zone: zero damage, reset fractional accumulator
    if (safeZone.Contains(playerPos)) {
        m_damageAccumulator = 0.0f;
        return false;
    }

    // Player is outside: accumulate fractional damage
    m_damageAccumulator += (dps * dt);

    // Apply when at least 0.1 HP accumulated (or full float application)
    if (m_damageAccumulator >= 0.1f) {
        float damageToApply = m_damageAccumulator;
        m_damageAccumulator = 0.0f;

        health.ApplyDamage(damageToApply);
        m_totalDamageDealt += damageToApply;
        m_damagedThisTick = true;

        // Trigger damage sizzle audio event (rate-limited to 1 sound per 0.5s)
        if (audioSys && m_audioCooldown <= 0.0f) {
            audioSys->PlaySound("sfx_storm_damage", 0.75f);
            m_audioCooldown = 0.5f;
        }

        return true;
    }

    return false;
}

} // namespace wildbound::storm

