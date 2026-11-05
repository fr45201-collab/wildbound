# Wildbound — Item Dropping & Transfer Transactions

## 1. Two-Phase Transactional Commit
To prevent item loss or item duplication bugs, item dropping adheres to an atomic two-phase commit:

```
[Phase 1: Reserve & Spawn]
      Inventory Slot Check (slotIndex, quantity <= slot.quantity)
                     |
                     v
      Calculate Drop Location: playerPos + playerFacing * 40.0f
                     |
                     v
      World::SpawnWorldItem(dropPos, ItemInstance{defId, quantity})
                     |
     +---------------+---------------+
     |                               |
  (Spawn OK)                    (Spawn Failed)
     |                               |
[Phase 2: Commit]               [Rollback]
Deduct from slot;               Return nullptr;
Reset slot if qty <= 0.         Inventory unaltered.
Audio: "Item.Drop"
```

---

## 2. Drop APIs
### Drop Specific Quantity:
```cpp
world::WorldItem* Inventory::DropItem(size_t slotIndex, int quantity, world::World& world,
                                      Vector2 playerPos, Vector2 playerFacing,
                                      audio::AudioSystem* audioSys);
```
- Drops specified count in front of the player.

### Drop Entire Stack:
```cpp
world::WorldItem* Inventory::DropAll(size_t slotIndex, world::World& world,
                                     Vector2 playerPos, Vector2 playerFacing,
                                     audio::AudioSystem* audioSys);
```
- Drops all items currently occupying `slotIndex`.

