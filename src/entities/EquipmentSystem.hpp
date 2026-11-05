#pragma once

#include "../items/ItemInstance.hpp"
#include "../weapons/WeaponData.hpp"
#include "../weapons/WeaponInstance.hpp"
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <memory>

namespace wildbound::items {
    class ItemManager;
    class Inventory;
}

namespace wildbound::weapons {
    class WeaponManager;
    struct WeaponConfig;
}

namespace wildbound::entities {

class ModularCharacter;
class Player;

enum class EquipmentSlot {
    Head = 0,
    Chest,
    Legs,
    Feet,
    Face,
    Back,
    Weapon,
    MainHand = Weapon,
    OffHand = 7
};

inline std::string_view EquipmentSlotToString(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::Head:    return "head";
        case EquipmentSlot::Chest:   return "chest";
        case EquipmentSlot::Legs:    return "legs";
        case EquipmentSlot::Feet:    return "feet";
        case EquipmentSlot::Face:    return "face";
        case EquipmentSlot::Back:    return "back";
        case EquipmentSlot::MainHand: return "mainhand";
        case EquipmentSlot::OffHand: return "offhand";
        default:                     return "unknown";
    }
}

inline EquipmentSlot StringToEquipmentSlot(std::string_view str) {
    if (str == "head" || str == "hat" || str == "headwear") return EquipmentSlot::Head;
    if (str == "chest" || str == "armor" || str == "shirt") return EquipmentSlot::Chest;
    if (str == "legs" || str == "pants")                    return EquipmentSlot::Legs;
    if (str == "feet" || str == "shoes" || str == "boots")  return EquipmentSlot::Feet;
    if (str == "face" || str == "mask" || str == "goggles") return EquipmentSlot::Face;
    if (str == "back" || str == "backpack")                 return EquipmentSlot::Back;
    if (str == "offhand" || str == "secondary" || str == "shield") return EquipmentSlot::OffHand;
    if (str == "weapon" || str == "mainhand" || str == "primary" ||
        str == "sword" || str == "bow")                     return EquipmentSlot::MainHand;
    return EquipmentSlot::Chest;
}

/**
 * Aggregate Stat Modifiers Derived From Equipped Items.
 */
struct EquipmentStatModifiers {
    float totalDefense = 0.0f;
    float damageReduction = 0.0f;      // Normalized percentage [0.0, 1.0]
    float healthBonus = 0.0f;          // Bonus max health
    float speedModifier = 1.0f;        // Movement speed scaling factor
    float attackSpeedModifier = 1.0f;  // Attack rate scaling factor
};

/**
 * Data-Driven Player & Character Equipment Subsystem.
 * Tracks equipped armor pieces, weapons (MainHand and OffHand), calculates defense/stat modifiers,
 * manages active weapon switching, and synchronizes with ModularCharacter paperdoll rendering layers.
 */
class EquipmentSystem {
public:
    EquipmentSystem();
    ~EquipmentSystem() = default;

    // Equip / Unequip
    bool Equip(EquipmentSlot slot,
               const items::ItemInstance& item,
               const items::ItemManager* itemMgr = nullptr,
               ModularCharacter* modularChar = nullptr,
               const weapons::WeaponManager* weaponMgr = nullptr);

    bool Equip(const std::string& definitionId,
               const items::ItemManager& itemMgr,
               Player* player = nullptr,
               const weapons::WeaponManager* weaponMgr = nullptr);

    bool Unequip(EquipmentSlot slot,
                 items::Inventory* inventory = nullptr,
                 ModularCharacter* modularChar = nullptr);

    [[nodiscard]] const std::optional<items::ItemInstance>& GetEquipped(EquipmentSlot slot) const;
    [[nodiscard]] bool HasEquipped(EquipmentSlot slot) const;

    // Weapon Switching
    bool SwitchWeapon(EquipmentSlot targetSlot,
                      ModularCharacter* modularChar = nullptr,
                      const weapons::WeaponManager* weaponMgr = nullptr);

    bool ToggleActiveWeapon(ModularCharacter* modularChar = nullptr,
                            const weapons::WeaponManager* weaponMgr = nullptr);

    [[nodiscard]] EquipmentSlot GetActiveWeaponSlot() const { return m_activeWeaponSlot; }
    [[nodiscard]] const weapons::WeaponConfig* GetEquippedWeaponConfig() const;
    [[nodiscard]] const weapons::WeaponConfig* GetWeaponConfig(EquipmentSlot slot) const;
    [[nodiscard]] const weapons::WeaponInstance* GetEquippedWeaponInstance(EquipmentSlot slot) const;
    [[nodiscard]] const weapons::WeaponInstance* GetActiveWeaponInstance() const;

    // Combat & Stat Modifiers
    [[nodiscard]] const EquipmentStatModifiers& GetStatModifiers() const { return m_modifiers; }
    [[nodiscard]] float GetTotalDefense() const { return m_modifiers.totalDefense; }
    [[nodiscard]] float GetDamageReduction() const { return m_modifiers.damageReduction; }
    [[nodiscard]] float GetHealthBonus() const { return m_modifiers.healthBonus; }
    [[nodiscard]] float GetSpeedModifier() const { return m_modifiers.speedModifier; }
    [[nodiscard]] float GetAttackSpeedModifier() const { return m_modifiers.attackSpeedModifier; }

    void RecalculateStats(const items::ItemManager* itemMgr);
    void Clear();

private:
    void SyncModularWeapon(ModularCharacter* modularChar, const std::string& weaponDefId);

    std::unordered_map<EquipmentSlot, items::ItemInstance> m_equippedSlots;
    std::unordered_map<EquipmentSlot, weapons::WeaponInstance> m_weaponInstances;
    std::unordered_map<EquipmentSlot, const weapons::WeaponConfig*> m_weaponConfigs;

    EquipmentSlot m_activeWeaponSlot = EquipmentSlot::MainHand;
    EquipmentStatModifiers m_modifiers;
};

} // namespace wildbound::entities
