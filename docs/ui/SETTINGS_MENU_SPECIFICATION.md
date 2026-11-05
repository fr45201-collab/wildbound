# Settings Menu Specification

## 1. Overview & Purpose

The Settings Menu (`wildbound::ui::SettingsMenu`) provides comprehensive runtime player configuration across four core tabs:
1. **Graphics**: Display resolution, window mode, vertical synchronization, target frame rate.
2. **Audio**: Five dedicated audio buses (Master, Music, SFX, UI, Environment) and global mute.
3. **Controls**: Keybinding viewer and remapping integrated with `InputSettings`.
4. **Gameplay**: Minimap visibility, HUD display mode, floating damage number toggles.

The Settings Menu is accessible from both the `MainMenu` and the in-game `PauseMenu`.

---

## 2. Layout & Tab Architecture

- **Modal Window Dimensions**: Centered at $(X = 80, Y = 30)$, Dimensions: $480 \times 300\text{ px}$.
- **Tab Header**:
  - Four selectable tab buttons across the top $(Y = 40..65)$:
    `[ Graphics ]   [ Audio ]   [ Controls ]   [ Gameplay ]`
  - Active tab is highlighted in primary Gold (`#D4AF37`) with a solid 2px bottom accent bar.
  - Tab navigation can be performed via mouse click or Left/Right bumper buttons (`Gamepad_LB` / `Gamepad_RB`) and `Q` / `E` keyboard shortcuts.

```
+-----------------------------------------------------------------------------+
| SETTINGS                                                      [x] Close     |
| [ Graphics ]  [ Audio* ]  [ Controls ]  [ Gameplay ]                        |
+-----------------------------------------------------------------------------+
|                                                                             |
|   Master Volume     [=======================       ]  75%                   |
|   Music Volume      [=================             ]  55%                   |
|   SFX Volume        [============================== ] 100%                   |
|   UI Volume         [====================          ]  65%                   |
|   Environment Vol   [================              ]  50%                   |
|                                                                             |
|   [ ] Mute All Audio                                                        |
|                                                                             |
+-----------------------------------------------------------------------------+
| [ Reset Defaults ]                                         [ Apply & Back ] |
+-----------------------------------------------------------------------------+
```

---

## 3. Tab Implementations

### 3.1 Audio Tab
The audio configuration directly binds to `wildbound::audio::AudioSystem`:
- **Master Bus Slider**: Controls master gain $[0.0..1.0]$.
- **Music Bus Slider**: Controls background track and ambient stream gain $[0.0..1.0]$.
- **SFX Bus Slider**: Controls combat, footstep, weapon swing, and impact gain $[0.0..1.0]$.
- **UI Bus Slider**: Controls button clicks, inventory shuffling, and notification pings $[0.0..1.0]$.
- **Environment Bus Slider**: Controls weather, wind, water, and zone atmosphere $[0.0..1.0]$.
- **Mute All Toggle**: Instantly zeroes master attenuation while preserving individual slider levels.

### 3.2 Graphics Tab
- **Display Resolution**:
  - $1280 \times 720$ (Default, 720p $16:9$, $2\times$ virtual canvas).
  - $1920 \times 1080$ (1080p $16:9$, $3\times$ virtual canvas).
  - $2560 \times 1440$ (1440p $16:9$, $4\times$ virtual canvas).
- **Window Mode**: Windowed / Borderless Fullscreen.
- **VSync**: Enabled / Disabled via Raylib `SetConfigFlags(FLAG_VSYNC_HINT)`.
- **Target Frame Rate**: 60 FPS / 120 FPS / 144 FPS / Unlimited.

### 3.3 Controls Tab
Integrated directly with `wildbound::input::InputSettings`:
- Displays action names (`Move Up`, `Move Down`, `Move Left`, `Move Right`, `Attack`, `Interact`, `Inventory`, `Use Potion`, `Roll / Dodge`).
- Primary Keybinding and Secondary Binding columns.
- Remap dialog: Clicking an action listens for the next keypress or gamepad button.
- **Reset to Defaults** button: Restores default bindings from configuration schema.

### 3.4 Gameplay Tab
- **Show Minimap**: Toggle radar visibility on HUD.
- **Minimap Zoom Default**: $1.0\times / 1.5\times / 2.0\times$.
- **Floating Damage Numbers**: Toggle damage popups in world space.
- **Screen Shake Intensity**: Slider $[0.0..1.0]$ controlling trauma attenuation.

---

## 4. Audio Feedback Integration

Every setting modification dispatches audio events via `wildbound::audio::AudioSystem`:
- Tab Switch: Dispatches `ui.tab.switch` (Frequency: $480\text{ Hz}$, $0.05\text{s}$ click).
- Slider Drag: Dispatches `ui.slider.tick` (subtle haptic tick sound).
- Toggle Checked: Dispatches `ui.toggle.check`.
- Settings Saved / Applied: Dispatches `ui.save.success`.

