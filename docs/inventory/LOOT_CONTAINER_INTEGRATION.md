# Wildbound — Loot Container Integration

## 1. Container Structure (`LootContainer`)
Chests, weapon caches, and barrels are represented by `LootContainer`:
- Position (`Vector2`).
- Embedded 12-slot `Inventory`.
- Locking state with required key definition ID (`m_requiredKeyId`).
- Open/Closed animation visual states.

---

## 2. Transactional `LootAll`
When a player interacts with an open container:
```cpp
int LootContainer::LootAll(items::Inventory& targetInventory, const items::ItemManager* itemMgr, audio::AudioSystem* audioSys);
```
1. Iterates over container slots sequentially.
2. For each occupied slot, calls `targetInventory.AddItem(slotItem)`.
3. Deducts only the transferred quantity from the container slot.
4. If target inventory becomes full, remaining items safely stay inside the container.
5. Returns total count of transferred items.
6. Dispatches `"Item.Pickup"` audio event.

