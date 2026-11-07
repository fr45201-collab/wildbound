#include "InvitationManager.hpp"

namespace wildbound::coordinator {

InvitationId InvitationManager::GenerateInvitationId() noexcept {
    return m_nextInvitationId++;
}

InvitationId InvitationManager::SendInvitation(SessionId senderId, const std::string& senderName,
                                            SessionId recipientId, InvitationType type,
                                            uint64_t targetId, float currentTime,
                                            float expirationSeconds) {
    if (senderId == INVALID_SESSION_ID || recipientId == INVALID_SESSION_ID) return INVALID_INVITATION_ID;
    if (senderId == recipientId) return INVALID_INVITATION_ID;

    // Check if duplicate pending invite exists
    for (const auto& [id, inv] : m_invitations) {
        if (inv.senderSessionId == senderId &&
            inv.recipientSessionId == recipientId &&
            inv.type == type &&
            inv.targetId == targetId &&
            inv.IsActionable(currentTime)) {
            return inv.invitationId; // Return existing active invitation
        }
    }

    InvitationId newId = GenerateInvitationId();
    PlayerInvitation inv{
        .invitationId = newId,
        .type = type,
        .senderSessionId = senderId,
        .senderDisplayName = senderName,
        .recipientSessionId = recipientId,
        .targetId = targetId,
        .createdAt = currentTime,
        .expirationSeconds = expirationSeconds,
        .state = InvitationState::Pending
    };

    m_invitations[newId] = std::move(inv);
    return newId;
}

PlayerInvitation* InvitationManager::GetInvitation(InvitationId id) noexcept {
    auto it = m_invitations.find(id);
    return (it != m_invitations.end()) ? &it->second : nullptr;
}

const PlayerInvitation* InvitationManager::GetInvitation(InvitationId id) const noexcept {
    auto it = m_invitations.find(id);
    return (it != m_invitations.end()) ? &it->second : nullptr;
}

bool InvitationManager::AcceptInvitation(InvitationId id, SessionId recipientId, float currentTime) {
    auto* inv = GetInvitation(id);
    if (!inv) return false;
    if (inv->recipientSessionId != recipientId) return false;
    if (!inv->IsActionable(currentTime)) {
        if (inv->IsExpired(currentTime)) {
            inv->state = InvitationState::Expired;
        }
        return false;
    }

    inv->state = InvitationState::Accepted;
    return true;
}

bool InvitationManager::DeclineInvitation(InvitationId id, SessionId recipientId, float currentTime) {
    auto* inv = GetInvitation(id);
    if (!inv) return false;
    if (inv->recipientSessionId != recipientId) return false;
    if (!inv->IsActionable(currentTime)) {
        if (inv->IsExpired(currentTime)) {
            inv->state = InvitationState::Expired;
        }
        return false;
    }

    inv->state = InvitationState::Declined;
    return true;
}

bool InvitationManager::RevokeInvitation(InvitationId id, SessionId senderId) {
    auto* inv = GetInvitation(id);
    if (!inv) return false;
    if (inv->senderSessionId != senderId) return false;
    if (inv->state != InvitationState::Pending) return false;

    inv->state = InvitationState::Revoked;
    return true;
}

void InvitationManager::CheckExpirations(float currentTime, std::vector<InvitationId>& outExpired) {
    outExpired.clear();
    for (auto& [id, inv] : m_invitations) {
        if (inv.state == InvitationState::Pending && inv.IsExpired(currentTime)) {
            inv.state = InvitationState::Expired;
            outExpired.push_back(id);
        }
    }
}

std::vector<const PlayerInvitation*> InvitationManager::GetPendingForRecipient(SessionId recipientId, float currentTime) const {
    std::vector<const PlayerInvitation*> result;
    for (const auto& [id, inv] : m_invitations) {
        if (inv.recipientSessionId == recipientId && inv.IsActionable(currentTime)) {
            result.push_back(&inv);
        }
    }
    return result;
}

void InvitationManager::Clear() {
    m_invitations.clear();
}

} // namespace wildbound::coordinator

