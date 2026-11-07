# WILDBOUND — REPLICATION RULES & COMPONENT SYNCHRONIZATION MATRIX
## Field-Level Authority, Replication Frequencies, Channels, and Visibility Rules

**Document ID:** `WB-NET-007`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. Component Replication Principles

Replicating every variable of every component to every client wastes immense bandwidth, leaks tactical data (enabling wallhacks), and causes network CPU starvation. Wildbound enforces strict component replication rules:

1. **Owner-Only vs. Public Broadcast**:
   - Private data (e.g. ammo counts, inventory items, unspent skill points) is replicated **exclusively to the owning client**.
   - Public data (e.g. world position, health bar percentage, equipped cosmetic skin, active weapon sprite) is **broadcast to all players within interest range**.
2. **Dirty Bitmasking**: Components maintain dirty bitflags. Fields that have not changed since the client's last acknowledged tick are omitted from the delta snapshot.
3. **Adaptive Update Cadence**: High-speed, fast-moving entities replicate at 30 Hz; stationary or slow objects replicate at 5–10 Hz; static chests replicate purely on state transitions.

---

### 2. Comprehensive Component Replication Matrix

| Entity Type | Component / State Field | Server Auth? | Client Writable? | Replicated? | Frequency | Delivery | Audience | Interpolation? | Prediction? |
|---|---|---|---|---|---|---|---|---|---|
| **Player** | Position $(x, y)$ | **Yes** | No | Yes | 30 Hz | Unreliable Sequenced | Interested | **Yes** | **Yes (Owner)** |
| **Player** | Velocity $(v_x, v_y)$ | **Yes** | No | Yes | 30 Hz | Unreliable Sequenced | Interested | **Yes** | **Yes (Owner)** |
| **Player** | Facing Angle $\theta$ | **Yes** | No | Yes | 15 Hz | Unreliable Sequenced | Interested | **Yes** | No |
| **Player** | Movement State (Idle/Walk/Run) | **Yes** | No | Yes | On Change | Reliable | Interested | No | No |
| **Player** | Health & MaxHealth | **Yes** | No | Yes | On Change | Reliable | Broadcast | No | No |
| **Player** | Armor Value | **Yes** | No | Yes | On Change | Reliable | Broadcast | No | No |
| **Player** | Equipped Weapon ID | **Yes** | No | Yes | On Change | Reliable | Broadcast | No | No |
| **Player** | Ammo Count | **Yes** | No | Yes | On Change | Reliable | **Owner Only** | No | No |
| **Player** | Inventory Items (Backpack) | **Yes** | No | Yes | On Change | Reliable | **Owner Only** | No | No |
| **Player** | Equipped Cosmetics (Skin, Hair) | **Yes** | No | Yes | Spawn / Change | Reliable | Broadcast | No | No |
| **Player** | Elimination Status | **Yes** | No | Yes | On Change | Reliable | Broadcast | No | No |
| **Enemy Bot**| Position & Velocity | **Yes** | No | Yes | 20 Hz | Unreliable Sequenced | Interested | **Yes** | No |
| **Enemy Bot**| Health & Combat State | **Yes** | No | Yes | On Change | Reliable | Interested | No | No |
| **Projectile**| Spawn Position & Velocity | **Yes** | No | Yes | On Spawn | Reliable | Interested | Math Lerp | Visual Only |
| **Ground Loot**| Item ID & Position | **Yes** | No | Yes | On Spawn | Reliable | Interested | No | No |
| **Loot Chest**| Open / Closed State | **Yes** | No | Yes | On Change | Reliable | Interested | No | No |
| **Door** | Open / Closed Angle | **Yes** | No | Yes | On Change | Reliable | Interested | Yes (Angle) | No |
| **Storm** | Current Center & Radius | **Yes** | No | Yes | 10 Hz | Unreliable Sequenced | Broadcast All | **Yes** | No |
| **Storm** | Target Center & Radius | **Yes** | No | Yes | Phase Change | Reliable | Broadcast All | No | No |
| **Match** | State (Countdown/Active/End) | **Yes** | No | Yes | On Change | Reliable | Broadcast All | No | No |
| **Match** | Remaining Alive Count | **Yes** | No | Yes | On Change | Reliable | Broadcast All | No | No |
| **Results** | Final Placement & Stats | **Yes** | No | Yes | On End | Reliable | Broadcast All | No | No |

---

### 3. Component Dirty Bitmask Definition

When assembling a snapshot for a dynamic entity (such as a Player), the server evaluates a 16-bit bitmask:

```cpp
namespace wildbound::network {

enum class EntityDirtyMask : uint16_t {
    Clean             = 0x0000,
    DirtyPosition     = 1 << 0,  // x, y coordinates
    DirtyVelocity     = 1 << 1,  // vx, vy coordinates
    DirtyFacing       = 1 << 2,  // aim/look angle
    DirtyAnimState    = 1 << 3,  // idle, walk, run, attack
    DirtyHealthArmor  = 1 << 4,  // health, shield/armor
    DirtyWeapon       = 1 << 5,  // active weapon slot & weapon id
    DirtyAmmo         = 1 << 6,  // ammo count (owner only)
    DirtyInventory    = 1 << 7,  // items in backpack (owner only)
    DirtyCosmetics    = 1 << 8,  // hair, skin, costume overrides
    DirtyStatusFlags  = 1 << 9,  // alive, knocked, spectating
    All               = 0xFFFF
};

constexpr EntityDirtyMask operator|(EntityDirtyMask a, EntityDirtyMask b) noexcept {
    return static_cast<EntityDirtyMask>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

constexpr bool HasFlag(EntityDirtyMask mask, EntityDirtyMask flag) noexcept {
    return (static_cast<uint16_t>(mask) & static_cast<uint16_t>(flag)) != 0;
}

} // namespace wildbound::network
```

---

### 4. Serialization Layout by Bitmask

The server serializes the bitmask first, followed immediately by only the dirty fields:

```text
[Header: 2 bytes]  -> EntityDirtyMask
[Conditional 1]    -> If DirtyPosition: float32 x, float32 y (8 bytes)
[Conditional 2]    -> If DirtyVelocity: int16 vx, int16 vy (4 bytes, fixed-point)
[Conditional 3]    -> If DirtyFacing:   uint8 angle_quantized (1 byte, 256 steps)
[Conditional 4]    -> If DirtyHealth:   uint16 health, uint16 armor (4 bytes)
[Conditional 5]    -> If DirtyWeapon:   uint8 weapon_slot, uint16 weapon_id (3 bytes)
...
```

If an entity is stationary behind cover, only `DirtyMask = 0x0000` is transmitted (2 bytes total overhead), saving up to **85% bandwidth** compared to continuous full struct replication.

