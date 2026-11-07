# PLAYER & MATCH STATISTICS SPECIFICATION

## 1. Overview
The Player Statistics subsystem in Wildbound Phase 33 aggregates, validates, and reports both match-level aggregate telemetry and individual competitor performance metrics.

---

## 2. Match-Scoped Telemetry

### A. Aggregate Match Statistics
- **`matchDuration`**: Monotonic time elapsed in seconds during `Active` state.
- **`totalParticipants`**: Total registered competitors participating in the match.
- **`totalEliminations`**: Cumulative count of competitors eliminated.
- **`survivorsCount`**: Living competitors remaining at match completion.
- **`winnerParticipantId`**: ID of victorious competitor (or `0` in Draw).
- **`winnerDisplayName`**: Display callsign of champion (or `"No Winner (Draw)"`).
- **`matchOutcome`**: Categorical completion outcome (`Victory`, `Draw`, `Cancelled`, `Failed`).

### B. Individual Competitor Statistics (`PlayerPerformance`)
- **`placement`**: Final tournament rank ($1 \dots N$).
- **`survivalDuration`**: Seconds elapsed from match start until elimination or victory.
- **`kills`**: Authoritative eliminations credited to this competitor.
- **`deaths`**: Binary death status (0 if winner, 1 if eliminated).
- **`damageDealt`**: Cumulative combat damage inflicted on other competitors.
- **`combatDamageReceived`**: Damage taken from enemy competitor attacks.
- **`stormDamageReceived`**: Damage taken from Corruption Storm exposure.
- **`totalDamageReceived`**: Sum of combat and storm damage accepted.
- **`eliminationCause`**: Authoritative cause of death (`Combat`, `CorruptionStorm`, `Environment`).
- **`killerDisplayName`**: Name of attributed killer (or `"Corruption Storm"` / empty).

---

## 3. Data Integrity & Unsupported Metrics Policy
In accordance with Wildbound architectural standards:
- Metrics with real backing data (`kills`, `damageDealt`, `combatDamageReceived`, `stormDamageReceived`, `survivalDuration`, `placement`) are reported accurately.
- Unsupported/untracked statistics (e.g., headshot percentages, APM, distance walked) are **not fabricated** or displayed as misleading zeros.
- Finalized statistics become strictly immutable upon match completion.

