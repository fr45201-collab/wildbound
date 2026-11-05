# Wildbound — Collision Layer & Mask Specification

> **Document ID**: `WB-SPEC-PHYS-002`  
> **Status**: APPROVED  
> **Date**: 2026-11-04  
> **Subsystem**: `wildbound::physics`

---

## 1. Collision Layers

Wildbound establishes 10 standardized logical collision layers represented as 32-bit integer bitflags (`uint32_t`). Each collider belongs to **exactly one** primary layer.

| Layer | Bit Value | Hex | Description |
|---|---|---|---|
| `None` | `0` | `0x00000000` | Unassigned or non-colliding layer |
| `World` | `1 << 0` | `0x00000001` | Static world geometry: stone walls, cliffs, water bodies, impassable trees |
| `Player` | `1 << 1` | `0x00000002` | Human-controlled character collider |
| `Enemy` | `1 << 2` | `0x00000004` | Hostile creatures: Goblins, Skeletons, Wolves, Bats |
| `Wildlife` | `1 << 3` | `0x00000008` | Ambient non-combat creatures: Songbirds, Butterflies |
| `Projectile` | `1 << 4` | `0x00000010` | High-speed ballistic broadhead arrows and future magic missiles |
| `Pickup` | `1 << 5` | `0x00000020` | Ground loot items: weapons, armor, potions, coins, crafting materials |
| `Trigger` | `1 << 6` | `0x00000040` | Non-blocking volumetric trigger regions: zone boundaries, ambushes |
| `Building` | `1 << 7` | `0x00000080` | Architectural structures: town hall, blacksmith walls, granary perimeters |
| `Hazard` | `1 << 8` | `0x00000100` | Environmental hazards: fire pits, spike traps, poison mire |
| `Interactable` | `1 << 9` | `0x00000200` | Interactive world objects: loot chests, doorway portals, levers |
| `All` | `~0u` | `0xFFFFFFFF` | Universal mask matching all categories |

---

## 2. Bilateral Mask Filtering Rule

Wildbound enforces a strict **bilateral consent model** for collision. Two colliders $A$ and $B$ are eligible for collision or overlap testing if and only if:

$$(A.\text{mask} \ \& \ B.\text{layer}) \ne 0 \quad \mathbf{AND} \quad (B.\text{mask} \ \& \ A.\text{layer}) \ne 0$$

If entity $A$ permits collision with layer $B$, but entity $B$ has masked out layer $A$, **no collision or trigger event occurs**. This prevents unilateral side effects (e.g., an enemy inadvertently triggering player-only loot containers or dialogue prompts).

### Self-Collision Exclusion
Additionally, if two colliders share the same non-zero `EntityId`:
$$A.\text{entityId} == B.\text{entityId} \implies \text{No Collision}$$
This prevents an entity's physical torso collider from colliding with its own weapon socket or trigger volume.

---

## 3. Default Collision Matrix

The table below details the default bilateral interaction eligibility across all 10 layers:

| Layer | World | Player | Enemy | Wildlife | Projectile | Pickup | Trigger | Building | Hazard | Interactable |
|---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **World** | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Player** | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Enemy** | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ |
| **Wildlife** | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ |
| **Projectile**| ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ |
| **Pickup** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Trigger** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Building** | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Hazard** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Interactable**| ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

### Matrix Design Rationale:
- **Static vs. Static**: `World` vs `World` and `Building` vs `Building` are disabled; static geometry never moves relative to itself.
- **Pickups**: Ground items interact strictly with `Player`. Enemies and projectiles ignore ground loot.
- **Wildlife**: Ambient birds and butterflies avoid terrain and players, but do not trigger enemy chase states or block arrows.
- **Interactables**: Chests and doors respond to `Player` proximity triggers.

