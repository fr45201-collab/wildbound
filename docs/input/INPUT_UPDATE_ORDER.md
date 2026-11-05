# Wildbound — Input Update Order & Frame Lifecycle

## 1. Frame Lifecycle Sequence

To ensure determinism and eliminate single-frame latency or race conditions, input processing strictly adheres to the following update sequence each frame:

```
+---------------------------------------------------------------------------------------+
| 1. HARDWARE ACQUISITION                                                               |
|    InputManager::Update(dt)                                                           |
|    - Polls raylib: Keyboard, Mouse, Gamepad.                                          |
|    - Computes Viewport Virtual & Camera World mouse positions.                        |
|    - Updates active device detection (Keyboard/Mouse vs Gamepad).                     |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
| 2. CONTEXT SYNCHRONIZATION                                                            |
|    Game::ProcessInput()                                                               |
|    - Synchronizes InputContextManager stack with active UIManager screen.             |
|    - Pushes/pops modal contexts (Gameplay, Menu, Inventory, Wardrobe, Paused).         |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
| 3. BUFFER EXPIRATION & ACTION RESOLUTION                                              |
|    - InputBuffer decrements remaining lifespans by dt; purges expired entries.        |
|    - Evaluates physical bindings for each InputAction under active Context mask.       |
|    - If an action was pressed, enqueues it to InputBuffer if within bufferable set.   |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
| 4. SYSTEM CONSUMPTION                                                                 |
|    A. UIManager::Update(dt, inputMgr)                                                 |
|       - Consumes UI navigation, confirm, cancel, and virtual mouse click events.      |
|    B. CharacterController::Update(inputMgr)                                           |
|       - Consumes movement vector (WASD + Analog Thumbstick).                          |
|       - Consumes buffered combat actions (Attack, Dodge, Interact).                   |
|    C. Game Engine Transitions                                                         |
|       - Handles Pause menu toggles and camera offsets.                                |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
| 5. POST-FRAME RESET & EDGE CLEARING                                                   |
|    - Resets single-frame transition flags (pressed / released) in ActionState table.  |
|    - Focus-loss handling: if window loses focus, all held states reset to false.      |
+---------------------------------------------------------------------------------------+
```

---

## 2. Window Focus Loss Safety

When the game window loses OS focus (e.g. `Alt+Tab` or clicking another monitor), operating systems often fail to dispatch key-up events, resulting in "stuck keys" (where the player endlessly walks in a direction).

Wildbound mitigates this in `InputManager::PollHardware()`:
```cpp
if (::IsWindowFocused && !::IsWindowFocused()) {
    Reset(); // Immediately flushes all held states, motion vectors, and buffers
}
```
When focus returns, the player is safely at rest until fresh hardware events are captured.

