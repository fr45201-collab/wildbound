# Wildbound — Anti-Cheat Foundations & Trust Boundaries

**Phase:** 47  
**Status:** Completed  
**Subsystems:** ServerSecurityManager, RateLimiter, DedicatedServer  

---

## 1. Threat Model & Trust Boundaries

In multiplayer games, the client application runs on hardware fully controlled by potentially malicious actors. Memory editors (e.g. CheatEngine), modified binaries, packet sniffers/injectors, and network latency simulators can alter client state at will.

### Core Security Tenets:
1. **The Client is Untrusted**:
   - The client is strictly an input generation and render display terminal.
   - The client **never** decides hit registration, damage dealt, player position, or item ownership.
2. **Server Authoritative State**:
   - The dedicated server maintains the canonical game truth.
   - Client requests are merely *intents* (e.g., "I intend to move here", "I intend to attack here", "I want to pick up this item").
3. **Detection vs Enforcement**:
   - The server validates all intents against physical laws and server-known timings.
   - Violations are categorized by severity (`Info`, `Warning`, `Violation`, `Critical`).
   - Minor anomalies (due to packet jitter) are clamped to the maximum physical boundary.
   - Blatant hacks (teleportation, duplicate claims, packet floods) are rejected outright and recorded in an audit trail.

---

## 2. Server Security Architecture

The anti-cheat foundation is implemented in the `wildbound::security` subsystem:

```
[ Client Ingress Datagram ]
           │
           ▼
[ PacketValidator / Deserialization Guard ] (Reject NaN, Inf, Malformed Wire)
           │
           ▼
[ RateLimiter ] (Token-bucket per Client & Category)
           │
           ▼
[ ServerSecurityManager ]
    ├── ValidateMovement()      (Speed bounds, teleport detection, snapback)
    ├── ValidateAttack()        (Range tolerance, weapon cooldown, finite math)
    ├── ValidateLootPickup()    (Atomic claim resolution, distance check)
    └── RecordEvent()           (Audit log, violation counter, auto-disconnect)
           │
           ▼
[ Authoritative Simulation State Execution ]
```

---

## 3. Security Event Types & Severity Classification

| Event Type | Typical Cause | Severity | Authoritative Action |
| :--- | :--- | :--- | :--- |
| `RateLimitExceeded` | Packet flood, script loop, macro spam | Warning / Violation | Packet dropped |
| `InvalidMovementSpeed` | Speed hack, modified move speed float | Warning | Clamped to max allowed distance |
| `TeleportAttempt` | Position hack, memory write to coordinates | Critical | Position snapped back to server truth |
| `NonFiniteCoordinates` | Memory injection of `NaN` / `$\pm\infty$` | Critical | Dropped, position snapped back |
| `AttackCooldownViolation` | Rapid-fire attack macro, cooldown bypass | Warning | Attack request rejected |
| `AttackRangeViolation` | Reach hack, infinite melee range injection | Violation | Attack request rejected |
| `DuplicateLootClaim` | Race condition exploit, double pickup packet | Violation | Rejected (already claimed) |
| `LootRangeViolation` | Vacuum hack, picking up loot from across map | Warning | Rejected (out of physical reach) |
| `MalformedPacket` | Fuzzing, invalid message enum, buffer overrun | Warning | Deserialization aborted, connection dropped if repeated |

---

## 4. Progressive Violation Escalation & Auto-Disconnect

- Each client connection is tracked by `ServerSecurityManager`.
- When a client accumulates $\ge 15$ severe security violations (`SecuritySeverity::Violation` or `Critical`), `ShouldDisconnectClient()` evaluates to `true`.
- The dedicated server terminates the client socket with ENet disconnect code `4003 (SecurityViolation)` to protect the match integrity for legitimate players.

