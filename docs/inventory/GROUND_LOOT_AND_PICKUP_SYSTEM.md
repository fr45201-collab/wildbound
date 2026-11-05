# Wildbound — Ground Loot and Pickup System

## 1. Ground Representation (`WorldItem`)
Items dropped or spawned in the game world instantiate a `WorldItem` entity containing:
- Unique 64-bit ID.
- World position (`Vector2`).
- Embedded `ItemInstance` (`definitionId`, `quantity`).
- Interaction pickup radius (`m_pickupRadius = 36.0f`).
- Smooth sinusoidal floating bob offset: `offsetY = sin(timer * 3.5f) * 3.0f`.
- Elliptical contact drop shadow rendered below the sprite.

---

## 2. Pickup Policies
Wildbound supports two pickup modes integrated with Phase 22 `InteractionManager`:

### Manual Pickup (`PickupPolicy::Manual`)
- Nearby ground items register with `InteractionManager` as `InteractionCategory::Collect`.
- When the player is within range and looking towards the item, an interaction prompt `[E] Collect ItemName` is rendered.
- Pressing `E` collects the item transactionally.

### Automatic Proximity Pickup (`PickupPolicy::Automatic`)
- Evaluated per frame in `World::ProcessPickups`.
- Any item within distance `<= pickupRadius` is picked up without keypress.
- If inventory is full, the item remains on the ground and an audio warning is dispatched.

---

## 3. Audio & Event Dispatch
- Successful pickup triggers event: `"Item.Pickup"`.
- Plays `"sfx_item_pickup"`, `"sfx_coin_pickup"`, or `"sfx_gem_pickup"` according to item category.

