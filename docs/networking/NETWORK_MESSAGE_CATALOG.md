# WILDBOUND — NETWORK MESSAGE CATALOG
## Complete Specification of Packet Types, Schemas, and Delivery Semantics

**Document ID:** `WB-NET-011`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. Message Type Registry

Every message carries a 16-bit type identifier in its envelope:

```cpp
namespace wildbound::network {

enum class MessageType : uint16_t {
    // 0x0001 - 0x000F: Handshake & Connection Control
    ClientHello          = 0x0001,
    ServerHello          = 0x0002,
    ConnectionAccepted   = 0x0003,
    ConnectionRejected   = 0x0004,
    Heartbeat            = 0x0005,
    DisconnectRequest    = 0x0006,

    // 0x0010 - 0x001F: Client Inputs & Acknowledgements
    PlayerInput          = 0x0010,
    InputAcknowledgement = 0x0011,

    // 0x0020 - 0x002F: World Replication
    EntitySpawn          = 0x0020,
    EntityDestroy        = 0x0021,
    StateSnapshot        = 0x0022,

    // 0x0030 - 0x004F: Gameplay Actions & Requests
    AttackRequest        = 0x0030,
    InteractionRequest   = 0x0031,
    PickupRequest        = 0x0032,
    DropRequest          = 0x0033,
    EquipRequest         = 0x0034,

    // 0x0050 - 0x005F: Match Lifecycle & World Events
    MatchStateUpdate     = 0x0050,
    StormStateUpdate     = 0x0051,
    PlayerEliminated     = 0x0052,
    MatchResultsNotice   = 0x0053
};

} // namespace wildbound::network
```

---

### 2. Message Specifications

#### 2.1 `MsgClientHello` (0x0001)
* **Direction**: Client $\to$ Server
* **Delivery**: Reliable (Channel 0: Control)
* **Frequency**: Once on connection initiation
* **Payload Fields**:
  - `uint16_t client_protocol_version`: Must equal `PROTOCOL_VERSION (1)`.
  - `std::string player_display_name`: Up to 32 characters UTF-8.
  - `std::string auth_token`: Prototype token string.

#### 2.2 `MsgServerHello` (0x0002)
* **Direction**: Server $\to$ Client
* **Delivery**: Reliable (Channel 0: Control)
* **Frequency**: Response to `MsgClientHello`
* **Payload Fields**:
  - `uint32_t challenge_nonce`: Random 32-bit salt for cryptographic proof / replay protection.
  - `uint16_t server_tick_rate`: e.g. 60 Hz or 30 Hz.

#### 2.3 `MsgConnectionAccepted` (0x0003)
* **Direction**: Server $\to$ Client
* **Delivery**: Reliable (Channel 0: Control)
* **Payload Fields**:
  - `uint16_t assigned_client_id`: Unique client session identifier ($1 - 65535$).
  - `uint32_t assigned_player_net_id`: Entity ID assigned to player's avatar.
  - `std::string match_id`: Unique match identifier.
  - `std::string map_name`: Name of map asset to load (e.g. `battle_royale_island`).

#### 2.4 `MsgConnectionRejected` (0x0004)
* **Direction**: Server $\to$ Client
* **Delivery**: Reliable (Channel 0: Control)
* **Payload Fields**:
  - `uint8_t rejection_code`: `1` (VersionMismatch), `2` (ServerFull), `3` (InvalidAuth), `4` (Banned).
  - `std::string reason`: Human-readable error message displayed on UI.

#### 2.5 `MsgPlayerInput` (0x0010)
* **Direction**: Client $\to$ Server
* **Delivery**: Unreliable Sequenced (Channel 2: Input)
* **Frequency**: 60 Hz
* **Payload Fields**:
  - `uint32_t input_sequence`: Monotonically incrementing input sequence.
  - `float dir_x, dir_y`: Normalized movement vector $[-1.0, 1.0]$.
  - `float facing_angle`: Aim angle in radians $[0, 2\pi)$.
  - `uint8_t action_flags`: Bit 0 = Sprint, Bit 1 = Attack, Bit 2 = Interact.

#### 2.6 `MsgStateSnapshot` (0x0022)
* **Direction**: Server $\to$ Client
* **Delivery**: Unreliable Sequenced (Channel 3: Snapshots)
* **Frequency**: 20–30 Hz
* **Payload Fields**:
  - `uint64_t server_tick`: Server simulation tick.
  - `uint32_t ack_input_sequence`: Last acknowledged input from this client.
  - `uint16_t entity_count`: Number of entities in snapshot.
  - Repeated Entity blocks:
    - `uint32_t net_id`, `uint32_t generation`, `uint16_t dirty_mask`, conditional fields.

#### 2.7 `MsgAttackRequest` (0x0030)
* **Direction**: Client $\to$ Server
* **Delivery**: Reliable (Channel 1: Events)
* **Payload Fields**:
  - `uint8_t weapon_slot`: `0` (Primary), `1` (Secondary).
  - `float aim_angle`: Aim direction at moment of attack.
  - `uint32_t client_tick`: Timestamp of attack trigger.

#### 2.8 `MsgPickupRequest` (0x0032)
* **Direction**: Client $\to$ Server
* **Delivery**: Reliable (Channel 1: Events)
* **Payload Fields**:
  - `NetworkEntityIdentity target_item_id`: Identity of ground loot or chest.

#### 2.9 `MsgMatchStateUpdate` (0x0050)
* **Direction**: Server $\to$ Client
* **Delivery**: Reliable (Channel 4: Match State)
* **Payload Fields**:
  - `uint8_t match_state`: Enum mapping to `match::MatchState` (`Waiting`, `Countdown`, `Active`, `Completed`).
  - `float state_time_remaining`: Seconds remaining in state.
  - `uint16_t alive_count`: Total surviving participants.

#### 2.10 `MsgStormStateUpdate` (0x0051)
* **Direction**: Server $\to$ Client
* **Delivery**: Unreliable Sequenced (Channel 4: Match State)
* **Frequency**: 10 Hz
* **Payload Fields**:
  - `uint8_t current_phase`: Current storm phase index (0–5).
  - `float center_x, center_y`: Current safe zone center.
  - `float current_radius`: Current storm boundary radius.
  - `float target_radius`: Final radius for current shrinking phase.

