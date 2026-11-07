# Corruption Storm Lifecycle Integration Specification

## 1. Unified Lifecycle Synchronization

The Corruption Storm (`wildbound::storm::StormManager`) is synchronized with `wildbound::match::MatchManager` across all 10 match lifecycle states:

| Match State | Storm State | Storm Progression / Behavior |
|---|---|---|
| `Uninitialized` | `Idle` | Storm inactive; no timers running, safe zone static. |
| `Initializing` | `Idle` | Loaded configuration verified; initial circle prepared. |
| `WaitingForPlayers` | `Idle` | Circle static; zero damage applied. |
| `Countdown` | `Idle` | Pre-match countdown running; storm timers frozen. |
| `Starting` | `Idle` $\rightarrow$ `Warning` | Safe zone and boundary renderer reset to initial radius. |
| `Active` | `Warning` / `Shrinking` | Full progression through phases 1–6; outside damage active. |
| `Finishing` | `Paused` | Contraction immediately frozen upon final survivor detection. |
| `Completed` | `Paused` | Haze and boundary rings static during results presentation. |
| `Cleanup` | `Idle` | Phase timers, damage accumulators, and particles reset. |
| `Failed` | `Idle` | Safe zones reset; no hazards active. |

---

## 2. Storm Damage Elimination Attribution

When an entity dies while located outside the safe perimeter:
```cpp
if (player->GetHealth().IsDead()) {
    EliminationCause cause = EliminationCause::Combat;
    if (stormMgr && stormMgr->GetDamageSystem().IsPlayerDamagedThisTick()) {
        cause = EliminationCause::CorruptionStorm;
    }
    matchMgr->ProcessElimination(localPlayerId, cause, 0);
}
```
1. `StormDamageSystem::DAMAGE_SOURCE_ID` (`"DAMAGE_SOURCE_CORRUPTION_STORM"`) attributes the kill specifically to the storm.
2. The victim's elimination info records killer name as `"Corruption Storm"`.
3. The kill is not credited to any player competitor, avoiding artificial kill inflation.
4. Surviving competitors inside the circle are preserved, triggering victory once all competitors outside have expired.

