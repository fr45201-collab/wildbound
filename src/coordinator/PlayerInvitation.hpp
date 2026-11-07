#pragma once

#include "CoordinatorProtocol.hpp"
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

enum class InvitationState : uint8_t {
    Pending,
    Accepted,
    Declined,
    Expired,
    Revoked
};

struct PlayerInvitation {
    InvitationId invitationId{ INVALID_INVITATION_ID };
    InvitationType type{ InvitationType::Party };
    SessionId senderSessionId{ INVALID_SESSION_ID };
    std::string senderDisplayName;
    SessionId recipientSessionId{ INVALID_SESSION_ID };
    uint64_t targetId{ 0 }; // PartyId or LobbyId
    float createdAt{ 0.0f };
    float expirationSeconds{ 60.0f };
    InvitationState state{ InvitationState::Pending };

    [[nodiscard]] bool IsExpired(float currentTime) const noexcept {
        return (currentTime - createdAt) >= expirationSeconds;
    }

    [[nodiscard]] bool IsActionable(float currentTime) const noexcept {
        return state == InvitationState::Pending && !IsExpired(currentTime);
    }
};

} // namespace wildbound::coordinator

