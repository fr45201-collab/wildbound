#pragma once

#include "ItemData.hpp"
#include <string>
#include <string_view>

namespace wildbound::entities {
    class Player;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::items {

class ItemManager;
class Inventory;

enum class ItemUseResult {
    Success = 0,
    InvalidItem,
    NotConsumable,
    FullHealth,
    FullStamina,
    InsufficientQuantity,
    TargetDead,
    Failed
};

inline std::string_view ItemUseResultToString(ItemUseResult result) {
    switch (result) {
        case ItemUseResult::Success:              return "Item used successfully.";
        case ItemUseResult::InvalidItem:          return "Invalid item.";
        case ItemUseResult::NotConsumable:        return "Item cannot be consumed.";
        case ItemUseResult::FullHealth:           return "Already at full health.";
        case ItemUseResult::FullStamina:          return "Already at full stamina.";
        case ItemUseResult::InsufficientQuantity: return "Not enough items.";
        case ItemUseResult::TargetDead:           return "Cannot use while fallen.";
        case ItemUseResult::Failed:               return "Failed to use item.";
        default:                                 return "Unknown item use result.";
    }
}

/**
 * Reusable data-driven Consumable Subsystem.
 * Manages item consumption rules, validates health and stamina constraints,
 * applies healing and restoration effects, and coordinates transactional inventory deduction.
 */
class ConsumableSystem {
public:
    ConsumableSystem() = default;
    ~ConsumableSystem() = default;

    /**
     * Attempts to consume an item from the specified inventory slot on a target Player.
     * Enforces health/stamina constraints and transactionally deducts 1 item upon success.
     */
    static ItemUseResult UseConsumable(entities::Player& player,
                                       size_t slotIndex,
                                       const ItemManager* itemMgr = nullptr,
                                       audio::AudioSystem* audioSys = nullptr);

    /**
     * Directly applies consumable effects by definition ID (e.g. from hotbar or quick-use).
     */
    static ItemUseResult UseConsumableById(entities::Player& player,
                                           const std::string& definitionId,
                                           const ItemManager* itemMgr = nullptr,
                                           audio::AudioSystem* audioSys = nullptr);

    /**
     * Checks whether an item can currently be used without mutating state.
     */
    static ItemUseResult CanUseConsumable(const entities::Player& player,
                                          const std::string& definitionId,
                                          const ItemManager* itemMgr = nullptr);
};

} // namespace wildbound::items

