# Consumable and Resource Integration

## 1. Overview & Catalog Items

Wildbound's consumable and resource system integrates the 40 items produced in Phase 16 into active gameplay loops, health components, and player interaction:

- **Potions & Elixirs**: Small Health Potion, Large Health Potion, Mana Potion, Stamina Potion, Antidote.
- **Foods & Provisions**: Hearth Bread, Orchard Apple, Cooked Game Meat, Healing Herb, Wild Berries.
- **Crafting Materials**: Wood Logs, Quarry Stone, Iron Ore, Gold Ore, Coal, Hide, Leather, Cloth.
- **Natural Resources**: Spring Water, Wild Honey, Mushroom, Medicinal Lichen, Resin, Herb Seeds.
- **Special Items**: Ancient Relic Coin, Dungeon Key, Golden Emblem, Teleport Charm.

---

## 2. Consumable Consumption Pipeline

```
       Player Input ('USE' in UI or Hotbar 1-4)
                         |
                         v
             Player::UseConsumable()
                         |
      +------------------+------------------+
      |                                     |
  Health Potion / Food              Stamina Potion
      |                                     |
      v                                     v
  HealthComponent::Heal()           Player::SetStamina()
  - Adds healAmount                 - Restores stamina
  - Clamped to maxHealth            - Clamped to maxStamina
      |                                     |
      +------------------+------------------+
                         |
                         v
             Inventory::RemoveItem(slot, 1)
             - Decrements stack by 1
             - Clears slot when empty
                         |
                         v
             AudioSystem::DispatchEvent("Item.Consume")
```

---

## 3. Configured Consumable Effects

| Item ID | Display Name | Category | Primary Effect | Value | Stack Size |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `con_health_potion_small_001` | Small Health Potion | Consumable | Restore Health | +50.0 HP | 10 |
| `con_health_potion_large_001` | Large Health Potion | Consumable | Restore Health | +100.0 HP | 10 |
| `con_bread_001` | Bread | Consumable | Restore Health | +15.0 HP | 10 |
| `con_apple_001` | Apple | Consumable | Restore Health | +20.0 HP | 10 |
| `con_cooked_meat_001` | Cooked Meat | Consumable | Restore Health | +35.0 HP | 10 |
| `con_healing_herb_001` | Healing Herb | Consumable | Restore Health | +25.0 HP | 10 |
| `con_stamina_potion_001` | Stamina Potion | Consumable | Restore Stamina | +50.0 SP | 10 |
| `con_antidote_001` | Antidote | Consumable | Cure Poison | Cures status | 10 |

---

## 4. Verification & Tests

Covered in [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp):
- Consuming `con_health_potion_small_001` restores 50 HP (verified from 40 to 90 HP).
- Stack decrements from 3 to 2.
- Consuming `con_bread_001` restores 15 HP, successfully clamping to 100 max health.
- Stack decrements to 0, resetting inventory slot to empty.
- Audio event `"Item.Consume"` dispatched.

