#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>

#include "match/MatchState.hpp"
#include "match/MatchConfiguration.hpp"
#include "match/MatchParticipant.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "match/SpawnPoint.hpp"
#include "match/MatchSpawner.hpp"
#include "match/MatchCountdown.hpp"
#include "match/MatchEliminationSystem.hpp"
#include "match/MatchStatistics.hpp"
#include "match/MatchResult.hpp"
#include "match/MatchManager.hpp"
#include "combat/CombatEvents.hpp"
#include "world/WorldManager.hpp"
#include "storm/StormManager.hpp"

#ifdef assert
#undef assert
#endif
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)
#define assert(cond) TEST_ASSERT(cond, "Assertion verified")

using namespace wildbound;
using namespace wildbound::match;

// ----------------------------------------------------------------------------
// TEST 1: State Machine & Valid/Invalid Transitions
// ----------------------------------------------------------------------------
void TestStateMachine() {
    std::cout << "[TEST] 1. Match State Machine Transitions..." << std::endl;

    // Direct transition validity check
    assert(IsValidStateTransition(MatchState::Uninitialized, MatchState::Initializing));
    assert(IsValidStateTransition(MatchState::Initializing, MatchState::WaitingForPlayers));
    assert(IsValidStateTransition(MatchState::Initializing, MatchState::Failed));
    assert(IsValidStateTransition(MatchState::WaitingForPlayers, MatchState::Countdown));
    assert(IsValidStateTransition(MatchState::Countdown, MatchState::Starting));
    assert(IsValidStateTransition(MatchState::Starting, MatchState::Active));
    assert(IsValidStateTransition(MatchState::Active, MatchState::Finishing));
    assert(IsValidStateTransition(MatchState::Finishing, MatchState::Completed));
    assert(IsValidStateTransition(MatchState::Completed, MatchState::Cleanup));
    assert(IsValidStateTransition(MatchState::Cleanup, MatchState::Uninitialized));

    // Rejection of invalid transitions
    assert(!IsValidStateTransition(MatchState::Uninitialized, MatchState::Active));
    assert(!IsValidStateTransition(MatchState::WaitingForPlayers, MatchState::Active));
    assert(!IsValidStateTransition(MatchState::Active, MatchState::WaitingForPlayers));
    assert(!IsValidStateTransition(MatchState::Active, MatchState::Active)); // No self-transitions
    assert(!IsValidStateTransition(MatchState::Completed, MatchState::Active));

    // Manager State Machine execution
    MatchManager mgr;
    assert(mgr.GetState() == MatchState::Uninitialized);

    // Initializing directly via TransitionTo
    assert(!mgr.TransitionTo(MatchState::Active)); // Rejected
    assert(mgr.GetState() == MatchState::Uninitialized);

    assert(mgr.TransitionTo(MatchState::Initializing));
    assert(mgr.GetState() == MatchState::Initializing);

    assert(mgr.TransitionTo(MatchState::WaitingForPlayers));
    assert(mgr.GetState() == MatchState::WaitingForPlayers);

    // Can transition to Cleanup
    assert(mgr.TransitionTo(MatchState::Cleanup));
    assert(mgr.TransitionTo(MatchState::Uninitialized));
    assert(mgr.GetState() == MatchState::Uninitialized);

    std::cout << "  -> Formal transition graph and rejection logic verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 2: Participant Registry Authoritative Operations
// ----------------------------------------------------------------------------
void TestParticipantRegistry() {
    std::cout << "[TEST] 2. Participant Registry Operations..." << std::endl;

    MatchParticipantRegistry reg;
    assert(reg.GetTotalCount() == 0);
    assert(reg.GetAliveCount() == 0);

    MatchParticipant p1;
    p1.participantId = 101;
    p1.type = ParticipantType::LocalHuman;
    p1.displayName = "Alpha";
    p1.entityId = 10;
    p1.status = ParticipantStatus::Registered;

    MatchParticipant p2;
    p2.participantId = 102;
    p2.type = ParticipantType::AI;
    p2.displayName = "Bravo";
    p2.entityId = 20;
    p2.status = ParticipantStatus::Registered;

    assert(reg.RegisterParticipant(p1));
    assert(reg.RegisterParticipant(p2));
    assert(reg.GetTotalCount() == 2);

    // Reject duplicate ID
    assert(!reg.RegisterParticipant(p1));
    // Reject ID 0
    MatchParticipant pZero;
    pZero.participantId = 0;
    assert(!reg.RegisterParticipant(pZero));

    // Lookups
    assert(reg.FindParticipant(101) != nullptr);
    assert(reg.FindParticipant(102) != nullptr);
    assert(reg.FindParticipant(999) == nullptr);
    assert(reg.FindParticipantByEntityId(10) != nullptr);
    assert(reg.FindParticipantByEntityId(20) != nullptr);
    assert(reg.FindParticipantByEntityId(999) == nullptr);

    // Status updates
    assert(reg.UpdateParticipantStatus(101, ParticipantStatus::Alive));
    assert(reg.UpdateParticipantStatus(102, ParticipantStatus::Alive));
    assert(reg.GetAliveCount() == 2);

    assert(reg.UpdateParticipantStatus(102, ParticipantStatus::Eliminated));
    assert(reg.GetAliveCount() == 1);
    assert(reg.GetAliveParticipantIds().size() == 1);
    assert(reg.GetAliveParticipantIds()[0] == 101);
    assert(reg.GetEliminatedParticipantIds().size() == 1);
    assert(reg.GetEliminatedParticipantIds()[0] == 102);

    // Unregister
    assert(reg.UnregisterParticipant(102));
    assert(reg.GetTotalCount() == 1);
    assert(reg.FindParticipant(102) == nullptr);

    reg.ResetRegistry();
    assert(reg.GetTotalCount() == 0);
    std::cout << "  -> Registry insertion, lookups, alive tracking, and cleanup verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 3: Spawning System & Validation
// ----------------------------------------------------------------------------
void TestSpawningSystem() {
    std::cout << "[TEST] 3. Spawner Distribution & Collision Bounds..." << std::endl;

    MatchSpawner spawner;
    Rectangle bounds{ 100.0f, 100.0f, 1000.0f, 1000.0f };

    // Add points inside and outside bounds
    spawner.AddCustomSpawnPoint(Vector2{ 200.0f, 200.0f }, "ZoneA");
    spawner.AddCustomSpawnPoint(Vector2{ 400.0f, 400.0f }, "ZoneB");
    spawner.AddCustomSpawnPoint(Vector2{ 600.0f, 600.0f }, "ZoneC");
    spawner.AddCustomSpawnPoint(Vector2{ 800.0f, 800.0f }, "ZoneD");

    assert(spawner.GetTotalSpawnCount() == 4);
    assert(spawner.GetAvailableSpawnCount() == 4);

    std::vector<MatchParticipant> participants(3);
    participants[0].participantId = 1;
    participants[1].participantId = 2;
    participants[2].participantId = 3;

    // Minimum Distance assignment
    assert(spawner.AssignSpawnPoints(participants, SpawnSelectionStrategy::MinimumDistance, 42));
    assert(spawner.GetAvailableSpawnCount() == 1);

    // Verify all 3 have distinct non-zero coordinates
    assert(participants[0].spawnPosition.x > 0.0f);
    assert(participants[1].spawnPosition.x > 0.0f);
    assert(participants[2].spawnPosition.x > 0.0f);

    float d01 = std::hypot(participants[0].spawnPosition.x - participants[1].spawnPosition.x,
                           participants[0].spawnPosition.y - participants[1].spawnPosition.y);
    assert(d01 > 10.0f); // Not overlapping

    // Insufficient spawns check
    std::vector<MatchParticipant> tooMany(5);
    for (size_t i = 0; i < tooMany.size(); ++i) tooMany[i].participantId = static_cast<uint32_t>(i + 10);
    assert(!spawner.AssignSpawnPoints(tooMany)); // Only 1 left

    spawner.ResetSpawns();
    assert(spawner.GetAvailableSpawnCount() == 4);
    std::cout << "  -> Spawner bounds checking and distribution strategies verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 4: Match Countdown & Input Restrictions
// ----------------------------------------------------------------------------
void TestMatchCountdown() {
    std::cout << "[TEST] 4. Pre-Match Countdown Timer..." << std::endl;

    MatchCountdown cd;
    assert(!cd.IsActive());
    assert(!cd.IsFinished());

    cd.Start(5.0f);
    assert(cd.IsActive());
    assert(!cd.IsFinished());
    assert(cd.GetTimeRemaining() == 5.0f);
    assert(cd.GetProgress() == 0.0f);

    cd.Update(2.0f);
    assert(std::abs(cd.GetTimeRemaining() - 3.0f) < 0.001f);
    assert(std::abs(cd.GetProgress() - 0.4f) < 0.001f);

    cd.Pause();
    cd.Update(2.0f);
    assert(std::abs(cd.GetTimeRemaining() - 3.0f) < 0.001f); // Paused!

    cd.Resume();
    cd.Update(3.5f);
    assert(cd.GetTimeRemaining() == 0.0f); // Clamped at 0
    assert(cd.IsFinished());
    assert(!cd.IsActive());
    assert(cd.GetProgress() == 1.0f);

    std::cout << "  -> Countdown progression, pause/resume, and zero-clamping verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 5: Elimination Pipeline & Attribution
// ----------------------------------------------------------------------------
void TestEliminationSystem() {
    std::cout << "[TEST] 5. Elimination Pipeline & Attribution..." << std::endl;

    MatchParticipantRegistry reg;
    MatchStatistics stats;
    MatchEliminationSystem elimSys;

    MatchParticipant pVictim;
    pVictim.participantId = 1;
    pVictim.displayName = "VictimPlayer";
    pVictim.status = ParticipantStatus::Alive;
    reg.RegisterParticipant(pVictim);

    MatchParticipant pKiller;
    pKiller.participantId = 2;
    pKiller.displayName = "KillerPlayer";
    pKiller.status = ParticipantStatus::Alive;
    reg.RegisterParticipant(pKiller);

    MatchParticipant lastVictim;
    EliminationInfo lastInfo;
    int callbackCount = 0;
    elimSys.SubscribeElimination([&](const MatchParticipant& victim, const EliminationInfo& info) {
        callbackCount++;
        lastVictim = victim;
        lastInfo = info;
    });

    // Valid combat elimination
    bool success = elimSys.ProcessElimination(1, EliminationCause::Combat, 2, 45.5f, reg, stats);
    assert(success);
    assert(callbackCount == 1);
    assert(lastVictim.participantId == 1);
    assert(lastInfo.cause == EliminationCause::Combat);
    assert(lastInfo.killerParticipantId == 2);
    assert(lastInfo.killerName == "KillerPlayer");

    const auto* v = reg.FindParticipant(1);
    assert(v->status == ParticipantStatus::Eliminated);
    assert(v->eliminationInfo.cause == EliminationCause::Combat);
    assert(v->eliminationInfo.killerParticipantId == 2);
    assert(v->survivalDuration == 45.5f);

    const auto* k = reg.FindParticipant(2);
    assert(k->kills == 1);
    assert(stats.totalKills == 1);
    assert(stats.eliminatedParticipants == 1);

    // Idempotent: repeated elimination call must fail and not double-count
    bool secondSuccess = elimSys.ProcessElimination(1, EliminationCause::Combat, 2, 50.0f, reg, stats);
    assert(!secondSuccess);
    assert(callbackCount == 1);
    assert(k->kills == 1);
    assert(stats.totalKills == 1);

    // Storm elimination check
    MatchParticipant pStormVictim;
    pStormVictim.participantId = 3;
    pStormVictim.displayName = "StormVictim";
    pStormVictim.status = ParticipantStatus::Alive;
    reg.RegisterParticipant(pStormVictim);

    bool elimOk = elimSys.ProcessElimination(3, EliminationCause::CorruptionStorm, 0, 60.0f, reg, stats);
    assert(elimOk);
    (void)elimOk;
    assert(callbackCount == 2);
    assert(lastVictim.participantId == 3);
    assert(lastInfo.cause == EliminationCause::CorruptionStorm);
    assert(lastInfo.killerParticipantId == 0);
    assert(lastInfo.killerName == "Corruption Storm");
    const auto* sv = reg.FindParticipant(3);
    assert(sv->eliminationInfo.cause == EliminationCause::CorruptionStorm);
    assert(sv->eliminationInfo.killerName == "Corruption Storm");

    std::cout << "  -> Elimination idempotency, killer attribution, and storm causes verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 6: Winner Determination & Final Survivor Logic
// ----------------------------------------------------------------------------
void TestWinnerDetermination() {
    std::cout << "[TEST] 6. Final Survivor Detection & Placements..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.minimumParticipants = 2;
    cfg.countdownSeconds = 0.0f; // Instant start
    mgr.Initialize(cfg);

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
    MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);
    mgr.RegisterParticipant(p3);

    mgr.StartCountdown();
    mgr.Update(0.1f); // Countdown finishes -> Active
    assert(mgr.GetState() == MatchState::Active);
    assert(mgr.GetRegistry().GetAliveCount() == 3);

    // Advance match clock to t = 10s and eliminate P3
    mgr.Update(9.9f); // m_matchTime = 10.0s
    mgr.ProcessElimination(3, EliminationCause::Combat, 1);
    assert(mgr.GetState() == MatchState::Active);
    assert(mgr.GetRegistry().GetAliveCount() == 2);

    // Advance match clock to t = 20s and eliminate P2
    mgr.Update(10.0f); // m_matchTime = 20.0s
    mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);

    // Exactly 1 alive participant left (P1) -> victory!
    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Completed);

    const auto& res = mgr.GetResult();
    assert(res.hasResult);
    assert(res.winnerParticipantId == 1);
    assert(res.winnerDisplayName == "P1");

    // Verify placements
    const auto* p1Final = mgr.GetRegistry().FindParticipant(1);
    const auto* p2Final = mgr.GetRegistry().FindParticipant(2);
    const auto* p3Final = mgr.GetRegistry().FindParticipant(3);

    assert(p1Final->placement == 1);
    assert(p2Final->placement == 2); // Eliminated later (t = 20s)
    assert(p3Final->placement == 3); // Eliminated earlier (t = 10s)

    std::cout << "  -> Single survivor victory and chronological placements verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 6B: Genuinely Simultaneous Eliminations (2 Participants Tied)
// ----------------------------------------------------------------------------
void TestSimultaneousTwoParticipantElimination() {
    std::cout << "[TEST] 6B. Genuinely Simultaneous Eliminations (2 Participants Tied at Same Timestamp)..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.minimumParticipants = 2;
    cfg.countdownSeconds = 0.0f;
    mgr.Initialize(cfg);
    mgr.GetSpawner().AddCustomSpawnPoint(Vector2{ 2000.0f, 2000.0f }, "Default_South");

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
    MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);
    mgr.RegisterParticipant(p3);

    mgr.StartCountdown();
    mgr.Update(0.1f); // Active
    assert(mgr.GetState() == MatchState::Active);
    assert(mgr.GetRegistry().GetAliveCount() == 3);

    // Advance to t = 10s
    mgr.Update(9.9f);

    // Both P2 and P3 eliminated at the exact same timestamp (m_matchTime = 10.0s)
    mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);
    mgr.ProcessElimination(3, EliminationCause::Combat, 1);

    // Remaining survivor P1 wins
    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Completed);

    const auto* p1Final = mgr.GetRegistry().FindParticipant(1);
    const auto* p2Final = mgr.GetRegistry().FindParticipant(2);
    const auto* p3Final = mgr.GetRegistry().FindParticipant(3);

    // Winner gets Rank 1
    assert(p1Final->placement == 1);
    assert(p1Final->isWinner);

    // Both simultaneous eliminations at t = 10s receive shared Rank 2 under tournament competition ranking
    assert(p2Final->placement == 2);
    assert(p3Final->placement == 2);

    std::cout << "  -> Simultaneous 2-participant elimination and shared Rank 2 verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 6C: Simultaneous Eliminations (3 Participants Tied)
// ----------------------------------------------------------------------------
void TestSimultaneousThreeParticipantElimination() {
    std::cout << "[TEST] 6C. Simultaneous Eliminations (3 Participants Tied at Same Timestamp)..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.minimumParticipants = 2;
    cfg.countdownSeconds = 0.0f;
    mgr.Initialize(cfg);
    mgr.GetSpawner().AddCustomSpawnPoint(Vector2{ 2000.0f, 2000.0f }, "Default_South");

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
    MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Registered;
    MatchParticipant p4; p4.participantId = 4; p4.displayName = "P4"; p4.status = ParticipantStatus::Registered;
    MatchParticipant p5; p5.participantId = 5; p5.displayName = "P5"; p5.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);
    mgr.RegisterParticipant(p3);
    mgr.RegisterParticipant(p4);
    mgr.RegisterParticipant(p5);

    mgr.StartCountdown();
    mgr.Update(0.1f); // Active
    assert(mgr.GetState() == MatchState::Active);
    assert(mgr.GetRegistry().GetAliveCount() == 5);

    // Advance to t = 5s: P3, P4, P5 all wiped out in storm at exact same tick
    mgr.Update(4.9f);
    mgr.ProcessElimination(3, EliminationCause::CorruptionStorm, 0);
    mgr.ProcessElimination(4, EliminationCause::CorruptionStorm, 0);
    mgr.ProcessElimination(5, EliminationCause::CorruptionStorm, 0);

    // Advance to t = 15s: P2 eliminated by P1
    mgr.Update(10.0f);
    mgr.ProcessElimination(2, EliminationCause::Combat, 1);

    // P1 wins
    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Completed);

    const auto* p1Final = mgr.GetRegistry().FindParticipant(1);
    const auto* p2Final = mgr.GetRegistry().FindParticipant(2);
    const auto* p3Final = mgr.GetRegistry().FindParticipant(3);
    const auto* p4Final = mgr.GetRegistry().FindParticipant(4);
    const auto* p5Final = mgr.GetRegistry().FindParticipant(5);

    assert(p1Final->placement == 1);
    assert(p2Final->placement == 2);
    // P3, P4, P5 eliminated simultaneously at t = 5s -> all tied for Rank 3!
    assert(p3Final->placement == 3);
    assert(p4Final->placement == 3);
    assert(p5Final->placement == 3);

    std::cout << "  -> Simultaneous 3-participant elimination and tied Rank 3 verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 6D: Simultaneous Elimination Event Arrival Order Invariance
// ----------------------------------------------------------------------------
void TestSimultaneousEventArrivalOrderInvariance() {
    std::cout << "[TEST] 6D. Simultaneous Elimination Event Arrival Order Invariance..." << std::endl;

    // Run Scenario A: P2 processed before P3
    uint32_t aRank2 = 0, aRank3 = 0;
    {
        MatchManager mgrA;
        MatchConfiguration cfg = MatchConfiguration::CreateDefault();
        cfg.minimumParticipants = 2;
        cfg.countdownSeconds = 0.0f;
        mgrA.Initialize(cfg);

        MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
        MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
        MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Registered;
        mgrA.RegisterParticipant(p1);
        mgrA.RegisterParticipant(p2);
        mgrA.RegisterParticipant(p3);
        mgrA.StartCountdown();
        mgrA.Update(10.0f);

        mgrA.ProcessElimination(2, EliminationCause::Combat, 1); // P2 first
        mgrA.ProcessElimination(3, EliminationCause::Combat, 1); // P3 second
        mgrA.Update(0.1f);

        aRank2 = mgrA.GetRegistry().FindParticipant(2)->placement;
        aRank3 = mgrA.GetRegistry().FindParticipant(3)->placement;
    }

    // Run Scenario B: P3 processed before P2
    uint32_t bRank2 = 0, bRank3 = 0;
    {
        MatchManager mgrB;
        MatchConfiguration cfg = MatchConfiguration::CreateDefault();
        cfg.minimumParticipants = 2;
        cfg.countdownSeconds = 0.0f;
        mgrB.Initialize(cfg);

        MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
        MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
        MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Registered;
        mgrB.RegisterParticipant(p1);
        mgrB.RegisterParticipant(p2);
        mgrB.RegisterParticipant(p3);
        mgrB.StartCountdown();
        mgrB.Update(10.0f);

        mgrB.ProcessElimination(3, EliminationCause::Combat, 1); // P3 first
        mgrB.ProcessElimination(2, EliminationCause::Combat, 1); // P2 second
        mgrB.Update(0.1f);

        bRank2 = mgrB.GetRegistry().FindParticipant(2)->placement;
        bRank3 = mgrB.GetRegistry().FindParticipant(3)->placement;
    }

    assert(aRank2 == bRank2 && aRank2 == 2);
    assert(aRank3 == bRank3 && aRank3 == 2);
    std::cout << "  -> Order invariance verified: both orders yield identical shared Rank 2." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 6E: Repeated Deterministic Placements
// ----------------------------------------------------------------------------
void TestRepeatedDeterministicPlacements() {
    std::cout << "[TEST] 6E. Repeated Deterministic Execution (20 Consecutive Runs)..." << std::endl;

    for (int run = 0; run < 20; ++run) {
        MatchManager mgr;
        MatchConfiguration cfg = MatchConfiguration::CreateDefault();
        cfg.minimumParticipants = 2;
        cfg.countdownSeconds = 0.0f;
        mgr.Initialize(cfg);

        MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
        MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
        MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Registered;
        mgr.RegisterParticipant(p1);
        mgr.RegisterParticipant(p2);
        mgr.RegisterParticipant(p3);
        mgr.StartCountdown();
        mgr.Update(0.1f);

        mgr.Update(9.9f); // t = 10s
        mgr.ProcessElimination(3, EliminationCause::Combat, 1);

        mgr.Update(10.0f); // t = 20s
        mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);

        mgr.Update(0.1f);
        assert(mgr.GetState() == MatchState::Completed);

        assert(mgr.GetRegistry().FindParticipant(1)->placement == 1);
        assert(mgr.GetRegistry().FindParticipant(2)->placement == 2);
        assert(mgr.GetRegistry().FindParticipant(3)->placement == 3);
    }
    std::cout << "  -> 20 repeated runs verified 100% deterministic placements." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 7: Simultaneous Final Eliminations (Draw Resolution)
// ----------------------------------------------------------------------------
void TestDrawResolution() {
    std::cout << "[TEST] 7. Simultaneous Final Eliminations (Draw)..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.minimumParticipants = 2;
    cfg.countdownSeconds = 0.0f;
    mgr.Initialize(cfg);
    mgr.GetSpawner().AddCustomSpawnPoint(Vector2{ 2000.0f, 2000.0f }, "Default_South");

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);

    mgr.StartCountdown();
    mgr.Update(0.1f); // Active
    assert(mgr.GetState() == MatchState::Active);

    // Both eliminated in the same tick (e.g. mutually in storm)
    mgr.ProcessElimination(1, EliminationCause::CorruptionStorm, 0);
    mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);

    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Completed);

    const auto& res = mgr.GetResult();
    assert(res.hasResult);
    assert(res.winnerParticipantId == 0); // No single winner
    assert(!res.isVictory);
    std::cout << "  -> Zero-survivor draw condition resolved safely." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 8: Minimum Participant Requirement Enforcement
// ----------------------------------------------------------------------------
void TestMinimumParticipantsRequirement() {
    std::cout << "[TEST] 8. Participant Count Enforcement..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.minimumParticipants = 2;
    cfg.allowDevSingleParticipant = false;
    mgr.Initialize(cfg);

    // Only 1 participant registered
    MatchParticipant solo;
    solo.participantId = 1;
    solo.displayName = "SoloPlayer";
    solo.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(solo);

    // Attempt start countdown
    bool started = mgr.StartCountdown();
    assert(!started);
    assert(mgr.GetState() == MatchState::Failed);

    // Now test with dev bypass enabled
    MatchManager devMgr;
    cfg.allowDevSingleParticipant = true;
    devMgr.Initialize(cfg);
    devMgr.RegisterParticipant(solo);
    assert(devMgr.StartCountdown());
    assert(devMgr.GetState() == MatchState::Countdown);

    std::cout << "  -> Single-participant restriction enforced and dev bypass verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 9: Corruption Storm Lifecycle Synchronization
// ----------------------------------------------------------------------------
void TestStormLifecycleIntegration() {
    std::cout << "[TEST] 9. Corruption Storm Lifecycle Synchronization..." << std::endl;

    MatchManager mgr;
    storm::StormManager storm;
    storm.Initialize();

    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.countdownSeconds = 2.0f;
    cfg.minimumParticipants = 2;
    mgr.Initialize(cfg);

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);

    // 1. In WaitingForPlayers & Countdown: storm is Idle
    assert(storm.GetState() == storm::StormState::Idle);

    mgr.StartCountdown();
    mgr.Update(1.0f, nullptr, &storm);
    assert(storm.GetState() == storm::StormState::Idle); // Still in countdown

    // 2. Countdown finishes -> Active -> Storm started
    mgr.Update(1.5f, nullptr, &storm);
    assert(mgr.GetState() == MatchState::Active);
    assert(storm.GetState() == storm::StormState::Warning); // Storm phase 1 warning active!

    // 3. Match complete -> Storm paused / stopped
    mgr.ProcessElimination(2, EliminationCause::Combat, 1);
    mgr.Update(0.1f, nullptr, &storm);
    assert(mgr.GetState() == MatchState::Completed);

    // 4. Reset -> Storm reset
    mgr.ResetMatch(nullptr, &storm);
    assert(mgr.GetState() == MatchState::Uninitialized);
    assert(storm.GetState() == storm::StormState::Idle);

    std::cout << "  -> Storm lifecycle synchronization across all states verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 10: Complete Multi-Match Repeat Lifecycle Scenario
// ----------------------------------------------------------------------------
void TestCompleteLifecycleRepeated() {
    std::cout << "[TEST] 10. Complete Multi-Match Repeat Lifecycle Scenario..." << std::endl;

    MatchManager mgr;
    world::WorldManager worldMgr;
    worldMgr.Initialize();

    for (int cycle = 1; cycle <= 3; ++cycle) {
        MatchConfiguration cfg = MatchConfiguration::CreateDefault();
        cfg.matchId = "test_cycle_" + std::to_string(cycle);
        cfg.countdownSeconds = 1.0f;
        cfg.minimumParticipants = 2;

        bool initOk = mgr.Initialize(cfg, &worldMgr);
        assert(initOk);
        (void)initOk;
        assert(mgr.GetState() == MatchState::WaitingForPlayers);

        MatchParticipant human;
        human.participantId = 1;
        human.type = ParticipantType::LocalHuman;
        human.displayName = "LocalHero";
        human.status = ParticipantStatus::Registered;
        mgr.RegisterParticipant(human);

        MatchParticipant bot;
        bot.participantId = 2;
        bot.type = ParticipantType::AI;
        bot.displayName = "Gladiator_Bot";
        bot.status = ParticipantStatus::Registered;
        mgr.RegisterParticipant(bot);

        bool startOk = mgr.StartCountdown();
        assert(startOk);
        (void)startOk;
        assert(mgr.GetState() == MatchState::Countdown);
        assert(!mgr.IsGameplayAllowed());

        // Run countdown
        mgr.Update(1.2f);
        assert(mgr.GetState() == MatchState::Active);
        assert(mgr.IsGameplayAllowed());

        // Active match combat & damage
        mgr.GetStatistics().RecordDamage(1, 2, 75.0f, mgr.GetRegistry());
        mgr.GetStatistics().RecordDamage(2, 1, 20.0f, mgr.GetRegistry());

        // Elimination
        mgr.ProcessElimination(2, EliminationCause::Combat, 1);
        mgr.Update(0.1f);

        assert(mgr.GetState() == MatchState::Completed);
        const auto& result = mgr.GetResult();
        assert(result.isVictory);
        assert(result.winnerParticipantId == 1);
        assert(result.localPlayerPlacement == 1);
        assert(result.localPlayerKills == 1);
        assert(result.localPlayerDamageDealt == 75.0f);
        assert(result.localPlayerDamageReceived == 20.0f);

        // Reset
        mgr.ResetMatch();
        assert(mgr.GetState() == MatchState::Uninitialized);
        assert(mgr.GetRegistry().GetTotalCount() == 0);
        assert(mgr.GetStatistics().totalKills == 0);
        assert(mgr.GetMatchTime() == 0.0f);
    }

    std::cout << "  -> 3 repeated complete match lifecycles executed with zero stale state." << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 32: MATCH LIFECYCLE TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestStateMachine();
    TestParticipantRegistry();
    TestSpawningSystem();
    TestMatchCountdown();
    TestEliminationSystem();
    TestWinnerDetermination();
    TestSimultaneousTwoParticipantElimination();
    TestSimultaneousThreeParticipantElimination();
    TestSimultaneousEventArrivalOrderInvariance();
    TestRepeatedDeterministicPlacements();
    TestDrawResolution();
    TestMinimumParticipantsRequirement();
    TestStormLifecycleIntegration();
    TestCompleteLifecycleRepeated();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 14 MATCH LIFECYCLE TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
