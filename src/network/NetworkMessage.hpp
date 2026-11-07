#pragma once

#include "NetworkTypes.hpp"
#include "NetworkProtocol.hpp"
#include "ReplicationTypes.hpp"
#include <string>
#include <vector>

namespace wildbound::network {

enum class MessageType : uint16_t {
    None                 = 0x0000,
    // Handshake
    ClientHello          = 0x0001,
    ServerHello          = 0x0002,
    ConnectionAccepted   = 0x0003,
    ConnectionRejected   = 0x0004,
    Heartbeat            = 0x0005,
    DisconnectRequest    = 0x0006,

    // Input & Ack
    PlayerInput          = 0x0010,
    InputAcknowledgement = 0x0011,

    // Replication
    EntitySpawn          = 0x0020,
    EntityDestroy        = 0x0021,
    StateSnapshot        = 0x0022,
    PlayerAppearance     = 0x0023,

    // Gameplay
    AttackRequest        = 0x0030,
    PickupRequest        = 0x0032,

    // Match
    MatchStateUpdate     = 0x0050,
    StormStateUpdate     = 0x0051
};

// ----------------------------------------------------------------------------
// Message Structs & Binary Serializers
// ----------------------------------------------------------------------------

struct MsgClientHello {
    uint16_t protocolVersion{ PROTOCOL_VERSION };
    std::string playerName;
    std::string authToken;

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgServerHello {
    uint32_t challengeNonce{ 0 };
    uint16_t serverTickRate{ 60 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgConnectionAccepted {
    ClientId assignedClientId{ 0 };
    NetworkEntityIdentity assignedPlayerId{};
    std::string matchId;
    std::string mapName;

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgConnectionRejected {
    uint8_t rejectionCode{ 0 };
    std::string reason;

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgPlayerInput {
    SequenceNumber inputSequence{ 0 };
    float dirX{ 0.0f };
    float dirY{ 0.0f };
    float facingAngle{ 0.0f };
    uint8_t actionFlags{ 0 }; // 1=Sprint, 2=Attack, 4=Interact

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgAttackRequest {
    uint8_t weaponSlot{ 0 };
    float aimAngle{ 0.0f };
    uint32_t clientTick{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgPickupRequest {
    NetworkEntityIdentity targetItemId{};

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgStateSnapshot {
    TickType serverTick{ 0 };
    SequenceNumber acknowledgedInputSeq{ 0 };
    std::vector<EntityReplicationState> entities;

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgMatchStateUpdate {
    uint8_t matchState{ 0 };
    float timeRemaining{ 0.0f };
    uint16_t aliveCount{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgStormStateUpdate {
    uint8_t currentPhase{ 0 };
    float centerX{ 0.0f };
    float centerY{ 0.0f };
    float currentRadius{ 0.0f };
    float targetRadius{ 0.0f };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

struct MsgPlayerAppearanceSync {
    ClientId targetClientId{ 0 };
    NetworkEntityIdentity targetEntityId{};
    uint32_t loadoutVersion{ 1 };
    std::vector<std::pair<std::string, std::string>> equippedSlots;

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

} // namespace wildbound::network

