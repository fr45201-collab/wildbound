#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <filesystem>

#include "results/MatchOutcome.hpp"
#include "results/EliminationRecord.hpp"
#include "results/PlayerPerformance.hpp"
#include "results/MatchResultModel.hpp"
#include "results/VictoryEvaluator.hpp"
#include "results/DefeatEvaluator.hpp"
#include "results/ResultCalculator.hpp"
#include "results/ResultSerializer.hpp"
#include "results/MatchResultManager.hpp"

#include "match/MatchState.hpp"
#include "match/MatchConfiguration.hpp"
#include "match/MatchParticipant.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "match/MatchStatistics.hpp"
#include "match/MatchManager.hpp"
#include "storm/StormManager.hpp"

using namespace wildbound;
using namespace wildbound::match;
using namespace wildbound::results;

// ----------------------------------------------------------------------------
// TEST 1: Victory Condition Evaluation
// ----------------------------------------------------------------------------
void TestVictoryEvaluator() {
    std::cout << "[TEST] 1. Victory Evaluator Scenarios..." << std::endl;

    VictoryEvaluator evaluator;
    MatchParticipantRegistry reg;

    MatchOutcome outcome = MatchOutcome::InProgress;
    uint32_t winnerId = 0;
    std::string winnerName;

    // A. Match not active -> InProgress
    bool decisive = evaluator.Evaluate(reg, false, outcome, winnerId, winnerName);
    assert(!decisive);
    assert(outcome == MatchOutcome::InProgress);

    // B. Match active but empty registry -> Failed
    decisive = evaluator.Evaluate(reg, true, outcome, winnerId, winnerName);
    assert(decisive);
    assert(outcome == MatchOutcome::Failed);

    // C. Register 3 participants, all alive -> InProgress
    MatchParticipant p1; p1.participantId = 1; p1.displayName = "P1"; p1.status = ParticipantStatus::Alive;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "P2"; p2.status = ParticipantStatus::Alive;
    MatchParticipant p3; p3.participantId = 3; p3.displayName = "P3"; p3.status = ParticipantStatus::Alive;
    reg.RegisterParticipant(p1);
    reg.RegisterParticipant(p2);
    reg.RegisterParticipant(p3);

    decisive = evaluator.Evaluate(reg, true, outcome, winnerId, winnerName);
    assert(!decisive);
    assert(outcome == MatchOutcome::InProgress);

    // D. Eliminate 2, 1 remains -> Decisive Victory!
    reg.FindParticipant(2)->status = ParticipantStatus::Eliminated;
    reg.FindParticipant(3)->status = ParticipantStatus::Eliminated;

    decisive = evaluator.Evaluate(reg, true, outcome, winnerId, winnerName);
    assert(decisive);
    assert(outcome == MatchOutcome::Victory);
    assert(winnerId == 1);
    assert(winnerName == "P1");

    // E. Eliminate the last one -> Decisive Draw (0 survivors)
    reg.FindParticipant(1)->status = ParticipantStatus::Eliminated;

    decisive = evaluator.Evaluate(reg, true, outcome, winnerId, winnerName);
    assert(decisive);
    assert(outcome == MatchOutcome::Draw);
    assert(winnerId == 0);
    assert(winnerName == "No Winner (Draw)");

    std::cout << "  -> Victory, in-progress, failed, and draw outcomes verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 2: Defeat Condition Evaluation
// ----------------------------------------------------------------------------
void TestDefeatEvaluator() {
    std::cout << "[TEST] 2. Defeat Evaluator Scenarios..." << std::endl;

    DefeatEvaluator evaluator;

    MatchParticipant pAlive;
    pAlive.participantId = 1;
    pAlive.displayName = "Survivor";
    pAlive.status = ParticipantStatus::Alive;

    MatchParticipant pDeadCombat;
    pDeadCombat.participantId = 2;
    pDeadCombat.displayName = "Casualty";
    pDeadCombat.status = ParticipantStatus::Eliminated;
    pDeadCombat.eliminationInfo.cause = EliminationCause::Combat;
    pDeadCombat.eliminationInfo.killerParticipantId = 1;
    pDeadCombat.eliminationInfo.killerName = "Survivor";

    MatchParticipant pDeadStorm;
    pDeadStorm.participantId = 3;
    pDeadStorm.displayName = "Wanderer";
    pDeadStorm.status = ParticipantStatus::Eliminated;
    pDeadStorm.eliminationInfo.cause = EliminationCause::CorruptionStorm;
    pDeadStorm.eliminationInfo.killerName = "Corruption Storm";

    // Winner gets PlayerOutcome::Victory
    PlayerOutcome out1 = evaluator.EvaluateParticipantOutcome(pAlive, MatchOutcome::Victory, 1);
    assert(out1 == PlayerOutcome::Victory);

    // Eliminated players get PlayerOutcome::Defeat
    PlayerOutcome out2 = evaluator.EvaluateParticipantOutcome(pDeadCombat, MatchOutcome::Victory, 1);
    assert(out2 == PlayerOutcome::Defeat);

    PlayerOutcome out3 = evaluator.EvaluateParticipantOutcome(pDeadStorm, MatchOutcome::Victory, 1);
    assert(out3 == PlayerOutcome::Defeat);

    // In a Draw match, all participants get PlayerOutcome::Draw
    PlayerOutcome outDraw = evaluator.EvaluateParticipantOutcome(pAlive, MatchOutcome::Draw, 0);
    assert(outDraw == PlayerOutcome::Draw);

    // Format defeat reasons
    std::string reasonCombat = evaluator.FormatDefeatReason(pDeadCombat);
    assert(reasonCombat == "Eliminated by Survivor");

    std::string reasonStorm = evaluator.FormatDefeatReason(pDeadStorm);
    assert(reasonStorm == "Consumed by Corruption Storm");

    std::cout << "  -> Personal outcome determination and reason formatting verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 3: Survival Duration Calculation & Formatting
// ----------------------------------------------------------------------------
void TestDurationFormatting() {
    std::cout << "[TEST] 3. Duration Calculation & Formatting..." << std::endl;

    // Negative clamps to 00:00
    assert(ResultCalculator::FormatDuration(-10.0f) == "00:00");
    assert(ResultCalculator::FormatDuration(0.0f) == "00:00");

    // Standard MM:SS under 1 hour
    assert(ResultCalculator::FormatDuration(45.0f) == "00:45");
    assert(ResultCalculator::FormatDuration(768.0f) == "12:48"); // 12m 48s
    assert(ResultCalculator::FormatDuration(3599.0f) == "59:59");

    // HH:MM:SS for 1 hour and above
    assert(ResultCalculator::FormatDuration(3600.0f) == "01:00:00");
    assert(ResultCalculator::FormatDuration(3665.0f) == "01:01:05");
    assert(ResultCalculator::FormatDuration(7325.0f) == "02:02:05");

    std::cout << "  -> Duration formatting across MM:SS and HH:MM:SS boundaries verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 4: Elimination Record Logging & Chronological Audit
// ----------------------------------------------------------------------------
void TestEliminationRecordLogging() {
    std::cout << "[TEST] 4. Chronological Elimination Logging..." << std::endl;

    MatchResultManager resMgr;

    MatchParticipant v1; v1.participantId = 10; v1.displayName = "Victim10";
    EliminationInfo info1; info1.cause = EliminationCause::Combat; info1.killerParticipantId = 1; info1.killerName = "Killer1";
    resMgr.RecordEliminationEvent(v1, info1, 45.0f);

    MatchParticipant v2; v2.participantId = 11; v2.displayName = "Victim11";
    EliminationInfo info2; info2.cause = EliminationCause::CorruptionStorm; info2.killerName = "Corruption Storm";
    resMgr.RecordEliminationEvent(v2, info2, 120.5f);

    const auto& logs = resMgr.GetEliminationRecords();
    assert(logs.size() == 2);

    assert(logs[0].eventId == 1);
    assert(logs[0].victimParticipantId == 10);
    assert(logs[0].victimName == "Victim10");
    assert(logs[0].killerParticipantId == 1);
    assert(logs[0].killerName == "Killer1");
    assert(logs[0].matchElapsedTime == 45.0f);

    assert(logs[1].eventId == 2);
    assert(logs[1].victimParticipantId == 11);
    assert(logs[1].victimName == "Victim11");
    assert(logs[1].cause == EliminationCause::CorruptionStorm);
    assert(logs[1].matchElapsedTime == 120.5f);

    std::cout << "  -> Incremental event IDs and chronological audit logs verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 5: Placement Calculation & Simultaneous Elimination Ties
// ----------------------------------------------------------------------------
void TestPlacementCalculationWithTies() {
    std::cout << "[TEST] 5. Placement Calculation & Tie Resolution..." << std::endl;

    ResultCalculator calc;
    MatchParticipantRegistry reg;

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "Winner"; p1.status = ParticipantStatus::Alive;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "Tie2_A"; p2.status = ParticipantStatus::Eliminated; p2.eliminationInfo.timestamp = 50.0f;
    MatchParticipant p3; p3.participantId = 3; p3.displayName = "Tie2_B"; p3.status = ParticipantStatus::Eliminated; p3.eliminationInfo.timestamp = 50.0f; // Simultaneous!
    MatchParticipant p4; p4.participantId = 4; p4.displayName = "Fourth"; p4.status = ParticipantStatus::Eliminated; p4.eliminationInfo.timestamp = 20.0f;

    reg.RegisterParticipant(p1);
    reg.RegisterParticipant(p2);
    reg.RegisterParticipant(p3);
    reg.RegisterParticipant(p4);

    calc.CalculatePlacements(reg, 1, MatchOutcome::Victory);

    const auto* r1 = reg.FindParticipant(1);
    const auto* r2 = reg.FindParticipant(2);
    const auto* r3 = reg.FindParticipant(3);
    const auto* r4 = reg.FindParticipant(4);

    // Winner gets Rank 1
    assert(r1->placement == 1);
    assert(r1->isWinner);

    // Both simultaneous eliminations at 50s get Rank 2
    assert(r2->placement == 2);
    assert(r3->placement == 2);

    // Next placement skips to Rank 4 (1 + 2 + 1 = 4)
    assert(r4->placement == 4);

    std::cout << "  -> Standard tournament tie resolution (tied ranks with subsequent rank skips) verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 6: Match Statistics & Separated Storm vs Combat Damage
// ----------------------------------------------------------------------------
void TestPlayerStatisticsAndDamageAttribution() {
    std::cout << "[TEST] 6. Separated Storm vs Combat Damage Attribution..." << std::endl;

    MatchStatistics stats;
    MatchParticipantRegistry reg;

    MatchParticipant p1; p1.participantId = 1; p1.status = ParticipantStatus::Alive;
    MatchParticipant p2; p2.participantId = 2; p2.status = ParticipantStatus::Alive;
    reg.RegisterParticipant(p1);
    reg.RegisterParticipant(p2);

    // Apply combat damage from P1 to P2
    stats.RecordDamage(1, 2, 40.0f, reg, false);

    // Apply storm damage to P2
    stats.RecordDamage(0, 2, 25.0f, reg, true);

    const auto* r2 = reg.FindParticipant(2);
    assert(r2->damageReceived == 65.0f);
    assert(r2->combatDamageReceived == 40.0f);
    assert(r2->stormDamageReceived == 25.0f);

    const auto* r1 = reg.FindParticipant(1);
    assert(r1->damageDealt == 40.0f);

    std::cout << "  -> Accurate damage attribution separating combat from storm verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 7: Result Finalization Immutability
// ----------------------------------------------------------------------------
void TestResultFinalizationImmutability() {
    std::cout << "[TEST] 7. Result Finalization & Immutability..." << std::endl;

    MatchResultManager resMgr;
    MatchParticipantRegistry reg;
    MatchStatistics stats;

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "Hero"; p1.status = ParticipantStatus::Alive;
    reg.RegisterParticipant(p1);

    bool firstFinalize = resMgr.FinalizeMatchResult("match_immutability_test", 120.0f, 1, reg, stats, false);
    assert(firstFinalize);
    assert(resMgr.IsFinalized());

    const auto& res = resMgr.GetFinalResult();
    assert(res.matchId == "match_immutability_test");
    assert(res.matchDuration == 120.0f);
    assert(res.localPlayerPerformance.placement == 1);

    // Attempting to finalize again must be rejected
    bool secondFinalize = resMgr.FinalizeMatchResult("match_immutability_test", 150.0f, 1, reg, stats, false);
    assert(!secondFinalize);
    assert(resMgr.GetFinalResult().matchDuration == 120.0f); // Unaltered

    // Attempting to record elimination after finalization must be rejected
    resMgr.RecordEliminationEvent(p1, EliminationInfo{}, 130.0f);
    assert(resMgr.GetEliminationRecords().empty());

    std::cout << "  -> Immutability of finalized result snapshots verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 8: Local Match History Persistence (JSON Serializer)
// ----------------------------------------------------------------------------
void TestResultSerializerPersistence() {
    std::cout << "[TEST] 8. JSON Result Serialization & Persistence..." << std::endl;

    ResultSerializer serializer;
    std::string testDir = "user_data/test_match_history";

    MatchResultModel model;
    model.matchId = "test_serialization_match_001";
    model.matchOutcome = MatchOutcome::Victory;
    model.localPlayerOutcome = PlayerOutcome::Victory;
    model.winnerParticipantId = 1;
    model.winnerDisplayName = "LocalHero";
    model.matchDuration = 540.5f;
    model.totalParticipants = 4;
    model.totalEliminations = 3;
    model.survivorsCount = 1;
    model.completionReason = "Sole survivor emerged victorious.";
    model.isFinalized = true;

    // Local player
    model.localPlayerPerformance.participantId = 1;
    model.localPlayerPerformance.displayName = "LocalHero";
    model.localPlayerPerformance.placement = 1;
    model.localPlayerPerformance.survivalDuration = 540.5f;
    model.localPlayerPerformance.kills = 3;
    model.localPlayerPerformance.damageDealt = 450.0f;
    model.localPlayerPerformance.combatDamageReceived = 50.0f;
    model.localPlayerPerformance.stormDamageReceived = 10.0f;
    model.localPlayerPerformance.totalDamageReceived = 60.0f;
    model.localPlayerPerformance.isWinner = true;
    model.participantPerformances.push_back(model.localPlayerPerformance);

    // Elimination record
    EliminationRecord elim;
    elim.eventId = 1;
    elim.victimParticipantId = 2;
    elim.victimName = "Competitor_2";
    elim.killerParticipantId = 1;
    elim.killerName = "LocalHero";
    elim.cause = EliminationCause::Combat;
    elim.matchElapsedTime = 120.0f;
    elim.placement = 4;
    model.eliminationRecords.push_back(elim);

    // Save
    bool saveOk = serializer.SaveResult(model, testDir);
    assert(saveOk);

    // Load back
    std::string fullPath = testDir + "/test_serialization_match_001.json";
    MatchResultModel loadedModel;
    bool loadOk = serializer.LoadResult(fullPath, loadedModel);
    assert(loadOk);

    // Verify fidelity
    assert(loadedModel.isFinalized);
    assert(loadedModel.matchId == model.matchId);
    assert(loadedModel.matchOutcome == MatchOutcome::Victory);
    assert(loadedModel.localPlayerOutcome == PlayerOutcome::Victory);
    assert(loadedModel.winnerParticipantId == 1);
    assert(loadedModel.winnerDisplayName == "LocalHero");
    assert(loadedModel.matchDuration == 540.5f);
    assert(loadedModel.totalParticipants == 4);
    assert(loadedModel.totalEliminations == 3);

    assert(loadedModel.localPlayerPerformance.placement == 1);
    assert(loadedModel.localPlayerPerformance.kills == 3);
    assert(loadedModel.localPlayerPerformance.damageDealt == 450.0f);
    assert(loadedModel.localPlayerPerformance.combatDamageReceived == 50.0f);
    assert(loadedModel.localPlayerPerformance.stormDamageReceived == 10.0f);

    assert(loadedModel.eliminationRecords.size() == 1);
    assert(loadedModel.eliminationRecords[0].victimName == "Competitor_2");

    // Cleanup test artifacts
    std::filesystem::remove_all(testDir);

    std::cout << "  -> Complete JSON serialization, schema validation, and round-trip fidelity verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 9: Full End-to-End Match Lifecycle & Results Integration
// ----------------------------------------------------------------------------
void TestFullMatchLifecycleIntegration() {
    std::cout << "[TEST] 9. End-to-End Match Lifecycle & Results Integration..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.matchId = "phase33_e2e_match";
    cfg.countdownSeconds = 0.0f; // Instant
    cfg.minimumParticipants = 3;
    mgr.Initialize(cfg);

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "Player1"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "Player2"; p2.status = ParticipantStatus::Registered;
    MatchParticipant p3; p3.participantId = 3; p3.displayName = "Player3"; p3.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);
    mgr.RegisterParticipant(p3);

    mgr.StartCountdown();
    mgr.Update(0.1f); // Transitions to Active
    assert(mgr.GetState() == MatchState::Active);

    // Apply combat damage from P1 to P3
    mgr.GetStatistics().RecordDamage(1, 3, 100.0f, mgr.GetRegistry(), false);

    // Eliminate P3 by Combat from P1
    mgr.ProcessElimination(3, EliminationCause::Combat, 1);
    assert(mgr.GetState() == MatchState::Active);

    // Apply storm damage to P2
    mgr.GetStatistics().RecordDamage(0, 2, 50.0f, mgr.GetRegistry(), true);

    // Eliminate P2 by Corruption Storm
    mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);

    // Exactly 1 survivor remaining (P1) -> MatchManager completes match
    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Completed);

    // Verify Phase 33 results model
    const auto& finalRes = mgr.GetFinalResultModel();
    assert(finalRes.isFinalized);
    assert(finalRes.matchOutcome == MatchOutcome::Victory);
    assert(finalRes.localPlayerOutcome == PlayerOutcome::Victory);
    assert(finalRes.winnerParticipantId == 1);
    assert(finalRes.winnerDisplayName == "Player1");

    // Local player performance
    assert(finalRes.localPlayerPerformance.placement == 1);
    assert(finalRes.localPlayerPerformance.kills == 1);
    assert(finalRes.localPlayerPerformance.damageDealt == 100.0f);

    // Eliminations log
    assert(finalRes.eliminationRecords.size() == 2);
    assert(finalRes.eliminationRecords[0].victimParticipantId == 3);
    assert(finalRes.eliminationRecords[0].cause == EliminationCause::Combat);
    assert(finalRes.eliminationRecords[1].victimParticipantId == 2);
    assert(finalRes.eliminationRecords[1].cause == EliminationCause::CorruptionStorm);

    // Reset match and verify clean state
    mgr.ResetMatch();
    assert(mgr.GetState() == MatchState::Uninitialized);
    assert(!mgr.GetFinalResultModel().isFinalized);
    assert(mgr.GetResultManager().GetEliminationRecords().empty());

    std::cout << "  -> Complete match progression from combat to victory finalization and reset verified." << std::endl;
}

// ----------------------------------------------------------------------------
// TEST 10: Simultaneous Elimination Draw Resolution Integration
// ----------------------------------------------------------------------------
void TestSimultaneousDrawIntegration() {
    std::cout << "[TEST] 10. Simultaneous Draw Resolution Integration..." << std::endl;

    MatchManager mgr;
    MatchConfiguration cfg = MatchConfiguration::CreateDefault();
    cfg.matchId = "draw_simulation_match";
    cfg.countdownSeconds = 0.0f;
    cfg.minimumParticipants = 2;
    mgr.Initialize(cfg);

    MatchParticipant p1; p1.participantId = 1; p1.displayName = "Gladiator_A"; p1.status = ParticipantStatus::Registered;
    MatchParticipant p2; p2.participantId = 2; p2.displayName = "Gladiator_B"; p2.status = ParticipantStatus::Registered;
    mgr.RegisterParticipant(p1);
    mgr.RegisterParticipant(p2);

    mgr.StartCountdown();
    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Active);

    // Both perish simultaneously in the storm
    mgr.ProcessElimination(1, EliminationCause::CorruptionStorm, 0);
    mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);

    mgr.Update(0.1f);
    assert(mgr.GetState() == MatchState::Completed);

    const auto& res = mgr.GetFinalResultModel();
    assert(res.isFinalized);
    assert(res.matchOutcome == MatchOutcome::Draw);
    assert(res.localPlayerOutcome == PlayerOutcome::Draw);
    assert(res.winnerParticipantId == 0);
    assert(res.winnerDisplayName == "No Winner (Draw)");

    // Placements: both get Rank #1 in a mutual draw
    for (const auto& p : res.participantPerformances) {
        assert(p.placement == 1);
        assert(!p.isWinner);
    }

    std::cout << "  -> Simultaneous elimination draw state and equal rank tie handling verified." << std::endl;
}

// ----------------------------------------------------------------------------
// MAIN TEST RUNNER
// ----------------------------------------------------------------------------
int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 33: VICTORY & RESULTS TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestVictoryEvaluator();
    TestDefeatEvaluator();
    TestDurationFormatting();
    TestEliminationRecordLogging();
    TestPlacementCalculationWithTies();
    TestPlayerStatisticsAndDamageAttribution();
    TestResultFinalizationImmutability();
    TestResultSerializerPersistence();
    TestFullMatchLifecycleIntegration();
    TestSimultaneousDrawIntegration();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 PHASE 33 RESULT TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

