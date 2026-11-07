# WILDBOUND — NETWORK INTEREST MANAGEMENT SPECIFICATION
## Spatial Partitioning, Distance Culling, and Bandwidth Optimization

**Document ID:** `WB-NET-013`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. The Scaling Problem on Battle Royale Maps

The Wildbound fantasy island is a large environment ($9000 \times 9000$ units, $281 \times 281$ tiles) populated by up to 60 players/bots, dozens of chests, and hundreds of ground loot items. If the server broadcast the coordinates of all entities to all 60 players 30 times per second:
- Each client would receive hundreds of irrelevant entity states per tick.
- Server egress bandwidth would scale at $O(N^2)$, exceeding 100 Mbps and crashing server network interfaces.
- Client memory would hold coordinates for players on the opposite side of the island, enabling wallhacks and map-wide ESP cheats.

**Network Interest Management** ensures that each client only receives updates for entities that are relevant to its gameplay awareness.

---

### 2. Spatial Grid Partitioning

The world is divided into a 2D uniform grid of **Interest Cells**:

```text
+-------------------+-------------------+-------------------+
|                   |                   |                   |
|   Cell (X-1, Y-1) |   Cell (X, Y-1)   |   Cell (X+1, Y-1) |
|                   |                   |                   |
+-------------------+-------------------+-------------------+
|                   |      PLAYER       |                   |
|   Cell (X-1, Y)   |   Cell (X, Y)     |   Cell (X+1, Y)   |
|                   |    [Local View]   |                   |
+-------------------+-------------------+-------------------+
|                   |                   |                   |
|   Cell (X-1, Y+1) |   Cell (X, Y+1)   |   Cell (X+1, Y+1) |
|                   |                   |                   |
+-------------------+-------------------+-------------------+
```

- **Cell Dimension**: $640 \times 360$ units (matches the virtual camera canvas dimensions).
- **Player Interest Zone**: The cell currently occupied by the player avatar plus the 8 adjacent neighbor cells (a $3 \times 3$ grid encompassing approx. $1920 \times 1080$ units).
- **Outer Perimeter**: Entities located in cells beyond the $3 \times 3$ grid are culled completely from that player's snapshot stream.

---

### 3. Tiered Replication Priorities

Entities within a player's spatial interest zone are prioritized based on tactical urgency:

| Priority Tier | Entity Category | Max Distance | Replication Rate | Rationale |
|---|---|---|---|---|
| **Tier 1 (Critical)** | Local Player Avatar | Direct | 60 Hz | Required for instantaneous input ack and reconciliation. |
| **Tier 2 (High)** | Enemy Players & Active Bots | $\le 800$ units | 30 Hz | Fast-moving combat opponents require maximum positional precision. |
| **Tier 3 (Medium)** | Ground Loot, Chests, Doors | $\le 600$ units | 10 Hz / Event | Static or stationary; high update rates are unnecessary. |
| **Tier 4 (Low)** | Distant Players / Teammates | $800 - 1500$ units | 5 Hz | Renders minimap blips or distant silhouettes; interpolated smoothly. |
| **Tier 5 (Culled)** | Distant Enemies / Inactive Bots | $> 1500$ units | 0 Hz | Completely excluded from packets (anti-ESP protection). |

---

### 4. Minimap & Global Match Awareness

In Battle Royale, players need global match awareness (e.g. teammate locations and storm boundaries) even when outside the visual interest cell:
1. **Teammates Are Always Replicated**: Teammates ignore distance culling. However, their updates are throttled to a low-frequency 5 Hz stream when distant, providing smooth minimap tracking without high bandwidth overhead.
2. **Storm Boundary Is Global**: The safe-zone circle and storm progress are broadcast globally to all clients on Channel 4 at 10 Hz.
3. **Sound Triggers (Acoustic Replication)**: Loud events (gunfire, explosions) that occur outside the visual cell generate lightweight sound events broadcast to players within hearing radius (approx. 1200 units) without revealing the shooter's exact coordinate or visual proxy.

