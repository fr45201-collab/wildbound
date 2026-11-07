#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "coordinator/PlayerSession.hpp"
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
              << "TEST SUITE: Player Sessions Subsystem (Phase 41)\n"
              << "========================================================" << std::endl;

    SessionManager mgr;
    TEST_ASSERT(mgr.GetActiveSessionCount() == 0, "Initial active sessions must be 0");

    // 1. Session creation & ID uniqueness
    std::cout << "[TEST] 1. Session Creation & Unique IDs..." << std::endl;
    SessionId s1 = mgr.CreateSession(1, "Player_Alpha", 10.0f);
    SessionId s2 = mgr.CreateSession(2, "Player_Bravo", 10.0f);
    TEST_ASSERT(s1 != INVALID_SESSION_ID, "Session 1 must be valid");
    TEST_ASSERT(s2 != INVALID_SESSION_ID, "Session 2 must be valid");
    TEST_ASSERT(s1 != s2, "Session IDs must be distinct");
    TEST_ASSERT(mgr.GetActiveSessionCount() == 2, "Active count must be 2");

    // 2. Lookup by ID and connection ID
    std::cout << "[TEST] 2. Session Lookup by ID & Connection..." << std::endl;
    auto* p1 = mgr.GetSession(s1);
    TEST_ASSERT(p1 != nullptr, "Session 1 lookup must succeed");
    TEST_ASSERT(p1->displayName == "Player_Alpha", "Display name must match");
    TEST_ASSERT(p1->connectionId == 1, "Connection ID must match");
    TEST_ASSERT(p1->state == SessionState::Connected, "Initial state must be Connected");

    auto* pConn2 = mgr.GetSessionByConnection(2);
    TEST_ASSERT(pConn2 != nullptr, "Lookup by connection 2 must succeed");
    TEST_ASSERT(pConn2->sessionId == s2, "Must resolve to session s2");

    // 3. Heartbeat updating
    std::cout << "[TEST] 3. Heartbeat Tracking..." << std::endl;
    TEST_ASSERT(mgr.UpdateHeartbeat(s1, 25.0f), "Heartbeat update must succeed");
    TEST_ASSERT(mgr.GetSession(s1)->lastHeartbeatTime == 25.0f, "Updated heartbeat time must be 25.0");

    // 4. Inactivity Timeout Detection
    std::cout << "[TEST] 4. Inactivity Timeout Detection & Expiration..." << std::endl;
    // Current time: 60.0f. s1 was updated at 25.0 (diff=35s). s2 was at 10.0 (diff=50s).
    // Timeout threshold: 30s. Both s1 and s2 should time out.
    std::vector<SessionId> timedOut;
    mgr.CheckTimeouts(60.0f, 30.0f, timedOut);
    TEST_ASSERT(timedOut.size() == 2, "Both sessions must time out");
    TEST_ASSERT(mgr.GetActiveSessionCount() == 0, "All timed-out sessions must be removed");

    // 5. Association with Party, Lobby, Match
    std::cout << "[TEST] 5. Association with Party, Lobby, Match..." << std::endl;
    SessionId s3 = mgr.CreateSession(3, "Player_Charlie", 100.0f);
    TEST_ASSERT(mgr.SetSessionParty(s3, 2001), "Setting party must succeed");
    TEST_ASSERT(mgr.GetSession(s3)->state == SessionState::InParty, "State must transition to InParty");
    TEST_ASSERT(mgr.GetSession(s3)->partyId == 2001, "PartyId must match");

    TEST_ASSERT(mgr.SetSessionMatch(s3, "match_alpha"), "Setting match must succeed");
    TEST_ASSERT(mgr.GetSession(s3)->state == SessionState::InMatch, "State must transition to InMatch");
    TEST_ASSERT(mgr.GetSession(s3)->currentMatchId == "match_alpha", "MatchId must match");

    std::cout << "[SUCCESS] Player Sessions Subsystem fully verified!" << std::endl;
    return 0;
}

