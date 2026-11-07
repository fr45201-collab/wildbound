#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "coordinator/MatchmakingQueue.hpp"
#include "coordinator/MatchmakingManager.hpp"

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
              << "TEST SUITE: Matchmaking Queues Subsystem (Phase 41)\n"
              << "========================================================" << std::endl;

    MatchmakingManager mgr;
    mgr.SetMatchSizeRequirement("solo", 2);
    mgr.SetMatchSizeRequirement("duos", 4);    // 2 duos = 4 players
    mgr.SetMatchSizeRequirement("squads", 8);  // 2 squads = 8 players

    // 1. Enqueue & Dequeue
    std::cout << "[TEST] 1. Enqueue & Dequeue Basics..." << std::endl;
    QueuedParty p1{
        .partyId = 201,
        .gameMode = "solo",
        .memberCount = 1,
        .queuedTime = 10.0f,
        .members = { 101 }
    };
    TEST_ASSERT(mgr.EnqueueParty(p1), "Enqueueing party 201 must succeed");
    TEST_ASSERT(mgr.IsPartyQueued(201), "Party 201 must be queued");
    TEST_ASSERT(mgr.GetQueueSize("solo") == 1, "Solo queue size must be 1");

    // Duplicate enqueue rejected
    TEST_ASSERT(!mgr.EnqueueParty(p1), "Duplicate enqueue must fail");

    // Dequeue
    TEST_ASSERT(mgr.DequeueParty(201), "Dequeueing party 201 must succeed");
    TEST_ASSERT(!mgr.IsPartyQueued(201), "Party 201 must no longer be queued");
    TEST_ASSERT(mgr.GetQueueSize("solo") == 0, "Solo queue size must be 0");

    // 2. Solo Match Formation
    std::cout << "[TEST] 2. Solo Match Formation (Threshold = 2)..." << std::endl;
    QueuedParty s1{ .partyId = 201, .gameMode = "solo", .memberCount = 1, .queuedTime = 12.0f, .members = { 101 } };
    QueuedParty s2{ .partyId = 202, .gameMode = "solo", .memberCount = 1, .queuedTime = 13.0f, .members = { 102 } };

    mgr.EnqueueParty(s1);
    std::vector<FormedMatch> matches;
    mgr.Update(14.0f, matches);
    TEST_ASSERT(matches.empty(), "1 player must not trigger match formation");

    mgr.EnqueueParty(s2);
    mgr.Update(15.0f, matches);
    TEST_ASSERT(matches.size() == 1, "2 players must trigger 1 match formation");
    TEST_ASSERT(matches[0].gameMode == "solo", "Match mode must be solo");
    TEST_ASSERT(matches[0].totalParticipants == 2, "Total participants must be 2");
    TEST_ASSERT(matches[0].parties.size() == 2, "Must contain 2 parties");
    TEST_ASSERT(mgr.GetQueueSize("solo") == 0, "Solo queue must now be empty");

    // 3. Duos Match Formation with Party Preservation
    std::cout << "[TEST] 3. Duos Match Formation & Party Preservation..." << std::endl;
    // Enqueue Duo A (2 players) and Duo B (2 players)
    QueuedParty d1{ .partyId = 301, .gameMode = "duos", .memberCount = 2, .queuedTime = 20.0f, .members = { 111, 112 } };
    QueuedParty d2{ .partyId = 302, .gameMode = "duos", .memberCount = 2, .queuedTime = 21.0f, .members = { 113, 114 } };

    mgr.EnqueueParty(d1);
    matches.clear();
    mgr.Update(22.0f, matches);
    TEST_ASSERT(matches.empty(), "1 duo (2 players) must not satisfy 4-player threshold");

    mgr.EnqueueParty(d2);
    mgr.Update(23.0f, matches);
    TEST_ASSERT(matches.size() == 1, "2 duos must satisfy 4-player threshold");
    TEST_ASSERT(matches[0].gameMode == "duos", "Match mode must be duos");
    TEST_ASSERT(matches[0].totalParticipants == 4, "Total participants must be 4");
    // Verify party preservation: Duo 1 members (111, 112) stayed together in party 301
    TEST_ASSERT(matches[0].parties[0].partyId == 301, "First party must be 301");
    TEST_ASSERT(matches[0].parties[0].members.size() == 2, "Duo 1 must have both members intact");
    TEST_ASSERT(matches[0].parties[1].partyId == 302, "Second party must be 302");
    TEST_ASSERT(matches[0].parties[1].members.size() == 2, "Duo 2 must have both members intact");

    // 4. Squads Match Formation (Parties Never Split)
    std::cout << "[TEST] 4. Squads Match Formation (8 Players, Whole Squads Kept Intact)..." << std::endl;
    QueuedParty sq1{ .partyId = 401, .gameMode = "squads", .memberCount = 4, .queuedTime = 30.0f, .members = { 1, 2, 3, 4 } };
    QueuedParty sq2{ .partyId = 402, .gameMode = "squads", .memberCount = 4, .queuedTime = 31.0f, .members = { 5, 6, 7, 8 } };

    mgr.EnqueueParty(sq1);
    matches.clear();
    mgr.Update(32.0f, matches);
    TEST_ASSERT(matches.empty(), "1 squad (4 players) must not trigger 8-player match");

    mgr.EnqueueParty(sq2);
    mgr.Update(33.0f, matches);
    TEST_ASSERT(matches.size() == 1, "2 squads must form 1 match");
    TEST_ASSERT(matches[0].gameMode == "squads", "Match mode must be squads");
    TEST_ASSERT(matches[0].totalParticipants == 8, "Total participants must be 8");
    TEST_ASSERT(matches[0].parties.size() == 2, "Match must contain exactly 2 intact squads");
    TEST_ASSERT(matches[0].parties[0].members.size() == 4, "Squad 1 preserved intact");
    TEST_ASSERT(matches[0].parties[1].members.size() == 4, "Squad 2 preserved intact");

    std::cout << "[SUCCESS] Matchmaking Queues Subsystem fully verified!" << std::endl;
    return 0;
}

