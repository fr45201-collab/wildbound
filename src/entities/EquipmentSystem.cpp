#include "EquipmentSystem.hpp"
#include "ModularCharacter.hpp"
#include "Player.hpp"
#include "../items/ItemManager.hpp"
#include "../items/Inventory.hpp"
#include "../weapons/WeaponManager.hpp"
#include "../core/Logger.hpp"
#include <algorithm>

using wildbound::core::Logger;

namespace wildbound::entities {

EquipmentSystem::EquipmentSystem() = default;

void EquipmentSystem::SyncModularWeapon(ModularCharacter* modularChar, const std::string& weaponDefId) {
    (void)modularChar;
    (void)weaponDefId;
    // Reserved for character held attachment sync if rendered through ModularCharacter
}

bool EquipmentSystem::Equip(EquipmentSlot slot,
                            const items::ItemInstance& item,
                            const items::ItemManager* itemMgr,
                            ModularCharacter* modularChar,
                            const weapons::WeaponManager* weaponMgr)
{
    if (!item.IsValid()) return false;

    // Equip item into internal slot
    m_equippedSlots[slot] = item;

    // If weapon slot (MainHand or OffHand), resolve weapon configuration and manufacture instance
    if (slot == EquipmentSlot::MainHand || slot == EquipmentSlot::OffHand) {
        if (weaponMgr) {
            const auto* cfg = weaponMgr->GetWeapon(item.definitionId);
            m_weaponConfigs[slot] = cfg;
            if (cfg) {
                // Determine rarity from item definition or weapon default
                weapons::WeaponRarity r = weapons::WeaponRarity::Common;
                if (itemMgr) {
                    const auto* itemDef = itemMgr->GetItem(item.definitionId);
                    if (itemDef) {
                        if (itemDef->rarityTier.find("Uncommon") != std::string::npos) r = weapons::WeaponRarity::Uncommon;
                        else if (itemDef->rarityTier.find("Rare") != std::string::npos) r = weapons::WeaponRarity::Rare;
                        else if (itemDef->rarityTier.find("Epic") != std::string::npos) r = weapons::WeaponRarity::Epic;
                        else if (itemDef->rarityTier.find("Legendary") != std::string::npos) r = weapons::WeaponRarity::Legendary;
                        else if (itemDef->rarityTier.find("Mythic") != std::string::npos) r = weapons::WeaponRarity::Mythic;
                    }
                }

                m_weaponInstances[slot] = weaponMgr->CreateInstance(cfg->id, r);
                Logger::Info("EQUIP", "Equipped weapon [" + std::string(EquipmentSlotToString(slot)) + "]: " +
                             cfg->displayName + " (" + std::string(weapons::WeaponRarityToString(r)) + ")");
            }
        } else {
            // Standalone instance creation without manager
            weapons::WeaponStats fallbackStats;
            if (item.definitionId.find("bow") != std::string::npos) {
                fallbackStats.baseDamage = 25.0f;
                fallbackStats.drawDuration = 0.45f;
                fallbackStats.projectileSpeed = 480.0f;
            } else {
                fallbackStats.baseDamage = 20.0f;
            }
            m_weaponInstances[slot] = weapons::WeaponInstance{ item.instanceId, item.definitionId, weapons::WeaponRarity::Common, fallbackStats };
        }
    } else if (modularChar) {
        // Synchronize visual layer with ModularCharacter
        CosmeticCategory cosCat = CosmeticCategory::Armor;
        switch (slot) {
            case EquipmentSlot::Head: cosCat = CosmeticCategory::Headwear; break;
            case EquipmentSlot::Chest: cosCat = CosmeticCategory::Armor; break;
            case EquipmentSlot::Legs: cosCat = CosmeticCategory::Pants; break;
            case EquipmentSlot::Feet: cosCat = CosmeticCategory::Shoes; break;
            case EquipmentSlot::Face: cosCat = CosmeticCategory::Face; break;
            case EquipmentSlot::Back: cosCat = CosmeticCategory::Back; break;
            default: break;
        }

        CosmeticItem cosItem;
        cosItem.id = item.definitionId;
        cosItem.name = item.definitionId;
        cosItem.category = cosCat;
        cosItem.spritesheetId = "sheet_" + item.definitionId;
        cosItem.textureId = "tex_" + item.definitionId;

        if (slot == EquipmentSlot::Head && (item.definitionId.find("helmet") != std::string::npos)) {
            cosItem.occlusionFlags.push_back("hair_hide_all");
        } else if (slot == EquipmentSlot::Face && (item.definitionId.find("mask") != std::string::npos)) {
            cosItem.occlusionFlags.push_back("mask_hide_goggles");
        }

        modularChar->GetEquipment().Equip(cosItem);
    }

    RecalculateStats(itemMgr);
    return true;
}

bool EquipmentSystem::Equip(const std::string& definitionId,
                            const items::ItemManager& itemMgr,
                            Player* player,
                            const weapons::WeaponManager* weaponMgr)
{
    const auto* def = itemMgr.GetItem(definitionId);
    EquipmentSlot slot = EquipmentSlot::Chest;

    if (def) {
        if (def->category == items::ItemCategory::Weapon) {
            // If MainHand occupied and OffHand free, equip in OffHand; otherwise MainHand
            if (HasEquipped(EquipmentSlot::MainHand) && !HasEquipped(EquipmentSlot::OffHand)) {
                slot = EquipmentSlot::OffHand;
            } else {
                slot = EquipmentSlot::MainHand;
            }
        } else if (def->category == items::ItemCategory::Armor || def->category == items::ItemCategory::Clothing) {
            if (definitionId.find("helm") != std::string::npos || definitionId.find("hat") != std::string::npos ||
                definitionId.find("hood") != std::string::npos || definitionId.find("cap") != std::string::npos) {
                slot = EquipmentSlot::Head;
            } else if (definitionId.find("pant") != std::string::npos || definitionId.find("trouser") != std::string::npos ||
                       definitionId.find("short") != std::string::npos) {
                slot = EquipmentSlot::Legs;
            } else if (definitionId.find("boot") != std::string::npos || definitionId.find("shoe") != std::string::npos ||
                       definitionId.find("greave") != std::string::npos) {
                slot = EquipmentSlot::Feet;
            } else if (definitionId.find("goggle") != std::string::npos || definitionId.find("mask") != std::string::npos ||
                       definitionId.find("eye") != std::string::npos) {
                slot = EquipmentSlot::Face;
            } else if (definitionId.find("pack") != std::string::npos || definitionId.find("cloak") != std::string::npos ||
                       definitionId.find("cape") != std::string::npos) {
                slot = EquipmentSlot::Back;
            } else {
                slot = EquipmentSlot::Chest;
            }
        }
    } else {
        if (definitionId.find("wpn_") != std::string::npos || definitionId.find("sword") != std::string::npos ||
            definitionId.find("bow") != std::string::npos) {
            slot = (HasEquipped(EquipmentSlot::MainHand) && !HasEquipped(EquipmentSlot::OffHand))
                   ? EquipmentSlot::OffHand : EquipmentSlot::MainHand;
        } else if (definitionId.find("helm") != std::string::npos || definitionId.find("hat") != std::string::npos) {
            slot = EquipmentSlot::Head;
        } else if (definitionId.find("pant") != std::string::npos) {
            slot = EquipmentSlot::Legs;
        } else if (definitionId.find("boot") != std::string::npos) {
            slot = EquipmentSlot::Feet;
        }
    }

    items::ItemInstance inst{ 0, definitionId, 1 };
    bool ok = Equip(slot, inst, &itemMgr, player ? &player->GetModularAppearance() : nullptr, weaponMgr);
    if (ok && (slot == EquipmentSlot::MainHand || slot == EquipmentSlot::OffHand)) {
        m_activeWeaponSlot = slot;
    }
    return ok;
}

bool EquipmentSystem::Unequip(EquipmentSlot slot,
                              items::Inventory* inventory,
                              ModularCharacter* modularChar)
{
    auto it = m_equippedSlots.find(slot);
    if (it == m_equippedSlots.end()) {
        return false;
    }

    items::ItemInstance removedItem = it->second;
    m_equippedSlots.erase(it);

    // Transfer back to inventory if provided
    if (inventory) {
        inventory->AddItem(removedItem);
    }

    if (slot == EquipmentSlot::MainHand || slot == EquipmentSlot::OffHand) {
        m_weaponConfigs.erase(slot);
        m_weaponInstances.erase(slot);
    } else if (modularChar) {
        CosmeticCategory cosCat = CosmeticCategory::Armor;
        switch (slot) {
            case EquipmentSlot::Head: cosCat = CosmeticCategory::Headwear; break;
            case EquipmentSlot::Chest: cosCat = CosmeticCategory::Armor; break;
            case EquipmentSlot::Legs: cosCat = CosmeticCategory::Pants; break;
            case EquipmentSlot::Feet: cosCat = CosmeticCategory::Shoes; break;
            case EquipmentSlot::Face: cosCat = CosmeticCategory::Face; break;
            case EquipmentSlot::Back: cosCat = CosmeticCategory::Back; break;
            default: break;
        }
        modularChar->GetEquipment().Unequip(cosCat);
    }

    RecalculateStats(nullptr);
    return true;
}

const std::optional<items::ItemInstance>& EquipmentSystem::GetEquipped(EquipmentSlot slot) const {
    static const std::optional<items::ItemInstance> emptySlot = std::nullopt;
    auto it = m_equippedSlots.find(slot);
    if (it != m_equippedSlots.end()) {
        static std::optional<items::ItemInstance> inst;
        inst = it->second;
        return inst;
    }
    return emptySlot;
}

bool EquipmentSystem::HasEquipped(EquipmentSlot slot) const {
    return m_equippedSlots.find(slot) != m_equippedSlots.end();
}

bool EquipmentSystem::SwitchWeapon(EquipmentSlot targetSlot,
                                  ModularCharacter* modularChar,
                                  const weapons::WeaponManager* weaponMgr)
{
    if (targetSlot != EquipmentSlot::MainHand && targetSlot != EquipmentSlot::OffHand) {
        return false;
    }

    m_activeWeaponSlot = targetSlot;
    (void)modularChar;
    (void)weaponMgr;
    return true;
}

bool EquipmentSystem::ToggleActiveWeapon(ModularCharacter* modularChar,
                                        const weapons::WeaponManager* weaponMgr)
{
    EquipmentSlot nextSlot = (m_activeWeaponSlot == EquipmentSlot::MainHand)
                             ? EquipmentSlot::OffHand : EquipmentSlot::MainHand;
    return SwitchWeapon(nextSlot, modularChar, weaponMgr);
}

const weapons::WeaponConfig* EquipmentSystem::GetEquippedWeaponConfig() const {
    return GetWeaponConfig(m_activeWeaponSlot);
}

const weapons::WeaponConfig* EquipmentSystem::GetWeaponConfig(EquipmentSlot slot) const {
    auto it = m_weaponConfigs.find(slot);
    if (it != m_weaponConfigs.end()) {
        return it->second;
    }
    return nullptr;
}

const weapons::WeaponInstance* EquipmentSystem::GetEquippedWeaponInstance(EquipmentSlot slot) const {
    auto it = m_weaponInstances.find(slot);
    if (it != m_weaponInstances.end()) {
        return &it->second;
    }
    return nullptr;
}

const weapons::WeaponInstance* EquipmentSystem::GetActiveWeaponInstance() const {
    return GetEquippedWeaponInstance(m_activeWeaponSlot);
}

void EquipmentSystem::RecalculateStats(const items::ItemManager* itemMgr) {
    m_modifiers.totalDefense = 0.0f;
    m_modifiers.healthBonus = 0.0f;
    m_modifiers.speedModifier = 1.0f;
    m_modifiers.attackSpeedModifier = 1.0f;

    for (const auto& [slot, item] : m_equippedSlots) {
        if (slot == EquipmentSlot::MainHand || slot == EquipmentSlot::OffHand) {
            continue;
        }

        if (itemMgr) {
            const auto* def = itemMgr->GetItem(item.definitionId);
            if (def && def->category == items::ItemCategory::Armor) {
                m_modifiers.totalDefense += def->armor.defenseRating;
                continue;
            }
        }

        // Procedural defense assignment based on item name tags
        if (item.definitionId.find("steel") != std::string::npos) {
            m_modifiers.totalDefense += (slot == EquipmentSlot::Chest) ? 35.0f : 15.0f;
            m_modifiers.healthBonus += 15.0f;
            m_modifiers.speedModifier -= 0.03f;
        } else if (item.definitionId.find("iron") != std::string::npos) {
            m_modifiers.totalDefense += (slot == EquipmentSlot::Chest) ? 25.0f : 10.0f;
            m_modifiers.healthBonus += 10.0f;
            m_modifiers.speedModifier -= 0.02f;
        } else if (item.definitionId.find("leather") != std::string::npos) {
            m_modifiers.totalDefense += (slot == EquipmentSlot::Chest) ? 12.0f : 5.0f;
            m_modifiers.healthBonus += 5.0f;
        } else {
            m_modifiers.totalDefense += 8.0f;
        }

        // Boot speed bonus
        if (slot == EquipmentSlot::Feet && item.definitionId.find("boot") != std::string::npos) {
            m_modifiers.speedModifier += 0.05f;
        }
    }

    // Clamp speed modifier to reasonable bounds
    m_modifiers.speedModifier = std::clamp(m_modifiers.speedModifier, 0.70f, 1.30f);

    // Standard diminishing return armor formula: defense / (defense + 100)
    if (m_modifiers.totalDefense > 0.0f) {
        m_modifiers.damageReduction = m_modifiers.totalDefense / (m_modifiers.totalDefense + 100.0f);
    } else {
        m_modifiers.damageReduction = 0.0f;
    }
}

void EquipmentSystem::Clear() {
    m_equippedSlots.clear();
    m_weaponInstances.clear();
    m_weaponConfigs.clear();
    m_activeWeaponSlot = EquipmentSlot::MainHand;
    m_modifiers = EquipmentStatModifiers{};
}

} // namespace wildbound::entities
