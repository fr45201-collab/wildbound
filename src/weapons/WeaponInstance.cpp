#include "WeaponInstance.hpp"

namespace wildbound::weapons {

WeaponInstance::WeaponInstance(uint64_t id,
                               std::string defId,
                               WeaponRarity r,
                               const WeaponStats& baseStats)
    : instanceId(id)
    , definitionId(std::move(defId))
    , rarity(r)
    , stats(baseStats)
    , durability(100.0f)
    , customDamageBonus(0.0f)
{
}

float WeaponInstance::GetEffectiveDamage() const {
    float mult = GetRarityDamageMultiplier(rarity);
    return (stats.baseDamage + customDamageBonus) * mult;
}

} // namespace wildbound::weapons

