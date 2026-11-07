#include "CoordinatorProtocol.hpp"

namespace wildbound::coordinator {

// MsgSessionRegisterRequest
void MsgSessionRegisterRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteString(displayName);
    writer.WriteString(clientVersion);
    writer.WriteString(authToken);
}
bool MsgSessionRegisterRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(2)) return false;
    displayName = reader.ReadString();
    clientVersion = reader.ReadString();
    if (reader.HasRemaining(1)) {
        authToken = reader.ReadString();
    }
    return true;
}

// MsgSessionRegisterResponse
void MsgSessionRegisterResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteU64(sessionId);
    writer.WriteString(errorMessage);
}
bool MsgSessionRegisterResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    success = (reader.ReadU8() != 0);
    sessionId = reader.ReadU64();
    errorMessage = reader.ReadString();
    return true;
}

// MsgSessionHeartbeat
void MsgSessionHeartbeat::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
}
bool MsgSessionHeartbeat::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(8)) return false;
    sessionId = reader.ReadU64();
    return true;
}

// MsgPartyCreateRequest
void MsgPartyCreateRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
    writer.WriteString(gameMode);
}
bool MsgPartyCreateRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    sessionId = reader.ReadU64();
    gameMode = reader.ReadString();
    return true;
}

// MsgPartyCreateResponse
void MsgPartyCreateResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteU64(partyId);
    writer.WriteString(errorMessage);
}
bool MsgPartyCreateResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    success = (reader.ReadU8() != 0);
    partyId = reader.ReadU64();
    errorMessage = reader.ReadString();
    return true;
}

// MsgPartyJoinRequest
void MsgPartyJoinRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
    writer.WriteU64(partyId);
}
bool MsgPartyJoinRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(16)) return false;
    sessionId = reader.ReadU64();
    partyId = reader.ReadU64();
    return true;
}

// MsgPartyJoinResponse
void MsgPartyJoinResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteU64(partyId);
    writer.WriteString(errorMessage);
}
bool MsgPartyJoinResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    success = (reader.ReadU8() != 0);
    partyId = reader.ReadU64();
    errorMessage = reader.ReadString();
    return true;
}

// MsgPartyLeaveRequest
void MsgPartyLeaveRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
}
bool MsgPartyLeaveRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(8)) return false;
    sessionId = reader.ReadU64();
    return true;
}

// MsgPartyLeaveResponse
void MsgPartyLeaveResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteString(errorMessage);
}
bool MsgPartyLeaveResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(1)) return false;
    success = (reader.ReadU8() != 0);
    errorMessage = reader.ReadString();
    return true;
}

// MsgPartySetReadyRequest
void MsgPartySetReadyRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
    writer.WriteU8(isReady ? 1 : 0);
}
bool MsgPartySetReadyRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    sessionId = reader.ReadU64();
    isReady = (reader.ReadU8() != 0);
    return true;
}

// MsgPartySetModeRequest
void MsgPartySetModeRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
    writer.WriteString(gameMode);
}
bool MsgPartySetModeRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    sessionId = reader.ReadU64();
    gameMode = reader.ReadString();
    return true;
}

// MsgPartyKickRequest
void MsgPartyKickRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(senderSessionId);
    writer.WriteU64(targetSessionId);
}
bool MsgPartyKickRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(16)) return false;
    senderSessionId = reader.ReadU64();
    targetSessionId = reader.ReadU64();
    return true;
}

// MsgPartyTransferLeaderRequest
void MsgPartyTransferLeaderRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(currentLeaderSessionId);
    writer.WriteU64(newLeaderSessionId);
}
bool MsgPartyTransferLeaderRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(16)) return false;
    currentLeaderSessionId = reader.ReadU64();
    newLeaderSessionId = reader.ReadU64();
    return true;
}

// MsgPartyStateNotification
void MsgPartyStateNotification::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(partyId);
    writer.WriteU64(leaderSessionId);
    writer.WriteString(gameMode);
    writer.WriteU32(maxCapacity);
    writer.WriteU32(static_cast<uint32_t>(members.size()));
    for (const auto& m : members) {
        writer.WriteU64(m.sessionId);
        writer.WriteString(m.displayName);
        writer.WriteU8(m.isLeader ? 1 : 0);
        writer.WriteU8(m.isReady ? 1 : 0);
    }
}
bool MsgPartyStateNotification::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(24)) return false;
    partyId = reader.ReadU64();
    leaderSessionId = reader.ReadU64();
    gameMode = reader.ReadString();
    maxCapacity = reader.ReadU32();
    uint32_t count = reader.ReadU32();
    members.clear();
    members.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        PartyMemberInfo m;
        m.sessionId = reader.ReadU64();
        m.displayName = reader.ReadString();
        m.isLeader = (reader.ReadU8() != 0);
        m.isReady = (reader.ReadU8() != 0);
        members.push_back(m);
    }
    return true;
}

// MsgInviteSendRequest
void MsgInviteSendRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(senderSessionId);
    writer.WriteU64(recipientSessionId);
    writer.WriteU8(static_cast<uint8_t>(type));
}
bool MsgInviteSendRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(17)) return false;
    senderSessionId = reader.ReadU64();
    recipientSessionId = reader.ReadU64();
    type = static_cast<InvitationType>(reader.ReadU8());
    return true;
}

// MsgInviteSendResponse
void MsgInviteSendResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteU64(invitationId);
    writer.WriteString(errorMessage);
}
bool MsgInviteSendResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    success = (reader.ReadU8() != 0);
    invitationId = reader.ReadU64();
    errorMessage = reader.ReadString();
    return true;
}

// MsgInviteReceivedNotification
void MsgInviteReceivedNotification::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(invitationId);
    writer.WriteU64(senderSessionId);
    writer.WriteString(senderDisplayName);
    writer.WriteU8(static_cast<uint8_t>(type));
    writer.WriteU64(targetId);
    writer.WriteU32(expirationSeconds);
}
bool MsgInviteReceivedNotification::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(29)) return false;
    invitationId = reader.ReadU64();
    senderSessionId = reader.ReadU64();
    senderDisplayName = reader.ReadString();
    type = static_cast<InvitationType>(reader.ReadU8());
    targetId = reader.ReadU64();
    expirationSeconds = reader.ReadU32();
    return true;
}

// MsgInviteRespondRequest
void MsgInviteRespondRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(invitationId);
    writer.WriteU64(recipientSessionId);
    writer.WriteU8(accept ? 1 : 0);
}
bool MsgInviteRespondRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(17)) return false;
    invitationId = reader.ReadU64();
    recipientSessionId = reader.ReadU64();
    accept = (reader.ReadU8() != 0);
    return true;
}

// MsgInviteRespondResponse
void MsgInviteRespondResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteString(errorMessage);
}
bool MsgInviteRespondResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(1)) return false;
    success = (reader.ReadU8() != 0);
    errorMessage = reader.ReadString();
    return true;
}

// MsgMatchmakingQueueRequest
void MsgMatchmakingQueueRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
    writer.WriteString(gameMode);
}
bool MsgMatchmakingQueueRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    sessionId = reader.ReadU64();
    gameMode = reader.ReadString();
    return true;
}

// MsgMatchmakingQueueResponse
void MsgMatchmakingQueueResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteString(gameMode);
    writer.WriteString(errorMessage);
}
bool MsgMatchmakingQueueResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(1)) return false;
    success = (reader.ReadU8() != 0);
    gameMode = reader.ReadString();
    errorMessage = reader.ReadString();
    return true;
}

// MsgMatchmakingCancelRequest
void MsgMatchmakingCancelRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(sessionId);
}
bool MsgMatchmakingCancelRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(8)) return false;
    sessionId = reader.ReadU64();
    return true;
}

// MsgMatchmakingCancelResponse
void MsgMatchmakingCancelResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteString(errorMessage);
}
bool MsgMatchmakingCancelResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(1)) return false;
    success = (reader.ReadU8() != 0);
    errorMessage = reader.ReadString();
    return true;
}

// MsgMatchmakingStatusUpdate
void MsgMatchmakingStatusUpdate::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(isQueued ? 1 : 0);
    writer.WriteString(gameMode);
    writer.WriteU32(queuePosition);
    writer.WriteU32(estimatedWaitSeconds);
    writer.WriteFloat(elapsedTimeSeconds);
}
bool MsgMatchmakingStatusUpdate::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(13)) return false;
    isQueued = (reader.ReadU8() != 0);
    gameMode = reader.ReadString();
    queuePosition = reader.ReadU32();
    estimatedWaitSeconds = reader.ReadU32();
    elapsedTimeSeconds = reader.ReadFloat();
    return true;
}

// MsgServerRegisterRequest
void MsgServerRegisterRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteString(serverName);
    writer.WriteString(hostAddress);
    writer.WriteU16(port);
    writer.WriteU32(maxCapacity);
    writer.WriteString(mapName);
    writer.WriteString(supportedGameMode);
}
bool MsgServerRegisterRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(10)) return false;
    serverName = reader.ReadString();
    hostAddress = reader.ReadString();
    port = reader.ReadU16();
    maxCapacity = reader.ReadU32();
    mapName = reader.ReadString();
    supportedGameMode = reader.ReadString();
    return true;
}

// MsgServerRegisterResponse
void MsgServerRegisterResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(success ? 1 : 0);
    writer.WriteU64(serverId);
    writer.WriteString(errorMessage);
}
bool MsgServerRegisterResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    success = (reader.ReadU8() != 0);
    serverId = reader.ReadU64();
    errorMessage = reader.ReadString();
    return true;
}

// MsgServerHeartbeat
void MsgServerHeartbeat::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(serverId);
    writer.WriteU32(currentPlayers);
    writer.WriteU8(serverState);
}
bool MsgServerHeartbeat::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(13)) return false;
    serverId = reader.ReadU64();
    currentPlayers = reader.ReadU32();
    serverState = reader.ReadU8();
    return true;
}

// MsgMatchReserveRequest
void MsgMatchReserveRequest::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteString(matchId);
    writer.WriteString(gameMode);
    writer.WriteString(mapName);
    writer.WriteU32(static_cast<uint32_t>(participants.size()));
    for (const auto& p : participants) {
        writer.WriteU64(p.sessionId);
        writer.WriteString(p.displayName);
        writer.WriteString(p.matchAuthToken);
        writer.WriteU32(p.teamId);
        writer.WriteString(p.persistentUserId);
    }
}
bool MsgMatchReserveRequest::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(8)) return false;
    matchId = reader.ReadString();
    gameMode = reader.ReadString();
    mapName = reader.ReadString();
    uint32_t count = reader.ReadU32();
    participants.clear();
    participants.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        ParticipantReservation p;
        p.sessionId = reader.ReadU64();
        p.displayName = reader.ReadString();
        p.matchAuthToken = reader.ReadString();
        p.teamId = reader.ReadU32();
        p.persistentUserId = reader.ReadString();
        participants.push_back(p);
    }
    return true;
}

// MsgMatchReserveResponse
void MsgMatchReserveResponse::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU8(accepted ? 1 : 0);
    writer.WriteString(matchId);
    writer.WriteString(errorMessage);
}
bool MsgMatchReserveResponse::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(1)) return false;
    accepted = (reader.ReadU8() != 0);
    matchId = reader.ReadString();
    errorMessage = reader.ReadString();
    return true;
}

// MsgMatchRedirectNotification
void MsgMatchRedirectNotification::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteString(matchId);
    writer.WriteString(serverHost);
    writer.WriteU16(serverPort);
    writer.WriteString(playerAuthToken);
    writer.WriteString(gameMode);
    writer.WriteString(mapName);
}
bool MsgMatchRedirectNotification::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(6)) return false;
    matchId = reader.ReadString();
    serverHost = reader.ReadString();
    serverPort = reader.ReadU16();
    playerAuthToken = reader.ReadString();
    gameMode = reader.ReadString();
    mapName = reader.ReadString();
    return true;
}

// MsgMatchCompletedNotification
void MsgMatchCompletedNotification::Serialize(network::ByteStreamWriter& writer) const {
    writer.WriteU64(serverId);
    writer.WriteString(matchId);
    writer.WriteString(winnerName);
    writer.WriteU32(totalDurationSeconds);
}
bool MsgMatchCompletedNotification::Deserialize(network::ByteStreamReader& reader) {
    if (!reader.HasRemaining(14)) return false;
    serverId = reader.ReadU64();
    matchId = reader.ReadString();
    winnerName = reader.ReadString();
    totalDurationSeconds = reader.ReadU32();
    return true;
}

} // namespace wildbound::coordinator
