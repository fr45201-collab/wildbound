#pragma once

#include "PlayerInvitation.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

class InvitationManager {
public:
    InvitationManager() = default;
    ~InvitationManager() = default;

    InvitationId SendInvitation(SessionId senderId, const std::string& senderName,
                                SessionId recipientId, InvitationType type,
                                uint64_t targetId, float currentTime,
                                float expirationSeconds = 60.0f);

    [[nodiscard]] PlayerInvitation* GetInvitation(InvitationId id) noexcept;
    [[nodiscard]] const PlayerInvitation* GetInvitation(InvitationId id) const noexcept;

    bool AcceptInvitation(InvitationId id, SessionId recipientId, float currentTime);
    bool DeclineInvitation(InvitationId id, SessionId recipientId, float currentTime);
    bool RevokeInvitation(InvitationId id, SessionId senderId);

    void CheckExpirations(float currentTime, std::vector<InvitationId>& outExpired);

    [[nodiscard]] std::vector<const PlayerInvitation*> GetPendingForRecipient(SessionId recipientId, float currentTime) const;
    [[nodiscard]] size_t GetInvitationCount() const noexcept { return m_invitations.size(); }

    void Clear();

private:
    InvitationId GenerateInvitationId() noexcept;

    std::unordered_map<InvitationId, PlayerInvitation> m_invitations;
    InvitationId m_nextInvitationId{ 40001 };
};

} // namespace wildbound::coordinator

