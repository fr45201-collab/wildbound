# Storm Debugger & DevUI Integration

## 1. Developer UI Architecture (Tab 19)

The Corruption Storm Developer Tooling is fully embedded in `wildbound::ui::DevUI` under dedicated Tab 19: **"Corruption Storm"**.

```
+-------------------------------------------------------------------------------+
| DEV UI — TAB 19: CORRUPTION STORM                                             |
+-------------------------------------------------------------------------------+
| STATUS & METRICS                                                              |
| State: SHALOW_WARNING    | Phase: 2 / 6 ("Phase 2 - Secondary Sector")        |
| Time In Phase: 14.2s     | Time Remaining: 45.8s                              |
| Damage Rate: 2.0 HP/s    | Interpolation Progress: [==========          ] 35% |
+-------------------------------------------------------------------------------+
| SAFE ZONE GEOMETRY                                                            |
| Current Center: (5103.1, 5343.6) | Current Radius: 3000.0 px                  |
| Target Center:  (4908.4, 5185.9) | Target Radius:  2200.0 px                  |
+-------------------------------------------------------------------------------+
| PLAYER STATUS                                                                 |
| Player Position: (5210.0, 5100.0)                                             |
| Status: SAFE (Depth: 280.4m inside safe boundary)                             |
| Penetration / Distance: 0.0m     | Fractional Damage Accumulator: 0.00 HP     |
+-------------------------------------------------------------------------------+
| INTERACTIVE CONTROLS                                                          |
| [ PAUSE / RESUME STORM ]  [ SKIP TO NEXT PHASE ]  [ RESTART MATCH STORM ]     |
| Override Radius: [======|=====] 2500 px  [ APPLY RADIUS OVERRIDE ]            |
| Jump to Phase:   [ 1 | 2 | 3 | 4 | 5 | 6 ]                                    |
| [ RELOAD STORM CONFIG FROM DISK ]                                             |
+-------------------------------------------------------------------------------+
```

---

## 2. Interactive Features

1. **Phase Jumping**:
   Allows instant teleportation of the match storm state machine to any phase (1 through 6) to test late-game or endgame collapse behavior without waiting.
2. **Timer Skipping**:
   `SkipToNextPhase()` immediately advances the hold or shrink state to test boundary animations and HUD updates.
3. **Radius & Center Sliders**:
   Enables manual live-tweaking of circle dimensions and positions.
4. **Live Metrics Display**:
   Real-time monitoring of safe zone coordinates, player penetration depth, damage rate, and accumulator status.

