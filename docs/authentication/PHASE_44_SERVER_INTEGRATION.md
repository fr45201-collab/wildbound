# Wildbound — Dedicated Server Identity Integration

## 1. Overview

`WildboundServer` is the authoritative headless dedicated game server. In Phase 44, the dedicated server was integrated with persistent player identities to ensure that match results, eliminations, damage, survival times, and placements are attributed accurately to verified accounts.

---

## 2. Reservation Validation & Identity Binding

When a dedicated server accepts an assignment from `WildboundCoordinator`:
1. The server populates an internal `std::unordered_map<std::string, ParticipantReservation>` mapping connection tickets to player reservations.
2. When a game client connects over ENet and sends its handshake packet containing `connectionTicket`:
   - The server validates the ticket against the active reservation map.
   - If valid, the connected entity is tagged with `persistentUserId`.
   - The player cannot forge or spoof their user ID because the mapping was dictated directly by the Coordinator via the reservation.

---

## 3. Match Result Ingestion Submission

When the match finishes and a victory or timeout condition occurs:
1. `DedicatedServer::FinalizeMatchRecord()` iterates over all participants in the match simulation.
2. For each participant, an entry is added to the participant payload:
   ```json
   {
     "user_id": "b2c6d708-3012-4217-a068-085eec567f1b",
     "participant_display_name": "StormStriker",
     "team_id": 1,
     "placement": 1,
     "eliminations": 4,
     "deaths": 0,
     "damage_dealt": 850.5,
     "damage_received": 140.0,
     "survival_seconds": 720.0,
     "result_status": "winner"
   }
   ```
3. If a player was a bot or unauthenticated guest, `user_id` is passed as `null`.
4. The server places the payload into its persistent outbox directory (`saved_games/outbox/match_results.json`).
5. The HTTP client worker thread transmits the payload to `POST /api/v1/internal/matches/results` using the shared secret header `X-Server-Token`.
6. Upon HTTP 201 Created or 200 OK (idempotent duplicate), the outbox entry is archived.

