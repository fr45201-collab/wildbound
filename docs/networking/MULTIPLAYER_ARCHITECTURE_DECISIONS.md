# WILDBOUND — ARCHITECTURE DECISION RECORDS (ADRs)
## Key Architectural Decisions, Alternatives Evaluated, Rationale, and Consequences

**Document ID:** `WB-NET-019`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### ADR-01: Client-Server Architecture vs. Peer-to-Peer (P2P)
- **Status**: Approved.
- **Context**: Multiplayer top-down fantasy Battle Royale with up to 60 players.
- **Options Considered**: 
  1. Peer-to-Peer (deterministic lockstep or distributed state mesh).
  2. Authoritative Client-Server.
- **Decision**: Authoritative Client-Server.
- **Rationale**: Peer-to-peer is completely unsuitable for competitive Battle Royale. P2P exposes all player IP addresses, requires consensus between all clients, suffers when one client lags, and permits game-breaking memory manipulation (god mode, wallhacks) because no single peer is trusted. Client-Server centralizes authority and prevents cheating.
- **Consequences**: Requires hosting dedicated server instances, but provides robust security, lower client bandwidth, and seamless spectator/reconnect capabilities.

---

### ADR-02: Dedicated Server vs. Listen Server
- **Status**: Approved.
- **Context**: Should one player act as both host and local client (listen server)?
- **Options Considered**:
  1. Listen Server (host player hosts local game).
  2. Dedicated Server (headless standalone process).
- **Decision**: Dedicated Server for competitive matchmaking; optional listen server for offline local development only.
- **Rationale**: Listen servers give the hosting player 0 ms ping (massive host advantage), expose the host's IP to all competitors, terminate the match if the host leaves, and allow the host to inspect process memory to view enemy positions. Dedicated servers guarantee neutral arbitration.
- **Consequences**: Engine must support headless compilation and execution without raylib rendering contexts.

---

### ADR-03: Transport Protocol Selection (ENet over UDP)
- **Status**: Approved.
- **Context**: Selecting the underlying network transport library.
- **Options Considered**:
  1. Raw TCP Sockets.
  2. Raw UDP Sockets with homegrown reliability layer.
  3. ENet.
  4. Steam Networking Sockets / GameNetworkingSockets.
- **Decision**: ENet over UDP, wrapped in `INetworkTransport`.
- **Rationale**: TCP suffers from catastrophic Head-of-Line blocking (a single dropped packet freezes all game updates). Raw UDP requires months of error-prone re-implementation of fragmentation, sequencing, and ACKs. ENet is a battle-tested, lightweight, C/C++ library providing virtual multi-channel sequencing, reliable ordered delivery, and unreliable unsequenced delivery without heavy framework bloat.
- **Consequences**: ENet must be integrated via CMake FetchContent in Phase 36.

---

### ADR-04: Absolute Server Authority Over Game Rules
- **Status**: Approved.
- **Context**: Determining which machine is authoritative over health, damage, hits, and loot.
- **Options Considered**:
  1. Client-authoritative hit detection (client declares hits, server accepts).
  2. Server-authoritative hit detection and damage simulation.
- **Decision**: Absolute Server Authority.
- **Rationale**: In client-authoritative shooters, cheat clients can send forged "hit" packets for enemies anywhere on the map (instant headshots through walls). Server-side hit detection ensures all hits satisfy physical trajectory, collision, line-of-sight, and cooldown validation.
- **Consequences**: Server CPU load is higher, and server must maintain full collision geometry.

---

### ADR-05: Generational Network Entity Identity
- **Status**: Approved.
- **Context**: Identifying entities across network packets without pointer addresses or slot aliasing.
- **Options Considered**:
  1. Raw 32-bit ID index.
  2. 128-bit UUID strings.
  3. 64-bit Generational Identity (32-bit ID + 32-bit Generation).
- **Decision**: 64-bit Generational Identity (`NetworkEntityIdentity`).
- **Rationale**: Raw 32-bit IDs suffer from stale reference aliasing when slots are recycled after entity destruction. UUIDs waste 16 bytes per reference. A 64-bit handle (32-bit slot ID + 32-bit generation counter) enables $O(1)$ table lookups and completely eliminates stale packet bugs.
- **Consequences**: All networked components and messages reference `NetworkEntityIdentity` rather than raw entity pointers.

---

### ADR-06: Explicit Binary Serialization vs. Raw Struct Casts / JSON
- **Status**: Approved.
- **Context**: Choosing the wire serialization format.
- **Options Considered**:
  1. JSON / Text strings over UDP.
  2. Direct C++ struct memory casting (`memcpy(&struct, buffer, sizeof(struct))`).
  3. Explicit Little-Endian field-by-field binary serialization (`ByteStreamWriter`/`Reader`).
- **Decision**: Explicit Little-Endian field serialization.
- **Rationale**: JSON is bloated and slow to parse. Struct memory casting is dangerous and undefined behavior across compilers, architectures, and platforms due to struct packing, alignment padding, and endianness. Explicit byte serialization guarantees portability, bounds safety, and minimal packet size.
- **Consequences**: Every message requires explicit `Serialize()` and `Deserialize()` routines.

---

### ADR-07: Channel Separation for Reliable vs. Unreliable Data
- **Status**: Approved.
- **Context**: Handling packet loss across different gameplay data streams.
- **Options Considered**:
  1. All reliable packets.
  2. All unreliable packets.
  3. 5-Channel Virtual Mesh (Reliable Control, Reliable Events, Unreliable Input, Unreliable Snapshots, Reliable Match State).
- **Decision**: 5-Channel Virtual Mesh.
- **Rationale**: Reliable delivery of 60 Hz input or 30 Hz snapshots creates disastrous retransmission cascades under packet loss. Dividing traffic into channels guarantees that time-critical position updates are never stalled by a dropped event packet.
- **Consequences**: Senders must specify the exact channel per message.

---

### ADR-08: Fixed Simulation Tick Rate (60 Hz / 30 Hz)
- **Status**: Approved.
- **Context**: Decoupling server physics from client display refresh rates.
- **Options Considered**:
  1. Variable `dt` tick matching server loop.
  2. Fixed 60 Hz simulation tick.
  3. Fixed 30 Hz simulation tick.
- **Decision**: Configurable Fixed Timestep (Default 60 Hz; fallback 30 Hz for high loads).
- **Rationale**: Variable delta times break simulation determinism and make speed hacking trivial to execute. A fixed timestep ensures reproducible collision and combat physics.
- **Consequences**: Server loop uses an accumulator; client interpolation buffers handle rendering between snapshots.

---

### ADR-09: Snapshot Broadcast Frequency (30 Hz)
- **Status**: Approved.
- **Context**: Frequency of server-to-client world updates.
- **Options Considered**:
  1. Broadcast every tick (60 Hz).
  2. Broadcast at 30 Hz.
  3. Broadcast at 15 Hz.
- **Decision**: 30 Hz Snapshot Broadcast with Delta Dirty-Masking.
- **Rationale**: 60 Hz snapshot broadcast doubles server bandwidth with negligible visual improvement over an interpolated 30 Hz stream. 15 Hz increases interpolation latency to 200 ms. 30 Hz provides optimal smoothness ($33.3\text{ ms}$ intervals) while keeping bandwidth below $45\text{ KB/s}$ per client.
- **Consequences**: Client interpolation buffer is set to $100\text{ ms}$ (3 snapshot intervals).

---

### ADR-10: Client Movement Prediction with Server Reconciliation
- **Status**: Approved.
- **Context**: Eliminating perceived movement latency for the local player.
- **Options Considered**:
  1. Authoritative no-prediction (player waits for server ack before moving).
  2. Full client-side prediction and server reconciliation for local movement.
- **Decision**: Full client-side prediction and server reconciliation for local movement only.
- **Rationale**: A top-down action game feels unplayable if the avatar hesitates for 80 ms after WASD input. Local prediction provides immediate tactile responsiveness. Server reconciliation ensures that if the client drifts or attempts to clip through a wall, it is snapped back to the authoritative coordinate.
- **Consequences**: Client maintains an input history buffer and can replay inputs forward after a server correction.

---

### ADR-11: Remote Entity Interpolation Buffer (100 ms Delay)
- **Status**: Approved.
- **Context**: Displaying remote opponents and bots smoothly on client screens.
- **Options Considered**:
  1. Display latest received position directly.
  2. Interpolate between buffered snapshots with a 100 ms delay.
- **Decision**: Interpolate between buffered snapshots with 100 ms delay.
- **Rationale**: Network packets suffer from arrival jitter. Directly displaying the newest position results in micro-stutters and jittery aim targets. Interpolating between two buffered snapshots in the recent past guarantees continuous, buttery-smooth visual motion at 60–144 FPS.
- **Consequences**: The visual representation of remote players is $100\text{ ms}$ behind the server's real-time state.

---

### ADR-12: Spatial Grid Interest Management
- **Status**: Approved.
- **Context**: Preventing $O(N^2)$ bandwidth scaling and anti-ESP data leaks on a large island.
- **Options Considered**:
  1. Broadcast all entities to all clients.
  2. Distance radius check ($O(N^2)$ distance calculations on server).
  3. Uniform Spatial Grid ($3 \times 3$ active interest cells per player).
- **Decision**: Uniform Spatial Grid ($640 \times 360$ unit cells, $3 \times 3$ active cluster).
- **Rationale**: Spatial grid lookups are $O(1)$ per entity. Entities outside the $3 \times 3$ cell cluster around a player are completely omitted from that player's snapshot, saving $70\%+$ bandwidth and making memory-reading wallhack cheats ineffective.
- **Consequences**: Server maintains entity memberships in grid cells as they move.

---

### ADR-13: Headless Server Architecture (Decoupling Raylib)
- **Status**: Approved.
- **Context**: Running dedicated servers on headless cloud Linux nodes without GPUs.
- **Options Considered**:
  1. Run client executable with hidden window.
  2. Native Headless Server executable (`wildbound_server`) with zero raylib window/audio dependencies.
- **Decision**: Native Headless Server executable.
- **Rationale**: Running virtual X11 framebuffers or hidden windows on cloud servers wastes hundreds of megabytes of RAM, consumes GPU/CPU cycles on invisible pixels, and crashes in standard containerized environments. A pure headless server runs cleanly anywhere.
- **Consequences**: Single-player and dedicated server share core simulation logic, but `wildbound_server` bypasses `Renderer`, `AudioSystem`, and `DevUI`.

---

### ADR-14: Strict Protocol Versioning & Magic Handshake
- **Status**: Approved.
- **Context**: Rejecting outdated or malicious client connections.
- **Options Considered**:
  1. Implicit packet parsing without version check.
  2. Versioned 16-byte fixed envelope with protocol magic (`WBND`) and integer version.
- **Decision**: Versioned 16-byte fixed envelope.
- **Rationale**: Prevents protocol corruption, garbage packet crashes, and outdated game clients from joining newer servers.
- **Consequences**: Protocol version must be incremented whenever message schemas change.

---

### ADR-15: Disconnection Grace Period & Reconnect Policy
- **Status**: Approved.
- **Context**: Handling transient WiFi drops and mobile packet loss during matches.
- **Options Considered**:
  1. Instant elimination on disconnect.
  2. 15-second grace period with in-world avatar retention.
- **Decision**: 15-second grace period with in-world avatar retention.
- **Rationale**: Instant elimination punishes players for minor router hiccups. Conversely, vanishing instantly allows players to cheat ("combat logging" when about to lose a fight). Retaining the avatar in the world for 15 seconds allows legitimate players to reconnect while ensuring combat integrity.
- **Consequences**: Player avatar must support a `Disconnected` visual indicator.

