# Wildbound — Inventory Slot & Stacking Specification

## 1. Slot Storage Mechanics
- Authoritative capacity: **24 slots**, modeled via `std::vector<std::optional<ItemInstance>>`.
- Slots are index-addressable: `0 <= slotIndex < 24`.
- Empty slots are represented explicitly as `std::nullopt` or instances with `quantity <= 0`.
- All inventory additions follow a deterministic two-pass algorithm:
  1. **Pass 1 (Merge)**: Scans existing non-full slots holding identical `definitionId` and merges quantity up to `maxStackSize`.
  2. **Pass 2 (Empty)**: Places any remaining count into the earliest unoccupied slots (`std::nullopt`), clamping each new slot to `maxStackSize`.
  3. If remaining quantity cannot fit after both passes, the unadded count is returned to the caller, leaving existing slots uncorrupted.

---

## 2. Stack Splitting & Merging API
### Stack Splitting:
```cpp
bool Inventory::SplitStack(size_t sourceSlot, size_t targetSlot, int count);
```
- Fails if `sourceSlot == targetSlot` or `sourceSlot >= capacity` or `targetSlot >= capacity`.
- Fails if `count <= 0` or `sourceSlot` does not have more than `count` items.
- If `targetSlot` is empty, creates a new `ItemInstance` with `count` units and decrements `sourceSlot`.
- If `targetSlot` has the same item, transfers `count` units without exceeding stack limits.

### Stack Merging:
```cpp
int Inventory::MergeStacks(size_t sourceSlot, size_t targetSlot, const ItemManager* itemMgr);
```
- Merges items from `sourceSlot` into `targetSlot` up to `maxStackSize`.
- Returns the actual quantity transferred. If `sourceSlot` is exhausted, it resets to `std::nullopt`.

