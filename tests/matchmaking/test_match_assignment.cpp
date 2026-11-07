#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "coordinator/MatchAssignment.hpp"
#include "coordinator/MatchAssignmentManager.hpp"
#include "coordinator/DedicatedServerRegistry.hpp"
#include "coordinator/SessionManager.hpp"

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
              << "TEST SUITE: Match Assignment Subsystem (Phase 41)\n"
              << "========================================================" << std::endl;

    MatchAssignmentManager assignMgr;
    SessionManager sessMgr;

    SessionId s1 = sessMgr.CreateSession(1, "Player_One", 10.0f);
    SessionId s2 = sessMgr.CreateSession(2, "Player_Two", 10.0f);
    SessionId s3 = sessMgr.CreateSession(3, "Player_Three", 10.0f);
    SessionId s4 = sessMgr.CreateSession(4, "Player_Four", 10.0f);

    ServerInstance server{
        .serverId = 7001,
        .connectionId = 99,
        .serverName = "Dedicated_Server_01",
        .hostAddress = "127.0.0.1",
        .port = 7777,
        .maxCapacity = 32,
        .supportedGameMode = "duos",
        .mapName = "maps/world/battle_royale_island.json",
        .state = ServerRegistryState::Available
    };

    FormedMatch match{
        .matchId = "match_1001",
        .gameMode = "duos",
        .parties = {
            QueuedParty{ .partyId = 201, .gameMode = "duos", .memberCount = 2, .queuedTime = 10.0f, .members = { s1, s2 } },
            QueuedParty{ .partyId = 202, .gameMode = "duos", .memberCount = 2, .queuedTime = 10.0f, .members = { s3, s4 } }
        },
        .totalParticipants = 4
    };

    // 1. Assignment Creation & Token Generation
    std::cout << "[TEST] 1. Match Assignment Creation & Participant Tokens..." << std::endl;
    auto* assignment = assignMgr.CreateAssignment(match, server, sessMgr, 15.0f, 5.0f);
    TEST_ASSERT(assignment != nullptr, "Assignment creation must succeed");
    TEST_ASSERT(assignment->matchId == "match_1001", "Match ID must match");
    TEST_ASSERT(assignment->assignedServerId == 7001, "Assigned server must match");
    TEST_ASSERT(assignment->serverPort == 7777, "Port must be 7777");
    TEST_ASSERT(assignment->state == AssignmentState::PendingReservation, "Initial state must be PendingReservation");
    TEST_ASSERT(assignment->participants.size() == 4, "Must have 4 participants");

    // Check participant auth tokens and team assignments
    auto* p1 = assignment->FindParticipant(s1);
    auto* p2 = assignment->FindParticipant(s2);
    auto* p3 = assignment->FindParticipant(s3);
    auto* p4 = assignment->FindParticipant(s4);
    TEST_ASSERT(p1 != nullptr && !p1->matchAuthToken.empty(), "Participant 1 must have valid token");
    TEST_ASSERT(p2 != nullptr && !p2->matchAuthToken.empty(), "Participant 2 must have valid token");
    TEST_ASSERT(p1->teamId == p2->teamId, "Party 1 members must have same team ID");
    TEST_ASSERT(p3->teamId == p4->teamId, "Party 2 members must have same team ID");
    TEST_ASSERT(p1->teamId != p3->teamId, "Different parties must have different team IDs");

    // Lookup by session
    auto* foundBySess = assignMgr.GetAssignmentBySession(s3);
    TEST_ASSERT(foundBySess != nullptr && foundBySess->matchId == "match_1001", "Lookup by session must resolve assignment");

    // 2. Reservation Confirmation Handshake
    std::cout << "[TEST] 2. Reservation Confirmation Handshake..." << std::endl;
    TEST_ASSERT(assignMgr.ConfirmReservation("match_1001"), "Confirmation must succeed");
    TEST_ASSERT(assignment->state == AssignmentState::Confirmed, "State must transition to Confirmed");

    // 3. Match Completion Handshake
    std::cout << "[TEST] 3. Match Completion Handling..." << std::endl;
    TEST_ASSERT(assignMgr.CompleteMatch("match_1001"), "CompleteMatch must succeed");
    TEST_ASSERT(assignment->state == AssignmentState::Completed, "State must transition to Completed");
    TEST_ASSERT(assignMgr.GetAssignmentBySession(s1) == nullptr, "Completed match must clear session references");

    // 4. Reservation Timeout & Rollback (5s threshold)
    std::cout << "[TEST] 4. Reservation Timeout & Rollback..." << std::endl;
    FormedMatch match2{
        .matchId = "match_1002",
        .gameMode = "solo",
        .parties = {
            QueuedParty{ .partyId = 301, .gameMode = "solo", .memberCount = 1, .queuedTime = 50.0f, .members = { s1 } },
            QueuedParty{ .partyId = 302, .gameMode = "solo", .memberCount = 1, .queuedTime = 50.0f, .members = { s2 } }
        },
        .totalParticipants = 2
    };

    auto* a2 = assignMgr.CreateAssignment(match2, server, sessMgr, 50.0f, 5.0f);
    TEST_ASSERT(a2 != nullptr, "Assignment 2 creation must succeed");

    // At t = 52.0s (elapsed 2s): not timed out
    std::vector<std::string> timedOutMatches;
    assignMgr.CheckReservationTimeouts(52.0f, timedOutMatches);
    TEST_ASSERT(timedOutMatches.empty(), "Must not time out at 2s");
    TEST_ASSERT(a2->state == AssignmentState::PendingReservation, "State remains PendingReservation");

    // At t = 56.0s (elapsed 6s): timed out (> 5s)
    assignMgr.CheckReservationTimeouts(56.0f, timedOutMatches);
    TEST_ASSERT(timedOutMatches.size() == 1, "Exactly 1 match must time out");
    TEST_ASSERT(timedOutMatches[0] == "match_1002", "Timed out match must be match_1002");
    TEST_ASSERT(a2->state == AssignmentState::Failed, "Assignment state must transition to Failed");
    TEST_ASSERT(assignMgr.GetAssignmentBySession(s1) == nullptr, "Timed out match must clear session references");

    std::cout << "[SUCCESS] Match Assignment Subsystem fully verified!" << std::endl;
    return 0;
}

