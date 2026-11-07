# WILDBOUND — EXISTING GAMEPLAY NETWORK INTEGRATION
## Adapting Phases 0–34 Gameplay Subsystems into Authoritative Multiplayer

**Document ID:** `WB-NET-017`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. Overview

Wildbound possesses rich gameplay systems implemented across 34 development phases. The primary goal of Phase 35 is to map these existing, proven offline systems into the client-server paradigm **without rewriting them from scratch**.

---

### 2. Subsystem Mapping & Migration Plan

#### 2.1 Combat & Health Systems (Phases 5 & 19)
- **Current Offline State**: `CombatSystem.cpp` updates projectiles, checks damage events, and applies damage directly to `HealthComponent`.
- **Server Integration**:
  - `AuthoritativeCombatSystem` runs on the dedicated server.
  - Receives `MsgAttackRequest`, simulates projectile trajectories, performs swept AABB checks.
  - On hit, calls `HealthComponent::ApplyDamage()`.
- **Client Integration**:
  - Client spawns a visual projectile dummy on attack.
  - On receipt of `MsgDamageApplied` or `MsgStateSnapshot`, client displays floating combat text and plays sound effects.
  - Visual projectile dummy is snapped or destroyed on server confirmation.

#### 2.2 Character Customization & Cosmetics (Phases 7 & 26)
- **Current Offline State**: `ModularCharacter` renders composite sprite layers based on local `CharacterEquipment`.
- **Server Integration**:
  - The server stores the equipped cosmetic IDs (`shirt_forest_tunic`, `hair_short_brown`) per player in `MatchParticipant`.
  - Replicates these cosmetic IDs to remote clients during `MsgEntitySpawn`.
- **Client Integration**:
  - Client receives cosmetic IDs and assigns textures from local `AssetManager` to the remote player's proxy `ModularCharacter`. Zero mesh/texture data is sent over the wire.

#### 2.3 Corruption Storm (Phase 31)
- **Current Offline State**: `StormManager` updates phase timers, lerps circle bounds, and applies storm DPS to `m_player` and active `Enemy` competitors.
- **Server Integration**:
  - `StormManager` runs identically on the server! It manages safe zones and deals damage to all registered participants in `MatchParticipantRegistry`.
  - Replicates `MsgStormStateUpdate` (center, current radius, target radius) at 10 Hz.
- **Client Integration**:
  - Client's `StormBoundaryRenderer` receives the replicated center and radius and renders the purple storm wall shader and minimap indicator.
  - Client does NOT compute storm damage. Damage arrives purely via server health updates.

#### 2.4 Match Lifecycle & Results (Phases 32 & 33)
- **Current Offline State**: `MatchManager` controls match state machine transitions (`Waiting`, `Countdown`, `Active`, `Finishing`, `Completed`). `MatchResultManager` computes placements and persists JSON match history.
- **Server Integration**:
  - `MatchManager` is the master match arbiter on the server.
  - It tracks human and AI participants, triggers countdown when minimum players connect, evaluates last survivor, and finalizes results.
  - Broadcasts `MsgMatchStateUpdate` on state transitions.
  - Authoritative match results are saved on the server and broadcast via `MsgMatchResultsNotice`.
- **Client Integration**:
  - Client listens for `MsgMatchStateUpdate` to switch local UI screens (e.g. show countdown HUD, show post-match victory/defeat screen).

#### 2.5 Multi-Team Game Modes (Phase 34)
- **Current Offline State**: `GameModeManager` and `TeamManager` enforce team capacities, friendly fire immunity, and clustered spawn locations.
- **Server Integration**:
  - `GameModeManager` runs on the server. When clients connect, the server assigns them to squads based on game mode rules (Solo, Duos, Squads).
  - Friendly fire filtering in `CombatRules` runs authoritatively on the server, ensuring teammates cannot damage each other.
- **Client Integration**:
  - Client receives `teamId` in `MsgConnectionAccepted` and highlights teammates with green HUD health bars and overhead markers.

#### 2.6 AI Bot Competitors (Phases 5 & 29)
- **Current Offline State**: `AIManager` and `EnemySpawner` run state machines and pathfinding for bot competitors.
- **Server Integration**:
  - Bots execute entirely on the dedicated server.
  - The server treats bots as standard participants registered in `MatchParticipantRegistry`.
  - Bot positions, animations, and combat actions replicate to clients identically to human players.
- **Client Integration**:
  - Clients have no idea whether a remote entity is an AI bot or a human player; both replicate as standard `NetworkEntityIdentity` proxies.

