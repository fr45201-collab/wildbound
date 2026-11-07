# WILDBOUND — PHASE 45: NETWORK SYNCHRONIZATION SPECIFICATION

## 1. Protocol Architecture & Message Definition

To synchronize character appearance across the network without interfering with high-frequency gameplay physics or movement ticks, Phase 45 introduces a dedicated appearance synchronization message: `MsgPlayerAppearanceSync` (`MessageType::PlayerAppearance = 0x0023`).

### 1.1 Channel Allocation
- **Channel**: `ChannelType::ReliableEvents` (`0x01`).
- **Delivery Mode**: `DeliveryMode::ReliableOrdered`.
- **Reasoning**: Appearance changes are low-frequency, high-importance state synchronization events. Reliable ordered delivery ensures that late appearance updates never arrive out of order and are guaranteed to reach all connected peers.

---

## 2. Binary Message Layout

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Message Type         |          Sequence No          |
|             (0x0023)          |             (uint16)          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Entity ID                           |
|                            (uint32)                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Loadout Version                        |
|                            (uint32)                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Skin ID Length (uint8)    |       Skin ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Head ID Length (uint8)    |       Head ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Chest ID Length (uint8)    |      Chest ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Legs ID Length (uint8)    |       Legs ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Boots ID Length (uint8)    |      Boots ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Back ID Length (uint8)    |       Back ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Weapon ID Length (uint8)    |     Weapon ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Accessory ID Length (uint8)  |  Accessory ID String ...      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### 2.1 C++ Struct Definition (`src/network/NetworkMessage.hpp`)
```cpp
struct MsgPlayerAppearanceSync {
    uint32_t entityId{0};
    uint32_t loadoutVersion{0};
    std::string skinId{"skin_default"};
    std::string headId;
    std::string chestId;
    std::string legsId;
    std::string bootsId;
    std::string backId;
    std::string weaponId;
    std::string accessoryId;

    void Serialize(BinaryWriter& writer) const;
    static MsgPlayerAppearanceSync Deserialize(BinaryReader& reader);
};
```

---

## 3. Server Broadcast & Late-Joiner Catch-Up

### 3.1 Live Broadcast on Equip / Spawn
When a player's loadout is verified by the dedicated server:
1. The server serializes `MsgPlayerAppearanceSync`.
2. It broadcasts the packet to **all** connected clients across `ChannelType::ReliableEvents`.
3. Receiving clients update the corresponding remote player's `ModularCharacter` equipment composite.

### 3.2 Late-Joiner Full Synchronization
When a new client connects midway through a match:
- The server calls `DedicatedServer::SyncAllAppearancesToClient(newPeerId)`.
- It iterates over all active player entities in the game world.
- For every existing player, a `MsgPlayerAppearanceSync` is dispatched specifically to the newly joined peer.
- The late-joining player sees every player wearing their authentic, server-verified appearance upon spawning in the arena.

---

## 4. Bandwidth & Performance Analysis

- **Average Packet Size**: 65 to 110 bytes (including strings and headers).
- **Frequency**: Once per player upon world join; once when changing equipment in lobby or wardrobe.
- **Bandwidth Impact**: Less than 0.05 KB/s under typical 32-player matches.
- **CPU Overhead**: Zero runtime decompression or re-rendering outside of updating texture layer pointers.

