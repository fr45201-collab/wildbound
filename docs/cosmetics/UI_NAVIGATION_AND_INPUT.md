# UI Navigation & Input Specification (Phase 26)

## 1. Contexts & Modal State Management

Store and Wardrobe interfaces operate as modal game screens governed by `wildbound::ui::UIManager`:
- When active, screen state is `UIScreen::Wardrobe` or `UIScreen::Store`.
- Normal player movement, combat swings, and world interactions are halted.
- The virtual mouse cursor coordinates are mapped from screen window space into the 640x360 virtual canvas via `Viewport::ScreenToVirtual`.

---

## 2. Input Shortcuts & Navigation Keys

| Input Action | Primary Key | Gamepad Binding | Function |
|---|---|---|---|
| **Toggle Wardrobe** | `C` | `Back / View` | Opens or closes the Character Wardrobe. |
| **Toggle Store** | `K` | `Right Thumb Click` | Opens or closes the Cosmetics Store. |
| **Back / Exit** | `Escape` | `B / Circle` | Closes Store/Wardrobe; returns to previous menu or gameplay. |
| **Tab Switch** | `Tab` | `LB / RB` | Toggles between Store View and Wardrobe View. |
| **Rotate Preview Left** | `A` or UI Button `<` | `D-Pad Left` | Rotates preview character counter-clockwise. |
| **Rotate Preview Right** | `D` or UI Button `>` | `D-Pad Right` | Rotates preview character clockwise. |
| **Toggle Preview Walk** | UI Button `ANIM` | `Y / Triangle` | Toggles between Idle and Walk animation cycles. |

---

## 3. Audio Feedback Mapping

All UI navigation and cosmetic actions dispatch audio events via `AudioSystem`:

| Event Trigger | Audio Event Key | Sample Sound Effect | Purpose |
|---|---|---|---|
| Open Store / Wardrobe | `UI.PanelOpen` | `ui_open` | Panel sliding into view. |
| Close Store / Wardrobe | `UI.PanelClose` | `ui_close` | Panel dismissing. |
| Tab / Category Switch | `UI.TabSwitch` | `ui_tab` | Switching categories or Store/Wardrobe. |
| Hover Button / Card | `UI.ButtonHover` | `ui_hover` | Subtle click tick on card mouseover. |
| Select Cosmetic Card | `UI.ButtonClick` | `ui_click` | Card selected, preview loaded. |
| Select Color Variant | `cosmetics.variant.select` | `ui_click` | Color swatch applied to preview. |
| Equip Cosmetic | `cosmetics.equip` | `ui_equip` | Audio cue when cosmetic is equipped. |
| Unequip Cosmetic | `cosmetics.unequip` | `ui_unequip` | Audio cue when cosmetic is removed. |
| Unlock Prototype | `cosmetics.unlock.prototype` | `ui_quest_complete` | Triumphant chime on prototype unlock. |
| Apply Loadout | `cosmetics.loadout.apply` | `ui_equip` | Bulk equip sound when loadout applied. |
| Invalid Action (Locked) | `cosmetics.error` | `ui_error` | Low buzz indicating locked item cannot equip. |

