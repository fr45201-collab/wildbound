# Ammunition and Reloading Architecture — Wildbound Phase 24

## 1. Design & Core Philosophy

In Wildbound's fantasy setting, modern firearms, magazines, clips, and reload mechanics do not exist. Instead, ranged combat relies on **physical ammunition stacks** (e.g. `arrow_iron_001`) stored in the player's inventory and drawn dynamically per shot.

---

## 2. Ammunition Inventory Model

1. **Stacking Properties**:
   - Item ID: `arrow_iron_001`.
   - Max Stack Size: `100`.
   - Weight: negligible / standard stack rules.
   - Category: `Consumable` / `Ammunition`.
2. **Dynamic Ammo Counting**:
   To render HUD ammo counters and determine firing capability, `Player::GetAmmoCount(const std::string& ammoId)` scans all inventory slots:
   ```cpp
   int Player::GetAmmoCount(const std::string& ammoId) const {
       if (!m_inventory) return 0;
       int total = 0;
       for (const auto& item : m_inventory->GetItems()) {
           if (item.definitionId == ammoId || 
               (ammoId.empty() && item.definitionId.rfind("arrow_", 0) == 0)) {
               total += item.quantity;
           }
       }
       return total;
   }
   ```
3. **Consumption on Fire**:
   When a bow shot is successfully released (`ReleaseDraw()` $\ge 0.20$):
   ```cpp
   if (player->GetAmmoCount("arrow_iron_001") > 0) {
       player->GetInventory()->RemoveItem("arrow_iron_001", 1);
   } else {
       // Shot cancelled or Dry-fire click
       AudioManager::PlaySound("ui_error");
       return false;
   }
   ```

---

## 3. Dry Fire Prevention & Player Feedback

- **Bow Draw Guard**: Attempting to draw a bow with 0 arrows immediately fails with an audio feedback cue and log message.
- **HUD Indicator**: When equipped weapon is `WeaponType::Ranged`, HUD displays:
  - Total arrows remaining in inventory.
  - Green count when $> 10$.
  - Yellow warning when $\le 10$.
  - Red flashing `NO AMMO` warning when $0$.

