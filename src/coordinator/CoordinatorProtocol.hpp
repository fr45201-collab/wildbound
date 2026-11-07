#pragma once

#include "../network/NetworkTypes.hpp"
#include "../network/NetworkProtocol.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace wildbound::coordinator {

using SessionId = uint64_t;
using PartyId = uint64_t;
using LobbyId = uint64_t;
using InvitationId = uint64_t;
using ServerInstanceId = uint64_t;

inline constexpr SessionId INVALID_SESSION_ID = 0;
inline constexpr PartyId INVALID_PARTY_ID = 0;
inline constexpr LobbyId INVALID_LOBBY_ID = 0;
inline constexpr InvitationId INVALID_INVITATION_ID = 0;
inline constexpr ServerInstanceId INVALID_SERVER_ID = 0;

enum class CoordinatorMsgType : uint16_t {
    None                        = 0x0100,

    // Session
    SessionRegisterRequest      = 0x0101,
    SessionRegisterResponse     = 0x0102,
    SessionHeartbeat            = 0x0103,
    SessionDisconnect           = 0x0104,

    // Party
    PartyCreateRequest          = 0x0110,
    PartyCreateResponse         = 0x0111,
    PartyJoinRequest            = 0x0112,
    PartyJoinResponse           = 0x0113,
    PartyLeaveRequest           = 0x0114,
    PartyLeaveResponse          = 0x0115,
    PartySetReadyRequest        = 0x0116,
    PartySetModeRequest         = 0x0117,
    PartyKickRequest            = 0x0118,
    PartyTransferLeaderRequest  = 0x0119,
    PartyStateNotification      = 0x011A,

    // Invitations
    InviteSendRequest           = 0x0120,
    InviteSendResponse          = 0x0121,
    InviteReceivedNotification  = 0x0122,
    InviteRespondRequest        = 0x0123,
    InviteRespondResponse       = 0x0124,

    // Matchmaking
    MatchmakingQueueRequest     = 0x0130,
    MatchmakingQueueResponse    = 0x0131,
    MatchmakingCancelRequest    = 0x0132,
    MatchmakingCancelResponse   = 0x0133,
    MatchmakingStatusUpdate     = 0x0134,

    // Dedicated Server Registry
    ServerRegisterRequest       = 0x0140,
    ServerRegisterResponse      = 0x0141,
    ServerHeartbeat             = 0x0142,
    ServerDeregister            = 0x0143,

    // Match Assignment & Redirection
    MatchReserveRequest         = 0x0150,
    MatchReserveResponse        = 0x0151,
    MatchRedirectNotification   = 0x0152,
    MatchCompletedNotification  = 0x0153
};

// ----------------------------------------------------------------------------
// Session Messages
// ----------------------------------------------------------------------------

struct MsgSessionRegisterRequest {
    std::string displayName;
    std::string clientVersion{ "1.0.0" };
    std::string authToken{};

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgSessionRegisterResponse {
    bool success{ false };
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgSessionHeartbeat {
    SessionId sessionId{ INVALID_SESSION_ID };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

// ----------------------------------------------------------------------------
// Party Messages
// ----------------------------------------------------------------------------

struct PartyMemberInfo {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string displayName;
    bool isLeader{ false };
    bool isReady{ false };
};

struct MsgPartyCreateRequest {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string gameMode{ "solo" };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyCreateResponse {
    bool success{ false };
    PartyId partyId{ INVALID_PARTY_ID };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyJoinRequest {
    SessionId sessionId{ INVALID_SESSION_ID };
    PartyId partyId{ INVALID_PARTY_ID };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyJoinResponse {
    bool success{ false };
    PartyId partyId{ INVALID_PARTY_ID };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyLeaveRequest {
    SessionId sessionId{ INVALID_SESSION_ID };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyLeaveResponse {
    bool success{ false };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartySetReadyRequest {
    SessionId sessionId{ INVALID_SESSION_ID };
    bool isReady{ false };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartySetModeRequest {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string gameMode;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyKickRequest {
    SessionId senderSessionId{ INVALID_SESSION_ID };
    SessionId targetSessionId{ INVALID_SESSION_ID };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyTransferLeaderRequest {
    SessionId currentLeaderSessionId{ INVALID_SESSION_ID };
    SessionId newLeaderSessionId{ INVALID_SESSION_ID };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgPartyStateNotification {
    PartyId partyId{ INVALID_PARTY_ID };
    SessionId leaderSessionId{ INVALID_SESSION_ID };
    std::string gameMode{ "solo" };
    uint32_t maxCapacity{ 1 };
    std::vector<PartyMemberInfo> members;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

// ----------------------------------------------------------------------------
// Invitation Messages
// ----------------------------------------------------------------------------

enum class InvitationType : uint8_t {
    Party = 0,
    Lobby = 1
};

struct MsgInviteSendRequest {
    SessionId senderSessionId{ INVALID_SESSION_ID };
    SessionId recipientSessionId{ INVALID_SESSION_ID };
    InvitationType type{ InvitationType::Party };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgInviteSendResponse {
    bool success{ false };
    InvitationId invitationId{ INVALID_INVITATION_ID };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgInviteReceivedNotification {
    InvitationId invitationId{ INVALID_INVITATION_ID };
    SessionId senderSessionId{ INVALID_SESSION_ID };
    std::string senderDisplayName;
    InvitationType type{ InvitationType::Party };
    uint64_t targetId{ 0 }; // PartyId or LobbyId
    uint32_t expirationSeconds{ 60 };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgInviteRespondRequest {
    InvitationId invitationId{ INVALID_INVITATION_ID };
    SessionId recipientSessionId{ INVALID_SESSION_ID };
    bool accept{ false };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgInviteRespondResponse {
    bool success{ false };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

// ----------------------------------------------------------------------------
// Matchmaking Messages
// ----------------------------------------------------------------------------

struct MsgMatchmakingQueueRequest {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string gameMode{ "solo" };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchmakingQueueResponse {
    bool success{ false };
    std::string gameMode;
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchmakingCancelRequest {
    SessionId sessionId{ INVALID_SESSION_ID };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchmakingCancelResponse {
    bool success{ false };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchmakingStatusUpdate {
    bool isQueued{ false };
    std::string gameMode;
    uint32_t queuePosition{ 0 };
    uint32_t estimatedWaitSeconds{ 0 };
    float elapsedTimeSeconds{ 0.0f };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

// ----------------------------------------------------------------------------
// Dedicated Server Registry Messages
// ----------------------------------------------------------------------------

struct MsgServerRegisterRequest {
    std::string serverName;
    std::string hostAddress;
    uint16_t port{ 7777 };
    uint32_t maxCapacity{ 32 };
    std::string mapName{ "maps/world/battle_royale_island.json" };
    std::string supportedGameMode{ "solo" };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgServerRegisterResponse {
    bool success{ false };
    ServerInstanceId serverId{ INVALID_SERVER_ID };
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgServerHeartbeat {
    ServerInstanceId serverId{ INVALID_SERVER_ID };
    uint32_t currentPlayers{ 0 };
    uint8_t serverState{ 0 }; // 0=Available, 1=Reserved, 2=InMatch, 3=Offline

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

// ----------------------------------------------------------------------------
// Match Reservation & Redirection Messages
// ----------------------------------------------------------------------------

struct ParticipantReservation {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string displayName;
    std::string matchAuthToken;
    uint32_t teamId{ 0 };
    std::string persistentUserId{};
};

struct MsgMatchReserveRequest {
    std::string matchId;
    std::string gameMode;
    std::string mapName;
    std::vector<ParticipantReservation> participants;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchReserveResponse {
    bool accepted{ false };
    std::string matchId;
    std::string errorMessage;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchRedirectNotification {
    std::string matchId;
    std::string serverHost;
    uint16_t serverPort{ 7777 };
    std::string playerAuthToken;
    std::string gameMode;
    std::string mapName;

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

struct MsgMatchCompletedNotification {
    ServerInstanceId serverId{ INVALID_SERVER_ID };
    std::string matchId;
    std::string winnerName;
    uint32_t totalDurationSeconds{ 0 };

    void Serialize(network::ByteStreamWriter& writer) const;
    bool Deserialize(network::ByteStreamReader& reader);
};

} // namespace wildbound::coordinator

