#include "Inventory.hpp"
#include "ItemManager.hpp"
#include "../world/World.hpp"
#include "../world/WorldItem.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::items {

Inventory::Inventory(size_t capacity) {
    m_slots.resize(capacity, std::nullopt);
}

void Inventory::SetCapacity(size_t capacity) {
    m_slots.resize(capacity, std::nullopt);
}

const std::optional<ItemInstance>& Inventory::GetSlot(size_t slotIndex) const {
    static const std::optional<ItemInstance> emptySlot = std::nullopt;
    if (slotIndex >= m_slots.size()) return emptySlot;
    return m_slots[slotIndex];
}

std::optional<ItemInstance>& Inventory::GetSlot(size_t slotIndex) {
    static std::optional<ItemInstance> dummySlot = std::nullopt;
    if (slotIndex >= m_slots.size()) return dummySlot;
    return m_slots[slotIndex];
}

bool Inventory::IsSlotEmpty(size_t slotIndex) const {
    if (slotIndex >= m_slots.size()) return true;
    return !m_slots[slotIndex].has_value() || m_slots[slotIndex]->quantity <= 0;
}

size_t Inventory::GetFilledSlotCount() const {
    size_t count = 0;
    for (const auto& slot : m_slots) {
        if (slot.has_value() && slot->quantity > 0) {
            count++;
        }
    }
    return count;
}

size_t Inventory::GetFreeSlotCount() const {
    return m_slots.size() - GetFilledSlotCount();
}

int Inventory::ResolveMaxStackSize(const std::string& definitionId, const ItemManager* itemMgr) const {
    if (itemMgr) {
        const auto* def = itemMgr->GetItem(definitionId);
        if (def) {
            return def->stackable ? def->maxStackSize : 1;
        }
    }
    // Default fallback: Currency/Materials = 99, others = 10, ammo = 30
    if (definitionId.find("col_") == 0 || definitionId.find("mat_") == 0 || definitionId.find("res_") == 0) {
        return 99;
    }
    if (definitionId.find("arrow") != std::string::npos || definitionId.find("ammo") != std::string::npos) {
        return 30;
    }
    if (definitionId.find("pot_") == 0 || definitionId.find("food_") == 0) {
        return 10;
    }
    return 1;
}

int Inventory::CalculateAvailableSpace(const std::string& definitionId, const ItemManager* itemMgr) const {
    if (definitionId.empty()) return 0;
    int maxStack = ResolveMaxStackSize(definitionId, itemMgr);
    int totalSpace = 0;

    for (const auto& slot : m_slots) {
        if (!slot.has_value() || slot->quantity <= 0) {
            totalSpace += maxStack;
        } else if (slot->definitionId == definitionId && slot->quantity < maxStack) {
            totalSpace += (maxStack - slot->quantity);
        }
    }
    return totalSpace;
}

bool Inventory::CanFitItem(const std::string& definitionId, int quantity, const ItemManager* itemMgr) const {
    if (quantity <= 0) return true;
    return CalculateAvailableSpace(definitionId, itemMgr) >= quantity;
}

int Inventory::AddItem(ItemInstance item, const ItemManager* itemMgr) {
    if (!item.IsValid()) {
        return 0;
    }

    if (item.instanceId == 0) {
        item.instanceId = m_nextInstanceId++;
    }

    int remaining = item.quantity;
    int maxStack = ResolveMaxStackSize(item.definitionId, itemMgr);

    // 1. Try to merge into existing non-full stacks if stackable
    if (maxStack > 1) {
        for (auto& slot : m_slots) {
            if (slot.has_value() && slot->definitionId == item.definitionId && slot->quantity < maxStack) {
                int space = maxStack - slot->quantity;
                int toAdd = std::min(space, remaining);
                slot->quantity += toAdd;
                remaining -= toAdd;
                if (remaining <= 0) {
                    return 0;
                }
            }
        }
    }

    // 2. Place remainder into empty slots
    for (auto& slot : m_slots) {
        if (!slot.has_value() || slot->quantity <= 0) {
            int toAdd = std::min(maxStack, remaining);
            ItemInstance newInst = item;
            newInst.instanceId = m_nextInstanceId++;
            newInst.quantity = toAdd;
            slot = newInst;
            remaining -= toAdd;
            if (remaining <= 0) {
                return 0;
            }
        }
    }

    return remaining; // Return items that did not fit
}

int Inventory::AddItemById(const std::string& definitionId, int quantity, const ItemManager* itemMgr) {
    if (definitionId.empty() || quantity <= 0) return 0;
    ItemInstance inst;
    inst.definitionId = definitionId;
    inst.quantity = quantity;
    return AddItem(inst, itemMgr);
}

bool Inventory::RemoveItem(size_t slotIndex, int quantity) {
    if (slotIndex >= m_slots.size() || quantity <= 0) return false;
    auto& slot = m_slots[slotIndex];
    if (!slot.has_value() || slot->quantity < quantity) {
        return false;
    }

    slot->quantity -= quantity;
    if (slot->quantity <= 0) {
        slot = std::nullopt;
    }
    return true;
}

bool Inventory::RemoveItemById(const std::string& definitionId, int quantity) {
    if (definitionId.empty() || quantity <= 0) return false;
    if (GetItemCount(definitionId) < quantity) {
        return false; // Transactional check: must have sufficient total quantity
    }

    int needed = quantity;
    for (auto& slot : m_slots) {
        if (slot.has_value() && slot->definitionId == definitionId) {
            int canRemove = std::min(slot->quantity, needed);
            slot->quantity -= canRemove;
            needed -= canRemove;
            if (slot->quantity <= 0) {
                slot = std::nullopt;
            }
            if (needed <= 0) {
                return true;
            }
        }
    }
    return needed <= 0;
}

bool Inventory::SwapSlots(size_t slotA, size_t slotB) {
    if (slotA >= m_slots.size() || slotB >= m_slots.size() || slotA == slotB) {
        return false;
    }
    std::swap(m_slots[slotA], m_slots[slotB]);
    return true;
}

bool Inventory::SplitStack(size_t sourceSlot, size_t targetSlot, int count) {
    if (sourceSlot >= m_slots.size() || targetSlot >= m_slots.size() || sourceSlot == targetSlot) {
        return false;
    }
    if (count <= 0) return false;

    auto& src = m_slots[sourceSlot];
    if (!src.has_value() || src->quantity <= count) {
        return false;
    }

    auto& tgt = m_slots[targetSlot];
    if (tgt.has_value()) {
        // Can only split into empty slot or identical definition
        if (tgt->definitionId != src->definitionId) {
            return false;
        }
        tgt->quantity += count;
    } else {
        ItemInstance splitInst = *src;
        splitInst.instanceId = m_nextInstanceId++;
        splitInst.quantity = count;
        tgt = splitInst;
    }

    src->quantity -= count;
    return true;
}

bool Inventory::MoveSlot(size_t fromSlot, size_t toSlot) {
    if (fromSlot >= m_slots.size() || toSlot >= m_slots.size() || fromSlot == toSlot) {
        return false;
    }
    auto& from = m_slots[fromSlot];
    if (!from.has_value()) return false;

    auto& to = m_slots[toSlot];
    if (!to.has_value()) {
        to = from;
        from = std::nullopt;
        return true;
    }

    // If same definition, try to merge
    if (to->definitionId == from->definitionId) {
        int maxStack = ResolveMaxStackSize(to->definitionId, nullptr);
        int space = maxStack - to->quantity;
        if (space > 0) {
            int toMove = std::min(space, from->quantity);
            to->quantity += toMove;
            from->quantity -= toMove;
            if (from->quantity <= 0) {
                from = std::nullopt;
            }
            return true;
        }
    }

    // Otherwise swap
    return SwapSlots(fromSlot, toSlot);
}

int Inventory::MergeStacks(size_t sourceSlot, size_t targetSlot, const ItemManager* itemMgr) {
    if (sourceSlot >= m_slots.size() || targetSlot >= m_slots.size() || sourceSlot == targetSlot) {
        return 0;
    }
    auto& src = m_slots[sourceSlot];
    auto& tgt = m_slots[targetSlot];
    if (!src.has_value() || !tgt.has_value()) return 0;
    if (src->definitionId != tgt->definitionId) return 0;

    int maxStack = ResolveMaxStackSize(tgt->definitionId, itemMgr);
    int space = maxStack - tgt->quantity;
    if (space <= 0) return 0;

    int toMerge = std::min(space, src->quantity);
    tgt->quantity += toMerge;
    src->quantity -= toMerge;
    if (src->quantity <= 0) {
        src = std::nullopt;
    }
    return toMerge;
}

world::WorldItem* Inventory::DropItem(size_t slotIndex, int quantity, world::World& world,
                                      Vector2 playerPos, Vector2 playerFacing,
                                      audio::AudioSystem* audioSys) {
    if (slotIndex >= m_slots.size() || quantity <= 0) return nullptr;
    auto& slot = m_slots[slotIndex];
    if (!slot.has_value() || slot->quantity < quantity) {
        return nullptr;
    }

    // Calculate drop position in front of player
    Vector2 facing = playerFacing;
    float len = std::sqrt(facing.x * facing.x + facing.y * facing.y);
    if (len > 0.001f) {
        facing.x /= len;
        facing.y /= len;
    } else {
        facing = { 0.0f, 1.0f }; // default down
    }
    float dropDistance = 40.0f; // Drop slightly in front of player
    Vector2 dropPos = { playerPos.x + facing.x * dropDistance, playerPos.y + facing.y * dropDistance };

    // Create item instance for world
    ItemInstance dropInst = *slot;
    dropInst.quantity = quantity;

    // Phase 1 of Transaction: Spawn in world
    world::WorldItem* spawned = world.SpawnWorldItem(dropPos, dropInst);
    if (!spawned) {
        Logger::Error("INVENTORY", "Failed to spawn dropped item in world; transaction rolled back.");
        return nullptr;
    }

    // Phase 2 of Transaction: Deduct from inventory
    slot->quantity -= quantity;
    if (slot->quantity <= 0) {
        slot = std::nullopt;
    }

    Logger::Info("INVENTORY", "Dropped " + std::to_string(quantity) + "x [" + dropInst.definitionId + "] at (" +
                 std::to_string(dropPos.x) + ", " + std::to_string(dropPos.y) + ")");

    if (audioSys) {
        audioSys->DispatchEvent("Item.Drop");
    }

    return spawned;
}

world::WorldItem* Inventory::DropAll(size_t slotIndex, world::World& world,
                                     Vector2 playerPos, Vector2 playerFacing,
                                     audio::AudioSystem* audioSys) {
    if (slotIndex >= m_slots.size()) return nullptr;
    auto& slot = m_slots[slotIndex];
    if (!slot.has_value() || slot->quantity <= 0) return nullptr;
    return DropItem(slotIndex, slot->quantity, world, playerPos, playerFacing, audioSys);
}

int Inventory::GetItemCount(const std::string& definitionId) const {
    if (definitionId.empty()) return 0;
    int total = 0;
    for (const auto& slot : m_slots) {
        if (slot.has_value() && slot->definitionId == definitionId) {
            total += slot->quantity;
        }
    }
    return total;
}

bool Inventory::HasItem(const std::string& definitionId, int count) const {
    return GetItemCount(definitionId) >= count;
}

int Inventory::FindFirstSlotWithItem(const std::string& definitionId) const {
    for (size_t i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].has_value() && m_slots[i]->definitionId == definitionId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Inventory::Clear() {
    for (auto& slot : m_slots) {
        slot = std::nullopt;
    }
}

} // namespace wildbound::items

