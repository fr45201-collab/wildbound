#pragma once

#include "WeaponData.hpp"
#include <string>
#include <cstdint>

namespace wildbound::weapons {

/**
 * Runtime Weapon Instance.
 * Encapsulates unique instance state, assigned rarity tier, effective combat statistics,
 * and rarity damage scaling calculations.
 */
class WeaponInstance {
public:
    WeaponInstance() = default;
    WeaponInstance(uint64_t id,
                   std::string defId,
                   WeaponRarity r = WeaponRarity::Common,
                   const WeaponStats& baseStats = WeaponStats{});

    [[nodiscard]] bool IsValid() const { return !definitionId.empty(); }

    [[nodiscard]] uint64_t GetInstanceId() const { return instanceId; }
    [[nodiscard]] const std::string& GetDefinitionId() const { return definitionId; }
    [[nodiscard]] WeaponRarity GetRarity() const { return rarity; }
    [[nodiscard]] const WeaponStats& GetStats() const { return stats; }

    void SetRarity(WeaponRarity newRarity) { rarity = newRarity; }
    void SetStats(const WeaponStats& newStats) { stats = newStats; }
    void SetDamageBonus(float bonus) { customDamageBonus = bonus; }

    // Effective Combat Calculations
    [[nodiscard]] float GetEffectiveDamage() const;
    [[nodiscard]] float GetEffectiveCritChance() const { return stats.critChance; }
    [[nodiscard]] float GetEffectiveCritMultiplier() const { return stats.critMultiplier; }
    [[nodiscard]] float GetEffectiveAttackCooldown() const { return stats.attackCooldown; }
    [[nodiscard]] float GetEffectiveRange() const { return stats.range; }
    [[nodiscard]] float GetEffectiveDrawDuration() const { return stats.drawDuration; }
    [[nodiscard]] float GetEffectiveProjectileSpeed() const { return stats.projectileSpeed; }

public:
    uint64_t instanceId = 0;
    std::string definitionId;
    WeaponRarity rarity = WeaponRarity::Common;
    WeaponStats stats;
    float durability = 100.0f;       // Reserved
    float customDamageBonus = 0.0f;  // Flat bonus applied before rarity multiplier
};

} // namespace wildbound::weapons

