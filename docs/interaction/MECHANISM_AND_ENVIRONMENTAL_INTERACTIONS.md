# Mechanism and Environmental Interactions

## 1. World Mechanisms (`Mechanism.hpp` / `Mechanism.cpp`)

Mechanisms represent active actuators embedded in the world: levers, switches, pressure plates, and gate cranks.

### 1.1 Supported Mechanism Types (`MechanismType`)

| Type | Default Visual Dimensions | Trigger Mode | Intended Use Case |
| :--- | :--- | :--- | :--- |
| `Lever` | $20 \times 20\text{ px}$ | Manual `[E]` | Wall levers, gate cranks, bridge winches |
| `Switch` | $20 \times 20\text{ px}$ | Manual `[E]` | Hidden wall bricks, altar buttons |
| `PressurePlate` | $32 \times 32\text{ px}$ | Automatic Proximity Overlap | Floor plates, dart traps, secret door sensors |
| `Turnstile` | $28 \times 28\text{ px}$ | Manual `[E]` | One-way passage controls |

### 1.2 State & Timing Semantics
* **Active / Inactive**: Mechanism state alternates on trigger unless configured as `isOneTime = true`.
* **One-Time Activations**: Once triggered, `m_hasBeenActivated = true` prevents subsequent deactivation or re-triggering.
* **Cooldown Timers**: Each activation initiates an internal cooldown (default 0.5s) preventing rapid spam or audio stuttering.
* **Callback Hooks (`MechanismStateCallback`)**:
  ```cpp
  lever.SetStateCallback([](Mechanism& mech, bool active) {
      // Open linked portcullis, light torches, or trigger trap
  });
  ```

---

## 2. Environmental Objects

Atmospheric and restorative world objects that enrich the fantasy world:

1. **Campfires**: Provide contextual prompt `"Rest"` or `"Inspect"`, trigger warm crackling audio, and emit animated smoke plumes.
2. **Shrines & Altars**: Prompt `"Pray"` or `"Channel"`, playing ceremonial audio events.
3. **Wishing Wells & Fountains**: Offer water interaction cues.
4. **Notice Boards & Signposts**: Surface lore inscriptions without impeding physical player movement.

