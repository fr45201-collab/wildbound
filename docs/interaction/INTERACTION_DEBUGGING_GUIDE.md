# Interaction Debugging Guide

## 1. Overview

Developer tooling for the interaction system is integrated into Wildbound's central Dear ImGui / rlImGui developer overlay (`DevUI`), specifically under **Tab 15: World Interactions**.

---

## 2. DevUI Controls & Diagnostics

Press **F1** or **`** to toggle DevUI and navigate to the **World Interactions** tab:

### 2.1 Live Statistics Banner
* **Registered Interactables**: Total count of active interactable components.
* **Discovered Candidates**: Count of interactables currently within player proximity range.
* **Current Focus**: ID, display name, category, and distance of the top-ranked candidate.
* **Pickup Policy**: Toggle between `Manual [E]` and `Automatic Proximity`.

### 2.2 Proximity Candidates Table
Inspect all candidates evaluated by the discovery pipeline:
* `ID`: Runtime numeric interactable handle.
* `Name`: Display name.
* `Category`: Door, Building, Container, Collectible, Mechanism, etc.
* `Distance`: Exact pixel distance to player.
* `Facing Dot`: Directional dot product with player orientation.
* `Priority`: Configuration priority tier.
* `Cooldown`: Remaining cooldown timer.
* `Status`: Active target vs In-range vs Blocked.

### 2.3 Domain Control Panels
* **Doors Control Panel**: View all registered doors, their states (`Open`, `Closed`, `Locked`), required keys, and test `Toggle Door` or `Force Unlock` buttons.
* **Mechanisms Control Panel**: View all registered levers, switches, and pressure plates with `Toggle Mechanism` and `Reset One-Time` buttons.

---

## 3. In-World Spatial Visualizer

When **Show Interaction Radii & Vectors** is enabled in DevUI:
1. **Dotted Cyan Circle**: Drawn around each interactable indicating its interaction range radius.
2. **Directional Arrow**: Rendered from player center along facing vector $\vec{f}$.
3. **Selected Highlight Box**: Gold bounding box and prompt preview rendered above the currently focused interactable.
4. **Collision Boxes**: Visualized solid bounding boxes for closed doors in red; open doors in green.

