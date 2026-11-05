# Container and Collectible Interactions

## 1. Overview

Containers (`LootContainer`) and dropped ground collectibles (`WorldItem`) are the primary sources of inventory acquisition in Wildbound. Phase 22 unifies their discovery, interaction prompts, and transaction safety.

---

## 2. Container Interactions (`LootContainer`)

Containers represent chests, crates, and storage receptacles placed across the map.

### 2.1 Interaction Behavior
* **First Interaction**: Opens the chest lid, plays `"Interact.ChestOpen"`, and executes `LootAll`, transferring all contained items into the player's inventory up to capacity.
* **Subsequent Interactions**: If the chest is already open and empty, interacting produces feedback `"Chest is empty"`.
* **Locked Containers**: If `m_requiredKeyId` is configured, `TryOpen` verifies the key in player inventory before permitting access.

### 2.2 Transactional Integrity
`LootContainer::LootAll` iterates over contained items:
```cpp
int lootedCount = container->LootAll(player.GetInventory(), itemMgr, audioSys);
```
If the player's inventory fills up mid-transaction, untransferred items remain securely in the chest for later retrieval.

---

## 3. Collectible World Items (`WorldItem`)

Dropped weapons, potions, crafting materials, and currency rest on the ground rendered with a gentle floating bob and contact drop shadow.

### 3.1 Manual vs Automatic Pickup Policy
Wildbound supports two pickup modes via `InteractionManager::SetPickupPolicy(PickupPolicy)`:

1. **`PickupPolicy::Manual` (Default)**:
   - Items require conscious player input: entering proximity surfaces the `[E] Collect <Item Name>` prompt.
   - Pressing interact triggers transactional pickup.
2. **`PickupPolicy::Automatic`**:
   - Items are automatically collected when the player enters their pickup radius ($\approx 36\text{px}$).
   - Useful for fast-paced battle royale circles or currency collection.

### 3.2 Two-Phase Transaction Safety
Under no circumstances are items destroyed or deleted if the player's inventory cannot store them:
```cpp
int unadded = inv.AddItem(item.definitionId, item.quantity, context.itemManager);
if (unadded == 0) {
    worldItem->MarkCollected(); // Fully collected
} else if (unadded < item.quantity) {
    item.quantity = unadded;    // Partial collection: remainder stays on ground
} else {
    // Inventory full: ground item left untouched
}
```

