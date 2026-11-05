#pragma once

#include <string>
#include <cstdint>

namespace wildbound::items {

/**
 * Represents a runtime instance of an item in the game world, inventory, or loot container.
 * Decouples mutable instance state (instance ID, quantity, durability) from static item definitions.
 */
struct ItemInstance {
    uint64_t instanceId = 0;
    std::string definitionId;
    int quantity = 1;
    float durability = 100.0f; // 0.0 to 100.0%

    [[nodiscard]] bool IsValid() const {
        return !definitionId.empty() && quantity > 0;
    }

    [[nodiscard]] bool CanStackWith(const ItemInstance& other, int maxStackSize) const {
        return definitionId == other.definitionId &&
               quantity < maxStackSize &&
               other.quantity > 0;
    }
};

} // namespace wildbound::items

