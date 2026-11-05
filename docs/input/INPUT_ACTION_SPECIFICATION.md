# Wildbound — Input Action Specification

## 1. Action Categories & Enumeration

Wildbound groups all user interactions into seven logical action categories. Every action has a discrete integer ID, category, name string, and expected value semantics (Digital: 0 or 1; Analog: 0.0 to 1.0).

| Action ID | Logical Name | Category | Type | Default Primary | Default Alternate | Description |
|:---|:---|:---|:---|:---|:---|:---|
| 0 | `MoveUp` | Movement | Analog | Key: W | Gamepad: LS_Y- | Move character upward / north |
| 1 | `MoveDown` | Movement | Analog | Key: S | Gamepad: LS_Y+ | Move character downward / south |
| 2 | `MoveLeft` | Movement | Analog | Key: A | Gamepad: LS_X- | Move character left / west |
| 3 | `MoveRight` | Movement | Analog | Key: D | Gamepad: LS_X+ | Move character right / east |
| 4 | `Sprint` | Movement | Digital | Key: Left Shift | Gamepad: LS (Click) | Engage accelerated sprint motion |
| 5 | `Dodge` | Movement | Digital | Key: Space | Gamepad: B / Circle | Tactical evasive roll / dodge |
| 6 | `Attack` | Combat | Digital | Mouse: Left | Gamepad: RT (Trigger) | Primary weapon strike / arrow release |
| 7 | `HeavyAttack` | Combat | Digital | Mouse: Right | Gamepad: RB / R1 | Charged heavy attack / power draw |
| 8 | `Aim` | Combat | Analog | Mouse: Right | Gamepad: LT (Trigger) | Ranged aim modifier / zoom |
| 9 | `Reload` | Combat | Digital | Key: R | Gamepad: X / Square | Quiver reload / arrow restock |
| 10 | `QuickHeal` | Combat | Digital | Key: Q | Gamepad: D-Pad Up | Immediate healing item use |
| 11 | `Interact` | General | Digital | Key: E | Gamepad: X / Square | Open chests, gather, enter doors |
| 12 | `InventoryToggle` | Inventory | Digital | Key: I | Gamepad: Back / Select | Toggle full backpack inventory |
| 13 | `MapToggle` | General | Digital | Key: M | Gamepad: D-Pad Down | Toggle regional biome world map |
| 14 | `CharacterToggle` | Character | Digital | Key: C | Gamepad: D-Pad Right | Toggle equipment & wardrobe |
| 15 | `QuickSlot1` | Inventory | Digital | Key: 1 | Gamepad: D-Pad Left | Select hotbar slot 1 |
| 16 | `QuickSlot2` | Inventory | Digital | Key: 2 | Gamepad: — | Select hotbar slot 2 |
| 17 | `QuickSlot3` | Inventory | Digital | Key: 3 | Gamepad: — | Select hotbar slot 3 |
| 18 | `QuickSlot4` | Inventory | Digital | Key: 4 | Gamepad: — | Select hotbar slot 4 |
| 19 | `Pause` | General | Digital | Key: Escape | Gamepad: Start / Options | Open pause menu / back out |
| 20 | `UINavUp` | UI | Digital | Key: Up | Gamepad: D-Pad Up | Menu navigation upward |
| 21 | `UINavDown` | UI | Digital | Key: Down | Gamepad: D-Pad Down | Menu navigation downward |
| 22 | `UINavLeft` | UI | Digital | Key: Left | Gamepad: D-Pad Left | Menu navigation left |
| 23 | `UINavRight` | UI | Digital | Key: Right | Gamepad: D-Pad Right | Menu navigation right |
| 24 | `UIConfirm` | UI | Digital | Key: Enter | Gamepad: A / Cross | Menu confirmation / selection |
| 25 | `UICancel` | UI | Digital | Key: Escape | Gamepad: B / Circle | Menu cancellation / dismiss |
| 26 | `DevToggleUI` | DevUI | Digital | Key: F1 | Gamepad: — | Toggle Dear ImGui dev HUD |
| 27 | `DevToggleCollision` | DevUI | Digital | Key: F2 | Gamepad: — | Toggle physics collision bounds |
| 28 | `DevToggleGrid` | DevUI | Digital | Key: F3 | Gamepad: — | Toggle world tile grid overlay |
| 29 | `DevSpawnEnemy` | DevUI | Digital | Key: F4 | Gamepad: — | Debug spawn enemy under cursor |
| 30 | `DevSpawnLoot` | DevUI | Digital | Key: F5 | Gamepad: — | Debug drop random loot container |
| 31 | `DevCycleBiome` | DevUI | Digital | Key: F6 | Gamepad: — | Debug cycle active world biome |
| 32 | `DevResetPlayer` | DevUI | Digital | Key: F7 | Gamepad: — | Teleport player to initial spawn |

---

## 2. Analog vs Digital Semantics

* **Digital Actions**: Evaluated through `WasActionPressed(action)`, `IsActionHeld(action)`, and `WasActionReleased(action)`. Return binary boolean states based on edge transitions.
* **Analog Actions**: In addition to boolean transitions, analog actions support `GetActionValue(action)`, returning normalized floating-point intensities in range $[0.0 \dots 1.0]$.
  - For keyboard keys: Returns `1.0f` if held, `0.0f` if released.
  - For analog triggers and thumbsticks: Returns filtered analog values after deadband and threshold processing.

