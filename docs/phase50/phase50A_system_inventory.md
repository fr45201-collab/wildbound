# WILDBOUND — PHASE 50A: SYSTEM INVENTORY
**Document Reference:** `docs/phase50/phase50A_system_inventory.md`  
**Parent Document:** Phase 50A Initial Repository Audit  
**Execution Mode:** STRICT AUDIT-ONLY  
**Date:** 2026-11-08  
**Repository Commit:** `e9630150bba720c48eccc5e7d537bc3e8385c84f` (HEAD, branch: `main`)  
**Project Version:** 0.6.0  

---

## 1. Executive Summary

This document presents a comprehensive, empirical inventory of all modules, source files, header interfaces, scripting assets, Go backend packages, database migration schemas, executables, libraries, configuration catalogs, and testing harnesses in the Wildbound repository as of Phase 50A.

### Global Codebase Metrics
- **Total Files (excluding build & git):** 1,168 files
- **Total Source Lines of Code:** 308,889 lines
- **Primary Languages:** C++20, Go 1.22+, Lua 5.4, SQL (PostgreSQL), Python 3.10
- **Configuration & Data Formats:** JSON (541 files), YAML (1 file), CMake (6 files)

---

## 2. C++ Engine Modules (`src/`)

The C++ codebase is partitioned into 27 functional subsystems under `src/`:

| Module Directory | Files | Primary Classes / Components | Subsystem Role |
| :--- | :---: | :--- | :--- |
| `src/ai/` | 12 | `AIAgent`, `BehaviorTree`, `NavMesh`, `Steering` | Local enemy simulation, pathfinding, and behavior arbitration |
| `src/audio/` | 8 | `AudioManager`, `AudioBus`, `SoundEmitter` | raylib audio abstraction, spatial attenuation, volume buses |
| `src/combat/` | 14 | `CombatSystem`, `Combatant`, `Hitbox`, `DamageEvent` | Server-authoritative combat resolution, damage application |
| `src/coordinator/` | 10 | `CoordinatorServer`, `ServerRegistry`, `Matchmaker` | Dedicated server fleet registry, matchmaking queue management |
| `src/core/` | 16 | `Logger`, `EngineConfig`, `TimeStep`, `Timer` | Engine lifecycle, logging subsystem, fixed/variable timesteps |
| `src/cosmetics/` | 12 | `CosmeticRegistry`, `CosmeticItem`, `Wardrobe` | Production cosmetics metadata, equipping validation |
| `src/customization/` | 10 | `CharacterCustomization`, `ColorPalette`, `Layers` | Character appearance assembly, color tinting, modular parts |
| `src/entities/` | 24 | `Entity`, `Player`, `Enemy`, `Creature`, `NPC` | World entity hierarchy, player state machine, creature base |
| `src/gamemodes/` | 8 | `GameModeManager`, `SoloMode`, `SquadsMode` | Game mode rule enforcement (teams, friendly fire, sizing) |
| `src/graphics/` | 18 | `Renderer2D`, `Camera2D`, `RenderTexture`, `YSort` | raylib rendering pipeline, $640 \times 360$ virtual canvas, depth sorting |
| `src/input/` | 8 | `InputManager`, `ActionMap`, `VirtualController` | Keyboard/mouse/gamepad polling, input action mapping |
| `src/interaction/` | 6 | `InteractionSystem`, `Interactable`, `Prompt` | Contextual object interaction (doors, chests, NPCs) |
| `src/items/` | 12 | `ItemManager`, `Inventory`, `LootTable`, `Drop` | Item catalog querying, container management, ground loot |
| `src/match/` | 14 | `MatchManager`, `MatchState`, `Placement` | BR match lifecycle (Waiting, Active, Shrinking, Completed) |
| `src/network/` | 22 | `ConnectionManager`, `PacketSerializer`, `Snapshot` | ENet client/server transport, delta compression, entity sync |
| `src/physics/` | 16 | `PhysicsWorld`, `CollisionGrid`, `SweptAABB` | 2D collision detection, spatial hash grid, wall sliding |
| `src/platform/` | 8 | `PlatformUtils`, `HeadlessStub`, `FileSystem` | Cross-platform utilities, headless raylib/rlImGui stubs |
| `src/resources/` | 16 | `AssetManager`, `AssetRegistry`, `SpriteSheet` | Master AMS manifest parsing, texture caching, sprite slicing |
| `src/results/` | 6 | `ResultsCalculator`, `MatchStats`, `Telemetry` | Match outcome calculation, kill/death/placement compilation |
| `src/scripting/` | 10 | `LuaManager`, `ScriptBindings`, `ScriptSandbox` | Lua 5.4 runtime, sol2 C++ bindings, instruction count safety hook |
| `src/security/` | 8 | `SecurityValidator`, `RateLimiter`, `Sanitizer` | Anti-teleport validation, packet size verification, sanitization |
| `src/server/` | 18 | `DedicatedServer`, `ServerConfig`, `ClientProxy` | Authoritative gameplay simulation, headless loop, state broadcast |
| `src/storm/` | 10 | `StormManager`, `StormPhase`, `SafeZone` | Corruption storm radius contraction, DPS calculation, boundary ring |
| `src/ui/` | 22 | `UIManager`, `HUDView`, `InventoryView`, `DevUI` | In-game HUD, inventory screens, ImGui debug diagnostic overlay |
| `src/utils/` | 10 | `MathUtils`, `StringUtils`, `Random`, `Hash` | Math helpers, GUID generation, vector transformations |
| `src/weapons/` | 10 | `WeaponManager`, `WeaponDefinition`, `Equipment` | Weapon specifications, attack animations, cooldown trackers |
| `src/world/` | 26 | `World`, `MapLoader`, `BiomeManager`, `Building` | Tiled JSON loader, spatial indexing, biome parameters, buildings |

---

## 3. Lua Scripting Inventory (`scripts/`)

| Script File | Category | Lines | Purpose / Implementation Status |
| :--- | :--- | :---: | :--- |
| `scripts/config/engine.lua` | Config | 42 | Engine script configuration defaults |
| `scripts/gameplay/ai/enemy_base.lua` | AI Base | 65 | Base prototype for enemy decision loops |
| `scripts/gameplay/ai/goblin.lua` | AI Archetype | 98 | Cowardly goblin archetype (retreat at <20% HP) |
| `scripts/gameplay/ai/skeleton.lua` | AI Archetype | 92 | Undead melee warrior (fearless, ignores retreat) |
| `scripts/gameplay/ai/behaviors/idle.lua` | AI Behavior | 24 | Standard idle wandering behavior |
| `scripts/gameplay/ai/behaviors/patrol.lua` | AI Behavior | 32 | Waypoint navigation behavior |
| `scripts/gameplay/ai/behaviors/chase.lua` | AI Behavior | 28 | Direct player pursuit behavior |
| `scripts/gameplay/ai/behaviors/attack.lua` | AI Behavior | 35 | Melee engagement distance and cooldown trigger |
| `scripts/gameplay/ai/behaviors/retreat.lua` | AI Behavior | 26 | Flee vector calculation away from combat target |
| `scripts/gameplay/ai/behaviors/investigate.lua` | AI Behavior | 30 | Sound/alert stimulus exploration |
| `scripts/gameplay/ai/behaviors/defend.lua` | AI Behavior | 28 | Guard post positioning and tether boundary |
| `scripts/gameplay/ai/behaviors/return.lua` | AI Behavior | 26 | Return to spawn origin after target loss |

*Notice:* The 3 newly integrated Phase 49 creatures (Marsh Slime, Marsh Viper, Mountain Cyclops) do not yet have specialized `.lua` AI scripts in `scripts/gameplay/ai/` and currently rely on C++ default AI fallbacks.

---

## 4. Go Backend Packages (`backend/`)

The backend is structured under `backend/internal/` and `backend/cmd/`:

| Package | Files | Core Responsibilities |
| :--- | :---: | :--- |
| `cmd/api` | 1 | Backend server entrypoint (`main.go`), routing setup, graceful shutdown |
| `cmd/migrate` | 1 | Standalone database migration CLI executable |
| `internal/auth` | 7 | Argon2id password hashing, JWT creation/refresh rotation, session verification |
| `internal/account` | 2 | Account registration, profile initialization, credential updates |
| `internal/profiles` | 3 | Player display name, level, XP, bio, avatar settings |
| `internal/progression` | 3 | Account XP curve, rank tiers, level up logic |
| `internal/cosmetics` | 3 | Cosmetic catalogue retrieval, store listings, equipping validations |
| `internal/inventory` | 2 | Player owned item queries, drop granting |
| `internal/matches` | 3 | Dedicated server match result ingest, player placement, kill logs |
| `internal/statistics` | 3 | Player K/D ratio, matches played, wins, damage dealt metrics |
| `internal/settings` | 2 | User audio/video/keybind cloud preferences synchronization |
| `internal/achievements`| 2 | Achievement progress tracking and unlocking triggers |
| `internal/database` | 5 | PostgreSQL `pgxpool` connection pool, transaction helpers, health check |
| `internal/middleware` | 7 | JWT auth, Server-to-Server auth, CORS, Rate limiting, RequestID, Logging |
| `internal/repository` | 11 | Direct PostgreSQL SQL queries, data access interfaces |
| `internal/validation` | 2 | Input regex, password complexity, SQL injection prevention sanitizers |
| `internal/server` | 1 | Chi HTTP router configuration, route grouping |
| `internal/response` | 1 | Standardized JSON envelopes (`data`, `error`, `status`) |
| `internal/observability`| 1 | Structured log/slog handler with request tracing |

---

## 5. PostgreSQL Database Migrations (`backend/migrations/`)

| Migration File | Direction | Version | Schema Impact |
| :--- | :---: | :---: | :--- |
| `000001_initial_schema.up.sql` | UP | 1 | Creates `users`, `profiles`, `sessions`, `match_history`, `player_stats` |
| `000002_seed_cosmetics.up.sql` | UP | 2 | Creates `cosmetic_items`, seeds 50 initial cosmetics |
| `000003_add_last_login_and_inventory.up.sql` | UP | 3 | Adds `user_inventory`, `last_login_at` column |
| `000004_auth_progression_and_settings.up.sql` | UP | 4 | Creates `account_settings`, `user_progression`, `achievements` |
| `000005_cosmetics_loadouts.up.sql` | UP | 5 | Creates `equipped_loadouts` with optimistic lock `version` column |

---

## 6. Build Targets & Executables Inventory

### C++ Executables (`build/`)
1. **`wildbound` (`wildbound.exe`):**
   - Source: `src/main.cpp`
   - Size: 13.85 MB (Debug), 3.59 MB (Release)
   - Role: Game client with raylib graphics, sound, ImGui DevUI, and local/remote networking.
2. **`WildboundServer` (`WildboundServer.exe`):**
   - Source: `src/server/main_server.cpp`
   - Size: 14.14 MB (Debug), 1.68 MB (Release)
   - Role: Headless authoritative dedicated gameplay server.
3. **`WildboundCoordinator` (`WildboundCoordinator.exe`):**
   - Source: `src/coordinator/main_coordinator.cpp`
   - Size: 2.20 MB (Debug), 281 KB (Release)
   - Role: Headless central matchmaking coordinator and server fleet manager.

### Go Executables (`backend/bin/`)
4. **`wildbound-backend.exe`:**
   - Source: `backend/cmd/api/main.go`
   - Role: HTTP REST API backend server (port 8080/8081).
5. **`wildbound-migrate.exe`:**
   - Source: `backend/cmd/migrate/main.go`
   - Role: Database migration tool.

---

## 7. Data Catalogs & Maps Inventory

### Data Catalogs (`data/`)
- `data/biomes/`: 5 biome definitions (`forest.json`, `desert.json`, `snow.json`, `swamp.json`, `mountain.json`).
- `data/cosmetics/`: `costume_catalog.json` (50 cosmetic definitions).
- `data/gamemodes/`: 5 game mode files (`solo.json`, `duos.json`, `squads.json`, `custom.json`, `training.json`).
- `data/items/`: `item_catalog.json` (40 item definitions).
- `data/match/`: `default_match.json` (standard match duration, storm timer, player limits).

### World Maps (`maps/`)
- `maps/world/battle_royale_island.json`: Master battle royale map ($320 \times 320$ tiles, 6 layers, 448 placed objects, 41,503 collision bounds).
- `maps/world/battle_royale_island_decorations.json`: Environmental decorative overlay.

