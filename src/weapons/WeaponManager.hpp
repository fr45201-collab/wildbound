#pragma once

#include "WeaponData.hpp"
#include "WeaponInstance.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::weapons {

/**
 * High-level manager for the Wildbound Weapon Asset Collection.
 * Loads weapon definitions, provides category and ID queries, validates AMS asset resolution,
 * and manufactures runtime WeaponInstance objects with data-driven stats and rarity scaling.
 */
class WeaponManager {
public:
    WeaponManager();
    ~WeaponManager();

    bool Initialize(const std::string& catalogPath = "data/weapons/weapon_catalog.json");

    [[nodiscard]] const WeaponConfig* GetWeapon(const std::string& id) const;
    [[nodiscard]] bool HasWeapon(const std::string& id) const;
    [[nodiscard]] const std::vector<WeaponConfig>& GetAllWeapons() const { return m_weapons; }
    [[nodiscard]] std::vector<const WeaponConfig*> GetWeaponsByCategory(WeaponCategory category) const;
    [[nodiscard]] size_t GetWeaponCount() const { return m_weapons.size(); }

    // Weapon Instance Manufacturing
    [[nodiscard]] WeaponInstance CreateInstance(const std::string& id,
                                                WeaponRarity rarity = WeaponRarity::Common) const;

    // Validation against AMS registry
    [[nodiscard]] bool ValidateAssets(const resources::AssetManager& assetMgr, std::vector<std::string>& outErrors) const;

private:
    void AssignDefaultStats(WeaponConfig& cfg);

    std::vector<WeaponConfig> m_weapons;
    std::unordered_map<std::string, size_t> m_weaponIndexMap;
    mutable uint64_t m_nextInstanceId = 1000;
};

} // namespace wildbound::weapons
