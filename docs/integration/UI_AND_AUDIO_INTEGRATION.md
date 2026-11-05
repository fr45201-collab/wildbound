# UI and Audio Integration

## 1. Overview & Architecture

Wildbound strictly separates player-facing UI and audio simulation:
- **UI Engine**: Custom C++/raylib UI manager ([`UIManager`](file:///c:/dev/Wildbound/src/ui/UIManager.hpp)) rendering inside the 640×360 virtual canvas scaled 2× to the 1280×720 display window.
- **Audio Engine**: Hardware miniaudio-backed [`AudioSystem`](file:///c:/dev/Wildbound/src/audio/AudioSystem.hpp) managing 4 hierarchical mixing buses (`Master`, `Music`, `SFX`, `UI`), 68 synthesized audio cues, and active biome ambient transitions.
- **Single Source of Truth**: UI screens never hold gameplay state; they observe authoritative state from `Player`, `Inventory`, and `World`, and submit mutation requests.

---

## 2. Integrated UI Screens & Controls

### 2.1 Gameplay HUD (`UIScreen::None`)
- **Health Bar**: Reflects `player->GetHealth().GetCurrentHealth() / player->GetHealth().GetMaxHealth()`. Displays formatted numerical text (e.g. `95/100`).
- **Stamina Bar**: Reflects `player->GetStamina() / player->GetMaxStamina()`.
- **Biome & Armor Badge**: Displays active biome name and aggregated armor defense rating (e.g. `Armor: 35`).
- **Hotbar (Slots 1-4)**: Displays icons, count badges, and slot numbers. Triggered via keys `[1]`, `[2]`, `[3]`, `[4]` to equip weapons or consume provisions.
- **Interaction Prompts**: Dynamic contextual world prompts (e.g. `Press [E] or [F] to Open Chest`, `Press [E] or [F] to Enter Building`).

### 2.2 Inventory Screen (`UIScreen::Inventory`)
- **Hotkey**: Toggled with `[I]` or `[Tab]`.
- **Grid Layout**: 24-slot container grid reflecting player inventory.
- **Category Tabs**: Filter slots by All, Coins/Gems, Potions/Food, Materials, Resources, Weapons/Gear.
- **Details Panel**: Shows item name, rarity tier, description, and large icon preview.
- **Action Buttons**:
  - `USE`: Invokes `player->UseConsumable()`.
  - `EQUIP`: Invokes `player->GetEquipmentSystem().Equip()`.
  - `CLOSE`: Resumes gameplay (`UIScreen::None`).

### 2.3 Wardrobe Screen (`UIScreen::Wardrobe`)
- **Hotkey**: Toggled with `[C]`.
- Allows real-time inspection and selection of cosmetic clothes, hats, goggles, pants, boots, and backpacks with live animated paperdoll preview.

### 2.4 Pause Menu (`UIScreen::PauseMenu`)
- **Hotkey**: Toggled with `[Escape]`.
- Options: `RESUME`, `SETTINGS`, `MAIN MENU`, `EXIT GAME`.

---

## 3. Audio Events & Bus Routing

| Event Name | Bus | File Asset | Gameplay Trigger |
| :--- | :--- | :--- | :--- |
| `Item.Pickup` | `SFX` | `sfx_item_pickup.wav` | Ground item collection, container looting |
| `Item.Equip` | `SFX` | `sfx_equip_armor.wav` | Equipping weapon or armor piece |
| `Item.Consume` | `SFX` | `sfx_item_consume.wav` | Using potion, bread, or apple |
| `Environment.ChestOpen` | `SFX` | `sfx_chest_open.wav` | Opening loot container |
| `Combat.SwordSlash` | `SFX` | `sfx_sword_slash.wav` | Executing sword melee attack |
| `Combat.BowShoot` | `SFX` | `sfx_bow_shoot.wav` | Firing arrow projectile |
| `Combat.BowHit` | `SFX` | `sfx_bow_hit.wav` | Arrow impact against combatant |
| `Combat.Hit` | `SFX` | `sfx_player_hurt.wav` | Damage received |
| `UI.ButtonHover` | `UI` | `sfx_button_hover.wav` | Cursor hover over button |
| `UI.ButtonClick` | `UI` | `sfx_button_click.wav` | Button press confirmation |
| `UI.TabSwitch` | `UI` | `sfx_tab_switch.wav` | Switching category tabs |
| `UI.PanelOpen` | `UI` | `sfx_panel_open.wav` | Opening inventory / wardrobe |

---

## 4. Verification & Tests

- [`tests/ui/test_ui.cpp`](file:///c:/dev/Wildbound/tests/ui/test_ui.cpp): UI screen rendering, button state management, and HUD rendering.
- [`tests/audio/test_audio.cpp`](file:///c:/dev/Wildbound/tests/audio/test_audio.cpp): Bus volume scaling, procedural WAV playback, event routing.
- [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp): Full end-to-end event dispatch verification during pickup, combat, and consumable use.

