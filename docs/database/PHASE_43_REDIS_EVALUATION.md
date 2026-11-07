# Wildbound — Phase 43: Redis Evaluation

**Project:** Wildbound  
**Phase:** 43 — Database Infrastructure  
**Subject:** Evaluation of Redis for Caching and Ephemeral State  
**Status:** Evaluation Completed — **Redis is Not Justified at Current Stage**

---

## 1. Executive Summary

This document evaluates whether introducing **Redis** is justified for Wildbound at this stage of development. 

Redis was evaluated across four primary potential use cases:
1. Session and Token Caching.
2. In-Memory Matchmaking Queues and Party State.
3. Live Player Leaderboards and Statistics.
4. Match Results Buffering.

### Verdict: **NOT JUSTIFIED**
After architectural analysis of Wildbound's existing components (specifically `WildboundCoordinator`, `WildboundServer` outbox spooling, and PostgreSQL indexing), introducing Redis at this stage introduces **unnecessary operational complexity, infrastructure overhead, network serialization cost, and split-brain / cache invalidation risks** without solving any active performance bottlenecks.

---

## 2. Evaluation by Use Case

### 2.1 Session & JWT Caching
- **Candidate Function**: Storing active player sessions, JWT blacklist / token revocations.
- **Current Wildbound Architecture**:
  - Stateless JWTs are used for access tokens (15-minute TTL) with cryptographic signature validation (`HS256`).
  - Refresh tokens (7-day TTL) are stored in PostgreSQL table `refresh_tokens`, hashed with SHA-256 and queried on refresh requests only.
  - Refresh requests occur infrequently (at most once every 10–15 minutes per active client).
  - PostgreSQL easily serves sub-millisecond point lookups on `idx_refresh_tokens_hash`.
- **Verdict**: A dedicated cache for sessions provides negligible latency improvements while adding cache-synchronization complexity.

### 2.2 Matchmaking Queues & Party State
- **Candidate Function**: Managing lobbies, party groups, match queues, and contender invites.
- **Current Wildbound Architecture**:
  - Implemented in **`WildboundCoordinator`** (Phase 41) directly in high-performance C++20 memory using standard concurrency primitives and sub-millisecond tick rate loops.
  - The Coordinator maintains complete authoritative state for active lobbies, matchmaking tickets, party formations, and dedicated server allocations in-process.
  - When matchmaking completes, the coordinator assigns tickets directly to `WildboundServer` dedicated instances via ENet.
- **Verdict**: Putting matchmaking queues into Redis would require serializing C++ structs across network sockets into Redis data types and polling/subscribing, strictly increasing latency compared to in-memory Coordinator processing.

### 2.3 Live Leaderboards & Statistics
- **Candidate Function**: Storing sorted sets (`ZSET`) for global contender leaderboards and rankings.
- **Current Wildbound Architecture**:
  - Contender statistics are stored in `player_statistics` and queried by `user_id` on the profile screen.
  - Global rankings are computed through indexed queries on `player_statistics(matches_won DESC, eliminations DESC)`.
  - For millions of players, read-replicas or materialized views with periodic refresh (`REFRESH MATERIALIZED VIEW CONCURRENTLY`) provide full consistency without cache desynchronization.
- **Verdict**: Premature optimization. PostgreSQL handles current player query volumes with index scans in under 1 millisecond.

### 2.4 Match Results Buffering & Ingestion
- **Candidate Function**: Ingesting match completion results from dedicated servers into a temporary message queue.
- **Current Wildbound Architecture**:
  - `WildboundServer` features an on-disk, append-only crash-resilient spool outbox (`MatchResultsOutbox`, Phase 42).
  - When a match ends, the dedicated server immediately writes the structured JSON payload to local disk before attempting HTTP transmission to the Go backend (`POST /api/v1/internal/matches/results`).
  - If the backend or database is unreachable, the server continues retrying with exponential backoff and jitter without losing match results.
  - The Go backend writes match records and participant stats within atomic ACID transactions.
- **Verdict**: Redis Pub/Sub or Streams would duplicate the durable disk outbox already operational on dedicated servers.

---

## 3. Operational & Architectural Costs of Redis

1. **Deployment Footprint**: Requires running, configuring, securing, monitoring, and backing up an additional standalone stateful service.
2. **Dual-Write Consistency**: Writing to both Redis and PostgreSQL creates race conditions, partial-write failures, and eventual consistency dilemmas.
3. **Cache Invalidation Overhead**: Requires complex cache eviction hooks on every profile edit, item consume, and match record insert.
4. **Memory Footprint**: In-memory storage of duplicate relational records increases cloud infrastructure cost.

---

## 4. Trigger Criteria for Future Redis Adoption

Redis should be reconsidered only when Wildbound reaches the following scale thresholds:
1. **Concurrent Active Contenders > 50,000**: When database connection limits are saturated despite connection pooling.
2. **Multi-Region Coordination**: When multiple `WildboundCoordinator` instances across North America, Europe, and Asia require a shared distributed lock manager (e.g. Redlock) or low-latency cross-region pub/sub.
3. **Real-time Global Leaderboard Throttling**: When real-time rank updates across millions of players require sub-millisecond `ZADD` / `ZRANK` operations.

