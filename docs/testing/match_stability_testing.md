# Wildbound — Match Stability & Repeated Match Lifecycle Testing

## 1. Objective

Dedicated game servers must run continuously across dozens of consecutive matches over multiple days without leaking heap memory, accumulating orphaned participant entries, or hanging during map or storm state transitions.

This test evaluated `MatchManager`, `World`, `StormManager`, and `DedicatedServer` through 3 sequential end-to-end match lifecycles with 8 players each.

---

## 2. Match Lifecycle State Flow

Each match executes the full state machine defined in `MatchTypes.hpp`:

```
 [Uninitialized]
        |  MatchManager::Initialize()
        v
  [Initializing]
        |  Map, Storm, Combat, Items Loaded
        v
[WaitingForPlayers] <===== 8 Bots Connect & Register
        |  Warmup Timer Expires / Player Threshold Met
        v
    [Countdown]
        |  Match Start Triggered
        v
    [InProgress] <===== Storm Phases Advance, Combat Active
        |  Victory Condition Met / Elimination
        v
    [Completed]
        |  Durable Outbox Queues Match Record
        v
     [Cleanup]  <===== Unload Entities, Reset Storm, Disconnect Clients
        |  ResetMatch()
        v
 [WaitingForPlayers] (Next Match Begins)
```

---

## 3. Empirical Test Results

| Lifecycle Run | Configured Players | Admitted Players | Match Duration (s) | Avg Tick (ms) | Peak Tick (ms) | Cleanup Clean? | Memory Bounded? |
|---|---|---|---|---|---|---|---|
| **Match 1** | 8 | 8 | 1.92 | 0.18 | 1.04 | **YES** | **YES (0 leak)** |
| **Match 2** | 8 | 8 | 1.92 | 0.19 | 0.97 | **YES** | **YES (0 leak)** |
| **Match 3** | 8 | 8 | 1.92 | 0.18 | 1.11 | **YES** | **YES (0 leak)** |

### Summary Verification:
- **Matches Started**: 3
- **Matches Completed**: 3
- **Matches Cleaned**: 3
- **Memory Bounded**: **YES (Zero Heap Leak)**

---

## 4. Key Subsystem Resets Verified

During each transition to `[Cleanup]` -> `[WaitingForPlayers]`:
1. **Match Registry**: All 8 `MatchParticipant` records from the prior match were purged; participant count reset cleanly to 0 before the next match admitted bots.
2. **Storm System**: `StormManager` reset from phase 6 contraction back to `StormState::Idle` with radius restored to 4,000 units.
3. **World Spatial Index**: Collision tree and item entity caches remained bounded without duplication.
4. **Lua Runtime**: Sol2 state and enemy AI script bindings remained valid without re-initialization crashes.
5. **Durable Match Outbox**: Match results were dispatched to disk/database outbox queues without blocking tick execution.

