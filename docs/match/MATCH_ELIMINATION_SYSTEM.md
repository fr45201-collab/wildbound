# Match Elimination System Specification

## 1. Authoritative Elimination Pipeline

Player eliminations are processed strictly through `wildbound::match::MatchEliminationSystem`:

```mermaid
flowchart TD
    A[Death Event / Health Zero] --> B{Participant Registered?}
    B -- No --> C[Ignore / Untracked Entity]
    B -- Yes --> D{Current Status == Alive?}
    D -- No --> E[Idempotent No-Op: Already Eliminated]
    D -- Yes --> F[Set Status = Eliminated]
    F --> G[Record Timestamp & Cause]
    G --> H{Killer Participant Exists?}
    H -- Yes --> I[Increment Killer Tally & Attribute Name]
    H -- No --> J[Attribute to Storm or Environment]
    I --> K[Update Global Match Statistics]
    J --> K
    K --> L[Dispatch Elimination Callbacks: UI & Audio]
    L --> M[Trigger Final Survivor Evaluation]
```

---

## 2. Elimination Causes

The system supports 5 explicit causes defined in `EliminationCause`:
- `Combat`: Damage applied by weapons, projectiles, or abilities from another competitor.
- `CorruptionStorm`: Entity perished while outside the safe zone boundary due to cumulative corruption damage.
- `Environment`: Hazard or fall damage outside direct competitor combat.
- `Disconnect`: Participant dropped session or conceded.
- `Unknown`: Fallback state when attribution cannot be verified.

---

## 3. Idempotency & Repeat Event Protection

In high-frequency combat loops with damage-over-time ticks, duplicate death notifications may occur within the same frame.
The elimination pipeline checks:
```cpp
if (victim->status == ParticipantStatus::Eliminated) {
    return false;
}
```
This guarantees that victim statistics, killer kill counts, match survivor counts, and leaderboard records are never duplicated.

