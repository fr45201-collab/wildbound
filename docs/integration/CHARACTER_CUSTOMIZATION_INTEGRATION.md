# Character Customization Integration

## 1. Overview & Architecture

Wildbound features a modular multi-layered character appearance pipeline supporting 32 production cosmetic designs (Phase 8) and 50 costume and armor designs (Phase 17).

Customization is managed by [`ModularCharacter`](file:///c:/dev/Wildbound/src/entities/ModularCharacter.hpp):
- **Base Layers**: Body skin tint, facial expression, base body geometry.
- **Hair**: 5 distinct styles (short, long, messy, ponytail, curly).
- **Upper Clothing / Armor**: Tunics, leather vests, iron cuirasses, traveler shirts, mage robes.
- **Lower Clothing / Pants**: Traveler trousers, armored greaves, leather pants, scout shorts.
- **Footwear**: Leather boots, iron boots, traveler shoes, snow boots, desert sandals.
- **Headwear**: Ranger hoods, iron helmets, adventurer hats, wizard hats, caps.
- **Face Accessories**: Eyepatches, adventure goggles, steampunk goggles, snow goggles, cloth masks.
- **Backwear**: Traveler backpacks, adventurer packs, leather packs, cloaks.

---

## 2. Occlusion & Layer Ordering Rules

To prevent pixel clashing and unnatural clipping, the rendering pipeline enforces automatic occlusion:
1. **Helmets**: Flag `hair_hide_all` suppresses hair rendering under full iron helms or armored coifs.
2. **Hoods**: Flag `hair_hide_sides` suppresses flared hairstyle segments while preserving bangs.
3. **Face Masks**: Flag `mask_hide_goggles` suppresses ocular accessory overlapping.
4. **Depth Order**:
   - Backwear (order 15-20)
   - Body & Skin (order 30)
   - Pants (order 40)
   - Shoes (order 50)
   - Shirt / Armor (order 60)
   - Head & Face (order 70)
   - Hair (order 75)
   - Helmet / Hat (order 80)
   - Weapon (order 85 in Down/Side, order 18 in Up)

---

## 3. Wardrobe UI Integration

The Wardrobe screen in [`UIManager`](file:///c:/dev/Wildbound/src/ui/UIManager.hpp) allows players to preview and select cosmetics in real-time:
- **Navigation Tabs**: Clothing, Armor, Pants, Shoes, Hats, Face, Back.
- **Live Paperdoll Preview**: Renders player model at $(500, 230)$ using the active animation controller and direction.
- **Audio Feedback**: Dispatches `"UI.CosmeticApply"` on selection and `"UI.TabSwitch"` on category navigation.
- **Hotkey**: Toggled with `[C]`.

---

## 4. Verification & Tests

- [`tests/entities/test_modular_character.cpp`](file:///c:/dev/Wildbound/tests/entities/test_modular_character.cpp): Modular component slot assignment and occlusion rule tests.
- [`tests/entities/test_production_cosmetics.cpp`](file:///c:/dev/Wildbound/tests/entities/test_production_cosmetics.cpp): 32 cosmetic production spritesheet integrity tests.
- [`tests/entities/test_costumes_armor.cpp`](file:///c:/dev/Wildbound/tests/entities/test_costumes_armor.cpp): 50 Phase 17 costume design verification tests.
- [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp): Automatic synchronization between `EquipmentSystem` and `ModularCharacter` visual slots.

