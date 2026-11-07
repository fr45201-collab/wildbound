#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "coordinator/Party.hpp"
#include "coordinator/PartyManager.hpp"
#include "coordinator/Lobby.hpp"
#include "coordinator/LobbyManager.hpp"

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
              << "TEST SUITE: Parties & Lobbies Subsystem (Phase 41)\n"
              << "========================================================" << std::endl;

    // 1. Party Creation & Leader Assignment
    std::cout << "[TEST] 1. Party Creation & Leader Assignment..." << std::endl;
    PartyManager pMgr;
    PartyId p1 = pMgr.CreateParty(101, "Leader_Ace", "duos");
    TEST_ASSERT(p1 != INVALID_PARTY_ID, "Party creation must succeed");

    auto* party = pMgr.GetParty(p1);
    TEST_ASSERT(party != nullptr, "Party lookup must succeed");
    TEST_ASSERT(party->leaderSessionId == 101, "Leader must be 101");
    TEST_ASSERT(party->gameMode == "duos", "Game mode must be duos");
    TEST_ASSERT(party->maxCapacity == 2, "Duos capacity must be 2");
    TEST_ASSERT(party->GetMemberCount() == 1, "Member count must be 1");
    TEST_ASSERT(party->IsLeader(101), "IsLeader must be true for leader");

    // 2. Member Capacity & Joining
    std::cout << "[TEST] 2. Capacity Enforcement & Member Joining..." << std::endl;
    TEST_ASSERT(pMgr.AddMember(p1, 102, "Member_Bob"), "Adding member 102 must succeed");
    TEST_ASSERT(party->GetMemberCount() == 2, "Member count must be 2");
    TEST_ASSERT(party->IsFull(), "Duos party must be full with 2 members");

    // Attempting to exceed capacity
    TEST_ASSERT(!pMgr.AddMember(p1, 103, "Member_Charlie"), "Adding 3rd member to Duos party must be rejected");

    // 3. Readiness Checks
    std::cout << "[TEST] 3. Member Readiness Checks..." << std::endl;
    // Leader is ready by default, member 102 is initially false
    TEST_ASSERT(!party->AreAllMembersReady(), "Party must not be ready until all members are ready");
    TEST_ASSERT(pMgr.SetMemberReady(p1, 102, true), "Setting member 102 ready must succeed");
    TEST_ASSERT(party->AreAllMembersReady(), "Party must be ready when all members are ready");

    // 4. Game Mode Switching & Capacity Expansion
    std::cout << "[TEST] 4. Game Mode Switching to Squads..." << std::endl;
    TEST_ASSERT(pMgr.SetPartyGameMode(p1, 101, "squads"), "Leader switching mode to Squads must succeed");
    TEST_ASSERT(party->maxCapacity == 4, "Squads capacity must be 4");
    TEST_ASSERT(!party->IsFull(), "Party must no longer be full");
    TEST_ASSERT(pMgr.AddMember(p1, 103, "Member_Charlie"), "Adding 3rd member must now succeed");
    TEST_ASSERT(pMgr.AddMember(p1, 104, "Member_Delta"), "Adding 4th member must succeed");
    TEST_ASSERT(party->IsFull(), "Squads party must now be full with 4 members");

    // Non-leader attempting mode change
    TEST_ASSERT(!pMgr.SetPartyGameMode(p1, 102, "solo"), "Non-leader switching mode must be rejected");

    // 5. Leader Transfer & Kicking
    std::cout << "[TEST] 5. Leader Transfer & Member Kicking..." << std::endl;
    TEST_ASSERT(pMgr.TransferLeader(p1, 101, 102), "Leader transfer to 102 must succeed");
    TEST_ASSERT(party->leaderSessionId == 102, "Leader must now be 102");

    // New leader kicks member 104
    TEST_ASSERT(pMgr.KickMember(p1, 102, 104), "Leader 102 kicking member 104 must succeed");
    TEST_ASSERT(party->GetMemberCount() == 3, "Party size must decrease to 3");
    TEST_ASSERT(!party->HasMember(104), "Member 104 must no longer be in party");

    // 6. Automatic Leader Reassignment upon Departure & Disbanding
    std::cout << "[TEST] 6. Auto Leader Reassignment & Disbanding..." << std::endl;
    bool disbanded = false;
    SessionId newLeader = INVALID_SESSION_ID;
    TEST_ASSERT(pMgr.RemoveMember(p1, 102, disbanded, newLeader), "Leader 102 leaving party must succeed");
    TEST_ASSERT(!disbanded, "Party must not disband while members remain");
    TEST_ASSERT(newLeader == 101, "Leadership must transfer to next senior member (101)");

    // Remaining members leave
    TEST_ASSERT(pMgr.RemoveMember(p1, 103, disbanded, newLeader), "Member 103 leaving must succeed");
    TEST_ASSERT(pMgr.RemoveMember(p1, 101, disbanded, newLeader), "Last member 101 leaving must succeed");
    TEST_ASSERT(disbanded, "Party must be disbanded when last member departs");
    TEST_ASSERT(pMgr.GetParty(p1) == nullptr, "Party must no longer exist in manager");

    // 7. Custom Lobbies
    std::cout << "[TEST] 7. Custom Lobby Management..." << std::endl;
    LobbyManager lMgr;
    LobbyId lob1 = lMgr.CreateLobby(201, "Host_Dave", "Custom Scrims", 16, true, "secret123");
    TEST_ASSERT(lob1 != INVALID_LOBBY_ID, "Lobby creation must succeed");

    auto* lobby = lMgr.GetLobby(lob1);
    TEST_ASSERT(lobby != nullptr, "Lobby lookup must succeed");
    TEST_ASSERT(lobby->isPrivate, "Lobby must be private");

    // Private join passcode validation
    TEST_ASSERT(!lMgr.AddMember(lob1, 202, "Guest_Eve", "wrongpass"), "Join with wrong passcode must fail");
    TEST_ASSERT(lMgr.AddMember(lob1, 202, "Guest_Eve", "secret123"), "Join with correct passcode must succeed");
    TEST_ASSERT(lobby->GetMemberCount() == 2, "Lobby member count must be 2");

    std::cout << "[SUCCESS] Parties & Lobbies Subsystem fully verified!" << std::endl;
    return 0;
}

