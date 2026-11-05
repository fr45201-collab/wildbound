# Weapon Rarity and Tier Scaling — Wildbound Phase 24

## 1. Rarity Hierarchy and Multipliers

Wildbound implements a 6-tier rarity progression for weapons, armor, and equipment:

| Rarity Tier | Display Name | Damage Multiplier | Bonus Crit Chance | UI Color (RGB) | Typical Spawn Source |
|---|---|---|---|---|---|
| `Common` | Common | 1.00x (+0%) | +0.0% | Gray `(180, 185, 195)` | Basic ground spawns, starter chests |
| `Uncommon` | Uncommon | 1.05x (+5%) | +1.0% | Green `(80, 220, 100)` | Standard chests, camp loot |
| `Rare` | Rare | 1.10x (+10%) | +2.0% | Blue `(70, 140, 245)` | Tier-2 chests, mid-game POIs |
| `Epic` | Epic | 1.15x (+15%) | +3.0% | Purple `(180, 80, 240)` | Boss monsters, dungeon vaults |
| `Legendary` | Legendary | 1.20x (+20%) | +4.0% | Gold `(245, 195, 50)` | Air drops, high-tier ruins |
| `Mythic` | Mythic | 1.25x (+25%) | +5.0% | Red `(245, 60, 60)` | Shrinking circle final caches |

---

## 2. Dynamic Calculation in `WeaponInstance`

Rather than duplicating static definition entries for every combination of weapon $\times$ rarity, `WeaponInstance` dynamically computes scaled combat attributes:

```cpp
float WeaponInstance::GetEffectiveDamage() const {
    if (!m_config) return 0.0f;
    float mult = GetRarityDamageMultiplier(m_rarity);
    return std::round(m_config->stats.baseDamage * mult);
}

float WeaponInstance::GetEffectiveCritChance() const {
    if (!m_config) return 0.0f;
    float bonus = static_cast<float>(static_cast<int>(m_rarity)) * 0.01f;
    return std::clamp(m_config->stats.critChance + bonus, 0.0f, 1.0f);
}
```

This guarantees:
1. Base configuration tables remain immutable and clean.
2. Designers can adjust base values without having to touch all rarity tiers.
3. Rarity modifiers can easily be tuned in a single centralized lookup table (`GetRarityDamageMultiplier`).

---

## 3. String & UI Color Conversions

* `WeaponRarityToString(WeaponRarity rarity)`: returns `"Common"`, `"Uncommon"`, `"Rare"`, `"Epic"`, `"Legendary"`, `"Mythic"`.
* `StringToWeaponRarity(const std::string& str)`: case-insensitive parsing mapping strings back to enum values.
* `GetRarityColor(WeaponRarity rarity)`: returns raylib `Color` used for HUD badges, inventory tooltips, and DevUI tables.

