#pragma once

#include "ItemInstance.hpp"
#include <raylib.h>
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

namespace wildbound::world {
    class World;
    class WorldItem;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::items {

class ItemManager;

/**
 * Authoritative Gameplay Inventory Subsystem.
 * Manages fixed-slot storage, transactional additions, removals, stack splitting,
 * stack merging, dropping to world, and capacity enforcement.
 */
class Inventory {
public:
    explicit Inventory(size_t capacity = 24);
    ~Inventory() = default;

    [[nodiscard]] size_t GetCapacity() const { return m_slots.size(); }
    void SetCapacity(size_t capacity);

    // Slot Queries
    [[nodiscard]] const std::optional<ItemInstance>& GetSlot(size_t slotIndex) const;
    [[nodiscard]] std::optional<ItemInstance>& GetSlot(size_t slotIndex);
    [[nodiscard]] bool IsSlotEmpty(size_t slotIndex) const;
    [[nodiscard]] size_t GetFilledSlotCount() const;
    [[nodiscard]] size_t GetFreeSlotCount() const;
    [[nodiscard]] bool IsFull() const { return GetFreeSlotCount() == 0; }

    // Capacity & Fit Queries
    [[nodiscard]] int CalculateAvailableSpace(const std::string& definitionId, const ItemManager* itemMgr = nullptr) const;
    [[nodiscard]] bool CanFitItem(const std::string& definitionId, int quantity = 1, const ItemManager* itemMgr = nullptr) const;

    // Transactional Operations
    // Returns remaining quantity that could not fit (0 on complete success)
    int AddItem(ItemInstance item, const ItemManager* itemMgr = nullptr);
    int AddItem(const std::string& definitionId, int quantity, const ItemManager& itemMgr) {
        return AddItemById(definitionId, quantity, &itemMgr);
    }
    int AddItem(const std::string& definitionId, int quantity = 1, const ItemManager* itemMgr = nullptr) {
        return AddItemById(definitionId, quantity, itemMgr);
    }
    int AddItemById(const std::string& definitionId, int quantity = 1, const ItemManager* itemMgr = nullptr);

    bool RemoveItem(size_t slotIndex, int quantity = 1);
    bool RemoveItemById(const std::string& definitionId, int quantity = 1);

    // Slot Organization & Stacking
    bool SwapSlots(size_t slotA, size_t slotB);
    bool SplitStack(size_t sourceSlot, size_t targetSlot, int count);
    int MergeStacks(size_t sourceSlot, size_t targetSlot, const ItemManager* itemMgr = nullptr);
    bool MoveSlot(size_t fromSlot, size_t toSlot);

    // Dropping (Two-Phase Transactional Commit)
    world::WorldItem* DropItem(size_t slotIndex, int quantity, world::World& world,
                               Vector2 playerPos, Vector2 playerFacing = { 0.0f, 1.0f },
                               audio::AudioSystem* audioSys = nullptr);
    world::WorldItem* DropAll(size_t slotIndex, world::World& world,
                              Vector2 playerPos, Vector2 playerFacing = { 0.0f, 1.0f },
                              audio::AudioSystem* audioSys = nullptr);

    // Balance & Search Queries
    [[nodiscard]] int GetItemCount(const std::string& definitionId) const;
    [[nodiscard]] bool HasItem(const std::string& definitionId, int count = 1) const;
    [[nodiscard]] int FindFirstSlotWithItem(const std::string& definitionId) const;

    void Clear();

private:
    [[nodiscard]] int ResolveMaxStackSize(const std::string& definitionId, const ItemManager* itemMgr) const;

    std::vector<std::optional<ItemInstance>> m_slots;
    uint64_t m_nextInstanceId = 1;
};

} // namespace wildbound::items

