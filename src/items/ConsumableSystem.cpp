#include "ConsumableSystem.hpp"
#include "ItemManager.hpp"
#include "Inventory.hpp"
#include "../entities/Player.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::items {

ItemUseResult ConsumableSystem::CanUseConsumable(const entities::Player& player,
                                                 const std::string& definitionId,
                                                 const ItemManager* itemMgr) {
    if (definitionId.empty()) {
        return ItemUseResult::InvalidItem;
    }

    if (!player.GetHealth().IsAlive()) {
        return ItemUseResult::TargetDead;
    }

    float healAmount = 0.0f;
    float staminaAmount = 0.0f;
    std::string effectType = "heal";
    bool isConsumable = false;

    if (itemMgr) {
        const auto* def = itemMgr->GetItem(definitionId);
        if (!def) {
            return ItemUseResult::InvalidItem;
        }
        if (def->category != ItemCategory::Consumable) {
            return ItemUseResult::NotConsumable;
        }
        isConsumable = true;
        healAmount = def->consumable.healAmount;
        staminaAmount = def->consumable.staminaAmount;
        effectType = def->consumable.effectType;
    } else {
        // Fallback heuristic for known ID conventions
        if (definitionId.find("pot_health") != std::string::npos ||
            definitionId.find("con_health") != std::string::npos ||
            definitionId.find("food_") != std::string::npos ||
            definitionId.find("con_bread") != std::string::npos ||
            definitionId.find("con_apple") != std::string::npos ||
            definitionId.find("con_cooked_meat") != std::string::npos ||
            definitionId.find("con_healing_herb") != std::string::npos) {
            isConsumable = true;
            healAmount = 25.0f;
            effectType = "heal";
        } else if (definitionId.find("stamina") != std::string::npos) {
            isConsumable = true;
            staminaAmount = 50.0f;
            effectType = "stamina";
        }
    }

    if (!isConsumable) {
        return ItemUseResult::NotConsumable;
    }

    // Health constraint
    if (effectType == "heal" || healAmount > 0.0f) {
        if (player.GetHealth().GetCurrentHealth() >= player.GetHealth().GetMaxHealth()) {
            return ItemUseResult::FullHealth;
        }
    }

    // Stamina constraint
    if (effectType == "stamina" || staminaAmount > 0.0f) {
        if (player.GetStamina() >= player.GetMaxStamina()) {
            return ItemUseResult::FullStamina;
        }
    }

    return ItemUseResult::Success;
}

ItemUseResult ConsumableSystem::UseConsumable(entities::Player& player,
                                              size_t slotIndex,
                                              const ItemManager* itemMgr,
                                              audio::AudioSystem* audioSys) {
    auto& inv = player.GetInventory();
    const auto& optSlot = inv.GetSlot(slotIndex);
    if (!optSlot.has_value() || optSlot->quantity <= 0) {
        return ItemUseResult::InsufficientQuantity;
    }

    const std::string defId = optSlot->definitionId;
    ItemUseResult canUse = CanUseConsumable(player, defId, itemMgr);
    if (canUse != ItemUseResult::Success) {
        return canUse;
    }

    // Resolve effect values
    float healAmount = 25.0f;
    float staminaAmount = 0.0f;
    std::string effectType = "heal";

    if (itemMgr) {
        const auto* def = itemMgr->GetItem(defId);
        if (def) {
            healAmount = def->consumable.healAmount;
            staminaAmount = def->consumable.staminaAmount;
            effectType = def->consumable.effectType;
        }
    } else {
        if (defId.find("large") != std::string::npos) healAmount = 60.0f;
        else if (defId.find("small") != std::string::npos) healAmount = 25.0f;
        else if (defId.find("herb") != std::string::npos) healAmount = 10.0f;
    }

    // 1. Apply Effects
    if (effectType == "heal" || healAmount > 0.0f) {
        player.GetHealth().Heal(healAmount);
        Logger::Info("CONSUMABLE", "Player consumed [" + defId + "] restoring " +
                     std::to_string(healAmount) + " HP! (HP: " +
                     std::to_string(player.GetHealth().GetCurrentHealth()) + "/" +
                     std::to_string(player.GetHealth().GetMaxHealth()) + ")");
    }

    if (effectType == "stamina" || staminaAmount > 0.0f) {
        player.SetStamina(player.GetStamina() + staminaAmount);
        Logger::Info("CONSUMABLE", "Player consumed [" + defId + "] restoring " +
                     std::to_string(staminaAmount) + " Stamina! (SP: " +
                     std::to_string(player.GetStamina()) + "/" +
                     std::to_string(player.GetMaxStamina()) + ")");
    }

    // 2. Transactionally deduct 1 item
    bool removed = inv.RemoveItem(slotIndex, 1);
    if (!removed) {
        Logger::Warn("CONSUMABLE", "Failed to deduct item from slot #" + std::to_string(slotIndex));
        return ItemUseResult::Failed;
    }

    // 3. Audio & Visual Feedback
    if (audioSys) {
        audioSys->DispatchEvent("Item.Consume");
    }

    return ItemUseResult::Success;
}

ItemUseResult ConsumableSystem::UseConsumableById(entities::Player& player,
                                                  const std::string& definitionId,
                                                  const ItemManager* itemMgr,
                                                  audio::AudioSystem* audioSys) {
    int slot = player.GetInventory().FindFirstSlotWithItem(definitionId);
    if (slot < 0) {
        return ItemUseResult::InsufficientQuantity;
    }
    return UseConsumable(player, static_cast<size_t>(slot), itemMgr, audioSys);
}

} // namespace wildbound::items

