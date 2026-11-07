#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "coordinator/PlayerInvitation.hpp"
#include "coordinator/InvitationManager.hpp"

using namespace wildbound::coordinator;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILURE] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            std::abort(); \
        } \
    } while (0)

int main() {
    std::cout << "========================================================\n"
              << "TEST SUITE: Player Invitations Subsystem (Phase 41)\n"
              << "========================================================" << std::endl;

    InvitationManager mgr;

    // 1. Sending an Invitation
    std::cout << "[TEST] 1. Sending Player Invitations..." << std::endl;
    InvitationId inv1 = mgr.SendInvitation(1001, "Inviter_Alpha", 1002, InvitationType::Party, 20001, 10.0f, 60.0f);
    TEST_ASSERT(inv1 != INVALID_INVITATION_ID, "Sending invitation must succeed");

    auto* pInv1 = mgr.GetInvitation(inv1);
    TEST_ASSERT(pInv1 != nullptr, "Invitation lookup must succeed");
    TEST_ASSERT(pInv1->state == InvitationState::Pending, "Initial state must be Pending");
    TEST_ASSERT(pInv1->senderSessionId == 1001, "Sender ID must match");
    TEST_ASSERT(pInv1->recipientSessionId == 1002, "Recipient ID must match");
    TEST_ASSERT(pInv1->IsActionable(15.0f), "Invitation must be actionable within expiration window");

    // Duplicate invite check (returns existing invitation ID)
    InvitationId dupInv = mgr.SendInvitation(1001, "Inviter_Alpha", 1002, InvitationType::Party, 20001, 12.0f, 60.0f);
    TEST_ASSERT(dupInv == inv1, "Duplicate active invitation must return existing ID");

    // Query pending invites for recipient
    auto pending = mgr.GetPendingForRecipient(1002, 15.0f);
    TEST_ASSERT(pending.size() == 1, "Must have exactly 1 pending invitation");

    // 2. Acceptance & Single-Use State Transition
    std::cout << "[TEST] 2. Invitation Acceptance & State Transition..." << std::endl;
    TEST_ASSERT(mgr.AcceptInvitation(inv1, 1002, 20.0f), "Accepting invitation must succeed");
    TEST_ASSERT(pInv1->state == InvitationState::Accepted, "State must transition to Accepted");
    TEST_ASSERT(!pInv1->IsActionable(21.0f), "Accepted invitation must no longer be actionable");

    // Cannot re-accept or decline once accepted (single-use invariant)
    TEST_ASSERT(!mgr.AcceptInvitation(inv1, 1002, 22.0f), "Re-accepting must be rejected");
    TEST_ASSERT(!mgr.DeclineInvitation(inv1, 1002, 22.0f), "Declining accepted invite must be rejected");

    // 3. Decline State Transition
    std::cout << "[TEST] 3. Invitation Decline Transition..." << std::endl;
    InvitationId inv2 = mgr.SendInvitation(1001, "Inviter_Alpha", 1003, InvitationType::Party, 20001, 25.0f, 60.0f);
    TEST_ASSERT(inv2 != INVALID_INVITATION_ID, "Invitation 2 must be created");
    TEST_ASSERT(mgr.DeclineInvitation(inv2, 1003, 30.0f), "Declining invitation must succeed");
    TEST_ASSERT(mgr.GetInvitation(inv2)->state == InvitationState::Declined, "State must be Declined");

    // 4. Revocation by Sender
    std::cout << "[TEST] 4. Sender Revocation..." << std::endl;
    InvitationId inv3 = mgr.SendInvitation(1001, "Inviter_Alpha", 1004, InvitationType::Party, 20001, 35.0f, 60.0f);
    // Non-sender cannot revoke
    TEST_ASSERT(!mgr.RevokeInvitation(inv3, 9999), "Unauthorized revocation must fail");
    // Sender revokes
    TEST_ASSERT(mgr.RevokeInvitation(inv3, 1001), "Sender revocation must succeed");
    TEST_ASSERT(mgr.GetInvitation(inv3)->state == InvitationState::Revoked, "State must be Revoked");

    // 5. 60-Second Expiration Lifecycle
    std::cout << "[TEST] 5. 60-Second Expiration Processing..." << std::endl;
    InvitationId inv4 = mgr.SendInvitation(1001, "Inviter_Alpha", 1005, InvitationType::Party, 20001, 100.0f, 60.0f);
    // At t = 150s (elapsed 50s): still active
    TEST_ASSERT(mgr.GetInvitation(inv4)->IsActionable(150.0f), "Invitation must still be actionable at 50s");

    // At t = 165s (elapsed 65s): expired
    std::vector<InvitationId> expiredList;
    mgr.CheckExpirations(165.0f, expiredList);
    TEST_ASSERT(expiredList.size() == 1, "Exactly 1 invitation must expire");
    TEST_ASSERT(expiredList[0] == inv4, "Expired invitation must be inv4");
    TEST_ASSERT(mgr.GetInvitation(inv4)->state == InvitationState::Expired, "State must be Expired");
    TEST_ASSERT(!mgr.GetInvitation(inv4)->IsActionable(165.0f), "Expired invitation must not be actionable");

    std::cout << "[SUCCESS] Player Invitations Subsystem fully verified!" << std::endl;
    return 0;
}

