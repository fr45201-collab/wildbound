# Wildbound — Server Failure & Recovery Architecture

## 1. Objective & Three-Tier Failure Scope

This document evaluates the recovery architecture of the Wildbound dedicated server when subjected to abrupt termination, unhandled exceptions, and host failures.

Recovery is formally evaluated across three distinct operational tiers:
1. **Process Recovery**: Can the executable restart cleanly, re-bind its network socket, and initialize subsystems?
2. **Service Recovery**: Can the coordination layer (`WildboundCoordinator`) detect node failure, remove unhealthy instances, and spawn replacements?
3. **Match Recovery**: Can in-flight match state survive a hard process crash, or is it discarded?

---

## 2. Empirical Test Execution & Results

During Phase 48's automated failure simulation:

| Recovery Dimension | Empirical Result | Behavior & Diagnostic Observation |
|---|---|---|
| **Graceful Shutdown** | **CLEAN** | Orderly SIGINT/SIGTERM or `RequestShutdown()` stops ENet, drains sockets, saves match outbox, resets match, exits code 0. |
| **Crash / Force Kill Detection** | **DETECTED** | Abrupt process termination triggers socket close on OS; Coordinator heartbeat loss detected within 5 seconds. |
| **Server Process Restart** | **SUCCESS** | Process restarted on port 17777 in <800 ms; re-bound UDP port, loaded world geometry (41,503 bounds), re-armed Lua runtime. |
| **Client Survivor Reconnection** | **FAILED** *(By Design)* | Surviving client attempting to re-use old connection tokens against restarted server is rejected; must negotiate new session. |
| **Active Match Recovery** | **Service Recovery** | In-flight match state is volatile in-memory; match is reset cleanly. Coordinator directs affected players to fresh match queue. |

---

## 3. Detailed Failure Tier Analysis

### 3.1 Process Recovery (Tier 1)
- **Port Re-binding**: ENet socket configuration uses `SO_REUSEADDR` semantics where appropriate, allowing instant process reboot without `WSAEADDRINUSE` (Address already in use) collisions.
- **Subsystem Idempotency**: Map files, Tiled JSON parsers, item catalogs, and Lua 5.4 runtimes initialize idempotently from read-only disk assets without state corruption.

### 3.2 Service Recovery (Tier 2)
```
[Dedicated Server] ---- Heartbeat (every 5s) ----> [Coordinator]
        X (Process Crashes)
[Dedicated Server] - - - - - - - - - - - - - - - > [Coordinator]
                                                          |
                                                  Misses 2 Heartbeats
                                                          v
                                                [Mark Instance Dead]
                                                [Remove from Matchmaking Pool]
                                                [Notify Process Manager / Daemon]
```
- The dedicated server transmits heartbeats every 5.0 seconds (`SendCoordinatorHeartbeat()`).
- If the coordinator misses 2 consecutive heartbeats (10s), the server instance is marked dead and removed from the active matchmaking pool.
- A supervisory daemon (systemd on Linux, service wrapper on Windows) automatically restarts the executable.

### 3.3 Match Recovery (Tier 3)
- **Volatile Match State**: Real-time entity physics, projectile trajectories, and storm radii are held in memory for low-latency authoritative execution. They are **not persisted to disk per-frame**.
- **Architectural Trade-Off**: Checkpointing 30 Hz physics to disk would destroy server tick performance. Therefore, in the event of an unrecoverable process crash, the current match is terminated.
- **Client Fallback**: Game clients detect socket loss via ENet timeout, display "Match Terminated by Server", and automatically failover back to the Coordinator lobby to queue for the next match.
- **Outbox Durability**: Completed match outcomes written to `MatchResultsOutbox` prior to crash are persisted in durable SQLite/JSON storage and sent to the Go backend upon restart.

