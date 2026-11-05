#pragma once

#include "StormTypes.hpp"
#include <cstdint>
#include <string>

namespace wildbound::entities {
    class Player;
}

namespace wildbound::combat {
    class HealthComponent;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::storm {

/**
 * Dedicated Authoritative Storm Damage Subsystem.
 * Manages frame-rate-independent damage application, fractional damage accumulation,
 * death/elimination checking, and audio feedback when a player is trapped outside the safe zone.
 */
class StormDamageSystem {
public:
    static constexpr const char* DAMAGE_SOURCE_ID = "DAMAGE_SOURCE_CORRUPTION_STORM";

    StormDamageSystem() = default;
    ~StormDamageSystem() = default;

    void Reset();

    /**
     * Updates storm damage for a player relative to the active safe zone.
     * @param dt Frame delta time in seconds.
     * @param safeZone Current active safe zone.
     * @param dps Damage per second to apply if outside the zone.
     * @param player Target player entity.
     * @param audioSys Optional audio system for damage tick sfx.
     * @return True if damage was applied this frame.
     */
    bool Update(float dt, const SafeZone& safeZone, float dps, entities::Player* player,
                audio::AudioSystem* audioSys = nullptr);

    /**
     * Overload operating directly on HealthComponent and player world position (for headless unit testing).
     */
    bool Update(float dt, const SafeZone& safeZone, float dps, Vector2 playerPos,
                combat::HealthComponent& health, audio::AudioSystem* audioSys = nullptr);

    [[nodiscard]] float GetDamageAccumulator() const noexcept { return m_damageAccumulator; }
    [[nodiscard]] float GetTotalDamageDealt() const noexcept { return m_totalDamageDealt; }
    [[nodiscard]] bool IsPlayerDamagedThisTick() const noexcept { return m_damagedThisTick; }
    [[nodiscard]] float GetAudioCooldown() const noexcept { return m_audioCooldown; }

private:
    float m_damageAccumulator = 0.0f;
    float m_totalDamageDealt = 0.0f;
    float m_audioCooldown = 0.0f;
    bool m_damagedThisTick = false;
};

} // namespace wildbound::storm

