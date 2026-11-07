# WILDBOUND — TEAM SYSTEM ARCHITECTURE

## 1. System Design

The Team System manages group affiliations, friendly-fire gating, and squad lifecycle without polluting entity classes or duplicating match management.

```
+-----------------------------------------------------------+
|                          Team                             |
|  - TeamId id                                              |
|  - std::string name                                       |
|  - std::vector<uint32_t> memberParticipantIds             |
|  - bool eliminated                                        |
|  - uint32_t placement                                     |
+-----------------------------+-----------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                      TeamManager                          |
|  - CreateTeam(name) -> TeamId                             |
|  - AddParticipantToTeam(teamId, pid)                      |
|  - RemoveParticipantFromTeam(pid)                         |
|  - IsTeammate(pidA, pidB) -> bool                         |
|  - IsEnemy(pidA, pidB) -> bool                            |
|  - GetAliveTeamCount(registry) -> size_t                  |
|  - UpdateTeamEliminationStatus(registry)                  |
+-----------------------------------------------------------+
```

---

## 2. Invariants & Guarantees

1. **Unique Membership**: A participant can belong to at most one team per match. If added to a new team, they are automatically removed from their prior team.
2. **Capacity Validation**: `AddParticipantToTeam` strictly enforces `team.size() < m_maxTeamSize`.
3. **Decoupled Identity**: Participants hold a `uint32_t teamId`, allowing $O(1)$ relationship checks via direct comparison.
4. **Clean Reset**: `TeamManager::Reset()` unbinds all team IDs, clears maps, and resets allocation counters to 1.

