# WILDBOUND — NETWORK SECURITY & VALIDATION SPECIFICATION
## Boundary Defenses, Packet Sanitization, Anti-Tamper Logic, and Attack Mitigation

**Document ID:** `WB-NET-016`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. Security Philosophy: Defense in Depth

Wildbound does not rely on invasive client-side kernel drivers or obfuscated third-party anti-cheat binaries. Instead, security is built natively into the engine through **mathematical server authority, strict packet boundary validation, and zero-trust protocol architecture**.

---

### 2. Network Boundary Threat Model

| Threat / Attack Vector | Potential Impact | Server-Side Mitigation Strategy |
|---|---|---|
| **Speed Hacking** | Player moves faster than allowed, evading storm or chasing enemies. | Server clamps velocity to $V_{\max} \cdot \Delta t \cdot 1.10$. Excess displacement triggers reconciliation snap. |
| **Teleportation** | Player instantaneously warps across map to POIs or loot. | Any displacement $> D_{\max}$ in a single tick is immediately rejected; logs security alert. |
| **Wall Clipping / Noclip** | Player passes through walls, rocks, or closed doors. | Movement is solved using server tilemap swept AABB collision; client cannot disable server collision. |
| **God Mode / Immortality** | Player client ignores incoming damage. | Health and damage calculation exist purely on the server. The client has no write access to health. |
| **Infinite Ammo / No Reload** | Player fires continuously without consuming ammunition. | Server tracks ammo in player inventory; rejects attack requests when ammo is zero. |
| **Rapid Fire Hack** | Player triggers 100 attacks per second. | Server enforces weapon `cooldownTimer`; attacks triggered before cooldown expires are discarded. |
| **Item Duplication** | Player rapidly picks up the same item twice from ground. | Server marks ground loot as `Claimed` in a single thread tick; second pickup request fails atomically. |
| **Memory ESP / Wallhack** | Cheat software reads remote player coordinates from RAM. | Interest management culls players beyond visual range and occluded terrain; client RAM never holds distant enemy coords. |
| **Packet Replay Attacks** | Attacker replays old successful attack packets. | Strict monotonically incrementing sequence numbers per connection; old sequences are dropped. |
| **Malformed Packet Buffer Overflow** | Attacker sends oversized payloads to crash server. | Binary ByteStream enforces bounds checking on every read; oversized payloads drop connection. |

---

### 3. Packet Sanitization & Validation Pipeline

Every byte received from a network socket passes through a 4-tier validation gauntlet:

```text
[Raw UDP Socket]
       |
       v
+--------------------------------------------------------------------------+
| 1. Wire-Level Validation:                                                |
|    - Size check: 16 bytes <= size <= 1264 bytes                          |
|    - Magic number check: 0x57424E44 ('WBND')                             |
|    - Protocol version check: 0x0001                                      |
+--------------------------------------------------------------------------+
       | Pass
       v
+--------------------------------------------------------------------------+
| 2. Envelope & Session Validation:                                        |
|    - Sender ClientId corresponds to an active, authenticated peer        |
|    - Sequence number is greater than previous acknowledged sequence      |
|    - MessageType is a known, valid enum value                            |
+--------------------------------------------------------------------------+
       | Pass
       v
+--------------------------------------------------------------------------+
| 3. Payload Bounds & Deserialization Validation:                          |
|    - PayloadLength matches remaining buffer bytes exactly                |
|    - String fields do not exceed maximum character limits                |
|    - Float values are valid numbers (no NaN or Infinity)                 |
+--------------------------------------------------------------------------+
       | Pass
       v
+--------------------------------------------------------------------------+
| 4. Semantic Gameplay Validation:                                         |
|    - Actor entity belongs to the sending client                          |
|    - Player is alive and not stunned                                     |
|    - Action satisfies range, cooldown, and resource constraints          |
+--------------------------------------------------------------------------+
       | Pass
       v
[Authoritative Game Simulation Execution]
```

---

### 4. Rate Limiting & Denial-of-Service Defense

To prevent malicious clients from saturating server CPU with packet flood attacks:
- **Packet Rate Ceiling**: Each client connection is metered using a token-bucket filter allowing up to $80\text{ packets/second}$.
- **Bursts**: Bursts up to $100$ packets are tolerated for up to $200\text{ ms}$; sustained rates above $80\text{ packets/second}$ trigger automatic packet dropping.
- **Connection Blacklisting**: If a client exceeds the threshold for $> 3.0$ seconds, the connection is forcefully closed (`ConnectionRejected::AbuseLimit`).

