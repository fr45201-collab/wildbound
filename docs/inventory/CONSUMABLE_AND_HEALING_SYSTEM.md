# Wildbound — Consumable & Healing System

## 1. System Overview (`ConsumableSystem`)
The `ConsumableSystem` handles consumption logic for potions, food, herbs, and recovery supplies.

### Usage Result Codes:
```cpp
enum class ItemUseResult {
    Success,
    InvalidItem,
    NotConsumable,
    FullHealth,
    FullStamina,
    InsufficientQuantity,
    TargetDead,
    Failed
};
```

---

## 2. Gameplay Validation Rules
1. **Alive Check**: Consumables cannot be used if `player.IsAlive() == false`. Returns `ItemUseResult::TargetDead`.
2. **Category Check**: Item definition must have `category == ItemCategory::Consumable`. Returns `ItemUseResult::NotConsumable`.
3. **Over-Healing Prevention**:
   - If item restores health (`healAmount > 0.0f`) and player is already at full health (`currentHealth >= maxHealth`), the item is **not** consumed and returns `ItemUseResult::FullHealth`.
4. **Stamina Overflow Prevention**:
   - If item restores stamina (`staminaAmount > 0.0f`) and player is already at full stamina (`currentStamina >= maxStamina`), the item is **not** consumed and returns `ItemUseResult::FullStamina`.
5. **Atomic Deduction**:
   - Upon successful effect application, exactly 1 unit is deducted from the player's inventory slot.
   - Audio event `"Item.Consume"` is dispatched.

