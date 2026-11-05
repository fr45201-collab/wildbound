# Equipment and Armor Integration

## 1. Overview & Architecture

The [`EquipmentSystem`](file:///c:/dev/Wildbound/src/entities/EquipmentSystem.hpp) manages character gear slots, aggregates defensive ratings, applies damage mitigation to combat damage requests, and synchronizes visual layers with the [`ModularCharacter`](file:///c:/dev/Wildbound/src/entities/ModularCharacter.hpp) paperdoll system.

### Equipment Slots
- `Head`: Helmets, hoods, caps, wizard hats.
- `Chest`: Cuirasses, tunics, vests, leather coats.
- `Legs`: Armored greaves, traveler trousers, scout shorts.
- `Feet`: Leather boots, iron boots, snow boots, sandals.
- `Face`: Eyepatches, adventure goggles, steampunk goggles, cloth masks.
- `Back`: Traveler backpacks, adventurer packs, cloaks.
- `Weapon`: Swords, bows, ranged weapons.

---

## 2. Armor Defense & Damage Mitigation

### 2.1 Defense Aggregation
Each equipped armor item contributes a defense rating defined in its metadata or derived from material tier:

| Material Tier | Chest Defense | Helmet / Legs Defense | Boots / Accessories Defense |
| :--- | :--- | :--- | :--- |
| **Cloth / Fiber** | 6 | 3 | 2 |
| **Leather** | 12 | 6 | 4 |
| **Iron** | 25 | 12 | 8 |
| **Steel / Armored** | 35 | 18 | 10 |
| **Relic / Enchanted** | 50 | 25 | 15 |

$$\text{Total Defense} = \sum_{\text{slot} \in \text{EquippedSlots}} \text{Defense}(\text{slot})$$

### 2.2 Diminishing Returns Damage Reduction Formula
Incoming damage is mitigated by a standard asymptotic curve:

$$\text{Damage Reduction} = \frac{\text{Total Defense}}{\text{Total Defense} + 100}$$

$$\text{Final Damage} = \text{Raw Damage} \times (1 - \text{Damage Reduction})$$

- Defense 0: 0% reduction (full damage)
- Defense 25: $25 / 125 = 20.0\%$ reduction
- Defense 50: $50 / 150 = 33.3\%$ reduction
- Defense 100: $100 / 200 = 50.0\%$ reduction
- Defense 200: $200 / 300 = 66.7\%$ reduction

### 2.3 DamageSystem Centralization
In [`DamageSystem::CalculateDamage`](file:///c:/dev/Wildbound/src/combat/DamageSystem.cpp):
```cpp
if (target != nullptr) {
    float reduction = target->GetDamageReduction();
    result.finalDamage *= (1.0f - std::clamp(reduction, 0.0f, 0.90f));
}
```
All damage receives this centralized mitigation check before reaching entity health pools.

---

## 3. Paperdoll Visual Synchronization

When an item is equipped via `EquipmentSystem::Equip`:
1. The system determines the corresponding [`CosmeticCategory`](file:///c:/dev/Wildbound/src/entities/ModularCharacter.hpp).
2. Generates a `CosmeticItem` with appropriate spritesheet ID and occlusion flags (e.g. helmets flag `"hair_hide_all"`, masks flag `"mask_hide_goggles"`).
3. Injects the cosmetic into `ModularCharacter::GetEquipment().Equip()`.
4. Renders automatically in the four-directional walking, idle, and combat sprite cycles.

---

## 4. Verification & Tests

Covered in [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp):
- Equipping `armor_iron_cuirass` (chest) and `helm_iron_001` (head).
- Total defense calculation verified ($> 0$).
- Damage reduction percentage verified against formula.
- Raw damage request of 50.0 verified mitigated by `DamageSystem::CalculateDamage`.
- Unequipping restores slot to empty and recalculates defense dynamically.

