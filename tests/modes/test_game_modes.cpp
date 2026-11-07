#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <memory>

#include "gamemodes/GameModeManager.hpp"
#include "gamemodes/GameModeRegistry.hpp"
#include "gamemodes/GameModeDefinition.hpp"
#include "gamemodes/TeamManager.hpp"
#include "gamemodes/TeamVictoryEvaluator.hpp"
#include "gamemodes/BotMatchConfiguration.hpp"
#include "gamemodes/PracticeMode.hpp"

#include "match/MatchManager.hpp"
#include "match/MatchParticipantRegistry.hpp"
#include "match/MatchSpawner.hpp"
#include "combat/CombatSystem.hpp"
#include "combat/CombatRules.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "storm/SafeZone.hpp"
#include "storm/StormDamageSystem.hpp"
#include "core/Logger.hpp"

using namespace wildbound;
using namespace wildbound::gamemodes;
using namespace wildbound::match;
using namespace wildbound::combat;

static void TestGameModeRegistry() {
    std::cout << "[TEST] 1. Game Mode Registry & Configuration..." << std::endl;

    GameModeRegistry registry;
    assert(registry.GetModeCount() >= 5);
    assert(registry.HasMode("solo"));
    assert(registry.HasMode("duos"));
    assert(registry.HasMode("squads"));
    assert(registry.HasMode("practice"));
    assert(registry.HasMode("offline_bots"));

    const auto* solo = registry.GetMode("solo");
    assert(solo != nullptr);
    assert(solo->teamSize == 1);
    assert(!solo->teamsEnabled);
    assert(solo->victoryCondition == "last_survivor");

    const auto* duos = registry.GetMode("duos");
    assert(duos != nullptr);
    assert(duos->teamSize == 2);
    assert(duos->teamsEnabled);
    assert(duos->victoryCondition == "last_team");

    const auto* squads = registry.GetMode("squads");
    assert(squads != nullptr);
    assert(squads->teamSize == 4);
    assert(squads->teamsEnabled);

    // Validation checks
    GameModeDefinition invalidDef;
    invalidDef.id = "";
    std::vector<std::string> errors;
    assert(!invalidDef.Validate(errors));

    invalidDef.id = "broken";
    invalidDef.teamsEnabled = true;
    invalidDef.teamSize = 1; // Error: teams enabled requires teamSize >= 2
    errors.clear();
    assert(!invalidDef.Validate(errors));

    std::cout << "  -> Registry, discovery, and schema validation verified." << std::endl;
}

static void TestSoloMode() {
    std::cout << "[TEST] 2. Solo Battle Royale Ruleset..." << std::endl;

    GameModeManager modeMgr;
    assert(modeMgr.SelectMode("solo"));
    const auto& mode = modeMgr.GetActiveMode();
    assert(mode.id == "solo");
    assert(!mode.teamsEnabled);
    assert(mode.teamSize == 1);

    MatchParticipantRegistry registry;
    MatchParticipant p1;
    p1.participantId = 1;
    p1.displayName = "Gladiator_1";
    p1.status = ParticipantStatus::Alive;
    registry.RegisterParticipant(p1);

    MatchParticipant p2;
    p2.participantId = 2;
    p2.displayName = "Gladiator_2";
    p2.status = ParticipantStatus::Alive;
    registry.RegisterParticipant(p2);

    modeMgr.SetupTeamsForParticipants(registry);

    // In solo, participants are enemies
    assert(modeMgr.CanDamage(1, 2));
    assert(!modeMgr.CanDamage(1, 1)); // Cannot damage self

    results::MatchOutcome outcome;
    uint32_t winnerId = 0;
    std::string winnerName;

    // Both alive -> in progress
    bool decided = modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName);
    assert(!decided);
    assert(outcome == results::MatchOutcome::InProgress);

    // Eliminate p2
    registry.UpdateParticipantStatus(2, ParticipantStatus::Eliminated);
    decided = modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName);
    assert(decided);
    assert(outcome == results::MatchOutcome::Victory);
    assert(winnerId == 1);
    assert(winnerName == "Gladiator_1");

    std::cout << "  -> Solo individual combat and victory evaluation verified." << std::endl;
}

static void TestDuosMode() {
    std::cout << "[TEST] 3. Duos Battle Royale Ruleset & Teams..." << std::endl;

    GameModeManager modeMgr;
    assert(modeMgr.SelectMode("duos"));
    const auto& mode = modeMgr.GetActiveMode();
    assert(mode.teamsEnabled);
    assert(mode.teamSize == 2);

    MatchParticipantRegistry registry;
    // Duo 1: Players 1 & 2
    for (uint32_t id = 1; id <= 4; ++id) {
        MatchParticipant p;
        p.participantId = id;
        p.displayName = "Warrior_" + std::to_string(id);
        p.status = ParticipantStatus::Alive;
        registry.RegisterParticipant(p);
    }

    modeMgr.SetupTeamsForParticipants(registry);
    auto& teamMgr = modeMgr.GetTeamManager();
    assert(teamMgr.GetTotalTeamCount() == 2);

    // Check teammate relationships
    assert(teamMgr.IsTeammate(1, 2));
    assert(teamMgr.IsTeammate(3, 4));
    assert(!teamMgr.IsTeammate(1, 3));
    assert(teamMgr.IsEnemy(1, 3));
    assert(teamMgr.IsEnemy(2, 4));

    // Friendly fire disabled -> cannot damage teammate
    assert(!modeMgr.CanDamage(1, 2));
    assert(!modeMgr.CanDamage(3, 4));
    // Enemies can damage each other
    assert(modeMgr.CanDamage(1, 3));
    assert(modeMgr.CanDamage(2, 4));

    // One teammate eliminated in Duo 2: team remains alive!
    registry.UpdateParticipantStatus(4, ParticipantStatus::Eliminated);
    assert(teamMgr.GetParticipantTeam(3)->GetAliveMemberCount(registry) == 1);
    assert(teamMgr.GetAliveTeamCount(registry) == 2);

    results::MatchOutcome outcome;
    uint32_t winnerId = 0;
    std::string winnerName;
    assert(!modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName));

    // Second teammate in Duo 2 eliminated: Team 2 is eliminated!
    registry.UpdateParticipantStatus(3, ParticipantStatus::Eliminated);
    assert(teamMgr.GetParticipantTeam(3)->GetAliveMemberCount(registry) == 0);

    bool decided = modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName);
    assert(decided);
    assert(outcome == results::MatchOutcome::Victory);
    assert(winnerId == 1); // Team 1 won!

    std::cout << "  -> Duos team creation, partial elimination, and team victory verified." << std::endl;
}

static void TestSquadsMode() {
    std::cout << "[TEST] 4. Squads Battle Royale (4-Player Teams)..." << std::endl;

    GameModeManager modeMgr;
    assert(modeMgr.SelectMode("squads"));
    assert(modeMgr.GetActiveMode().teamSize == 4);

    MatchParticipantRegistry registry;
    // 8 players = 2 squads of 4
    for (uint32_t id = 1; id <= 8; ++id) {
        MatchParticipant p;
        p.participantId = id;
        p.displayName = "Soldier_" + std::to_string(id);
        p.status = ParticipantStatus::Alive;
        registry.RegisterParticipant(p);
    }

    modeMgr.SetupTeamsForParticipants(registry);
    auto& teamMgr = modeMgr.GetTeamManager();
    assert(teamMgr.GetTotalTeamCount() == 2);

    // Verify squad membership
    assert(teamMgr.IsTeammate(1, 4));
    assert(teamMgr.IsTeammate(5, 8));
    assert(teamMgr.IsEnemy(1, 5));

    // Eliminate 3 members of Squad 2 -> Squad 2 is still active
    registry.UpdateParticipantStatus(5, ParticipantStatus::Eliminated);
    registry.UpdateParticipantStatus(6, ParticipantStatus::Eliminated);
    registry.UpdateParticipantStatus(7, ParticipantStatus::Eliminated);

    assert(teamMgr.GetAliveTeamCount(registry) == 2);

    results::MatchOutcome outcome;
    uint32_t winnerId = 0;
    std::string winnerName;
    assert(!modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName));

    // Eliminate the last member of Squad 2
    registry.UpdateParticipantStatus(8, ParticipantStatus::Eliminated);
    bool decided = modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName);
    assert(decided);
    assert(outcome == results::MatchOutcome::Victory);
    assert(winnerId == 1); // Squad 1 won!

    std::cout << "  -> Squads 4-player team rules and last-team victory verified." << std::endl;
}

static void TestPracticeMode() {
    std::cout << "[TEST] 5. Practice Arena & Controls..." << std::endl;

    GameModeManager modeMgr;
    assert(modeMgr.SelectMode("practice"));
    const auto& mode = modeMgr.GetActiveMode();
    assert(mode.id == "practice");
    assert(!mode.competitiveResults);
    assert(mode.victoryCondition == "practice_freeplay");
    assert(!mode.stormEnabled);

    auto& practice = modeMgr.GetPracticeMode();
    assert(practice.IsActive());

    // Practice victory evaluation must never declare competitive victory
    MatchParticipantRegistry registry;
    MatchParticipant player;
    player.participantId = 1;
    player.status = ParticipantStatus::Alive;
    registry.RegisterParticipant(player);

    results::MatchOutcome outcome;
    uint32_t winnerId = 0;
    std::string winnerName;
    bool decided = modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName);
    assert(!decided); // Freeplay: never ends
    assert(outcome == results::MatchOutcome::InProgress);

    // Player health restoration test
    entities::Player localHero(1, "TestHero");
    localHero.GetHealth().ApplyDamage(40.0f);
    assert(localHero.GetHealth().GetCurrentHealth() < localHero.GetHealth().GetMaxHealth());

    practice.RestorePlayerHealth(&localHero);
    assert(localHero.GetHealth().GetCurrentHealth() == localHero.GetHealth().GetMaxHealth());

    // Player position reset test
    practice.SetSpawnPosition({ 250.0f, 350.0f });
    practice.ResetPlayerPosition(&localHero);
    assert(localHero.GetPosition().x == 250.0f && localHero.GetPosition().y == 350.0f);

    std::cout << "  -> Practice mode freeplay and interactive controls verified." << std::endl;
}

static void TestBotDifficultyProfiles() {
    std::cout << "[TEST] 6. Bot Difficulty Configuration..." << std::endl;

    BotMatchConfiguration botConfig;
    auto easyParams = botConfig.GetParams(BotDifficulty::Easy);
    auto normalParams = botConfig.GetParams(BotDifficulty::Normal);
    auto hardParams = botConfig.GetParams(BotDifficulty::Hard);

    // Easy should have shorter perception range and longer reaction delay than Hard
    assert(easyParams.perceptionRange < normalParams.perceptionRange);
    assert(normalParams.perceptionRange < hardParams.perceptionRange);

    assert(easyParams.reactionDelay > normalParams.reactionDelay);
    assert(normalParams.reactionDelay > hardParams.reactionDelay);

    assert(easyParams.attackCooldownMultiplier > hardParams.attackCooldownMultiplier);
    assert(easyParams.aggressionWeight < hardParams.aggressionWeight);

    std::cout << "  -> Bot difficulty parameter hierarchy (Easy/Normal/Hard) verified." << std::endl;
}

static void TestCombatFriendlyFireFiltering() {
    std::cout << "[TEST] 7. Combat Friendly Fire Integration..." << std::endl;

    GameModeManager modeMgr;
    modeMgr.SelectMode("duos");

    MatchParticipantRegistry registry;
    MatchParticipant p1; p1.participantId = 1; p1.entityId = 10; p1.status = ParticipantStatus::Alive;
    MatchParticipant p2; p2.participantId = 2; p2.entityId = 20; p2.status = ParticipantStatus::Alive;
    MatchParticipant p3; p3.participantId = 3; p3.entityId = 30; p3.status = ParticipantStatus::Alive;

    registry.RegisterParticipant(p1);
    registry.RegisterParticipant(p2);
    registry.RegisterParticipant(p3);

    modeMgr.SetupTeamsForParticipants(registry);

    CombatRules rules = CombatRules::CreateDefault();
    rules.allowPvP = true;
    rules.friendlyFire = false;
    rules.friendlyFireFilter = [&](CombatantId attackerId, CombatantId targetId) -> bool {
        const auto* attP = registry.FindParticipantByEntityId(attackerId);
        const auto* tarP = registry.FindParticipantByEntityId(targetId);
        if (!attP || !tarP) return true;
        return modeMgr.CanDamage(attP->participantId, tarP->participantId);
    };

    // Attacker 10 and Target 20 are teammates -> Cannot target!
    assert(!rules.CanTarget(10, 20, Faction::Players, Faction::Players));

    // Attacker 10 and Target 30 are on different squads -> Can target!
    assert(rules.CanTarget(10, 30, Faction::Players, Faction::Players));

    std::cout << "  -> CombatRules friendlyFireFilter callback integration verified." << std::endl;
}

static void TestTeamSpawnClustering() {
    std::cout << "[TEST] 8. Team Spawn Clustering..." << std::endl;

    MatchSpawner spawner;
    // Add 8 custom spawn points across two distant clusters (Cluster A at x=100, Cluster B at x=1000)
    spawner.AddCustomSpawnPoint({ 100.0f, 100.0f }, "ClusterA");
    spawner.AddCustomSpawnPoint({ 120.0f, 100.0f }, "ClusterA");
    spawner.AddCustomSpawnPoint({ 100.0f, 120.0f }, "ClusterA");
    spawner.AddCustomSpawnPoint({ 120.0f, 120.0f }, "ClusterA");

    spawner.AddCustomSpawnPoint({ 1000.0f, 1000.0f }, "ClusterB");
    spawner.AddCustomSpawnPoint({ 1020.0f, 1000.0f }, "ClusterB");
    spawner.AddCustomSpawnPoint({ 1000.0f, 1020.0f }, "ClusterB");
    spawner.AddCustomSpawnPoint({ 1020.0f, 1020.0f }, "ClusterB");

    std::vector<MatchParticipant> participants(4);
    // Team 1: 0 & 1
    participants[0].participantId = 1; participants[0].teamId = 1;
    participants[1].participantId = 2; participants[1].teamId = 1;

    // Team 2: 2 & 3
    participants[2].participantId = 3; participants[2].teamId = 2;
    participants[3].participantId = 4; participants[3].teamId = 2;

    bool assigned = spawner.AssignTeamSpawnPoints(participants, 42);
    assert(assigned);

    // Teammates in Team 1 must spawn close to each other
    float dx1 = participants[0].spawnPosition.x - participants[1].spawnPosition.x;
    float dy1 = participants[0].spawnPosition.y - participants[1].spawnPosition.y;
    float dist1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    assert(dist1 <= 50.0f);

    // Teammates in Team 2 must spawn close to each other
    float dx2 = participants[2].spawnPosition.x - participants[3].spawnPosition.x;
    float dy2 = participants[2].spawnPosition.y - participants[3].spawnPosition.y;
    float dist2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
    assert(dist2 <= 50.0f);

    std::cout << "  -> Team member spatial clustering at spawn verified." << std::endl;
}

static void TestOfflineBotLifecycleIntegration() {
    std::cout << "[TEST] 9. Offline Bot Match & Storm Reaction..." << std::endl;

    GameModeManager modeMgr;
    modeMgr.SelectMode("offline_bots");
    modeMgr.GetBotConfig().SetBotCount(2);

    MatchParticipantRegistry registry;
    MatchParticipant human;
    human.participantId = 1;
    human.entityId = 1;
    human.displayName = "LocalHuman";
    human.type = ParticipantType::LocalHuman;
    human.status = ParticipantStatus::Alive;
    registry.RegisterParticipant(human);

    MatchParticipant bot1;
    bot1.participantId = 101;
    bot1.entityId = 101;
    bot1.displayName = "Bot_1";
    bot1.type = ParticipantType::AI;
    bot1.status = ParticipantStatus::Alive;
    registry.RegisterParticipant(bot1);

    modeMgr.SetupTeamsForParticipants(registry);

    // Simulate bot outside safe zone taking storm damage
    storm::SafeZone safeZone{ { 500.0f, 500.0f }, 100.0f }; // Safe zone at (500,500) r=100
    storm::StormDamageSystem stormDmg;

    combat::HealthComponent botHealth(20.0f, 20.0f);
    Vector2 botPosOutside{ 1000.0f, 1000.0f };

    // Apply 5 seconds of 10 DPS storm damage -> lethal!
    bool dmgApplied = stormDmg.Update(5.0f, safeZone, 10.0f, botPosOutside, botHealth);
    assert(dmgApplied);
    assert(botHealth.IsDead());

    // Eliminate bot
    registry.UpdateParticipantStatus(101, ParticipantStatus::Eliminated);

    results::MatchOutcome outcome;
    uint32_t winnerId = 0;
    std::string winnerName;
    bool decided = modeMgr.EvaluateVictory(registry, true, outcome, winnerId, winnerName);
    assert(decided);
    assert(outcome == results::MatchOutcome::Victory);
    assert(winnerId == 1);

    std::cout << "  -> Offline bot storm attrition and match victory resolution verified." << std::endl;
}

static void TestEndToEndModeSwitchingScenario() {
    std::cout << "[TEST] 10. End-to-End Mode Switching Scenario..." << std::endl;

    GameModeManager modeMgr;

    // 1. Solo
    assert(modeMgr.SelectMode("solo"));
    assert(modeMgr.GenerateMatchConfiguration().victoryCondition == "last_survivor");
    modeMgr.Reset();

    // 2. Duos
    assert(modeMgr.SelectMode("duos"));
    assert(modeMgr.GetActiveMode().teamSize == 2);
    modeMgr.Reset();

    // 3. Squads
    assert(modeMgr.SelectMode("squads"));
    assert(modeMgr.GetActiveMode().teamSize == 4);
    modeMgr.Reset();

    // 4. Practice
    assert(modeMgr.SelectMode("practice"));
    assert(!modeMgr.GetActiveMode().competitiveResults);
    assert(modeMgr.GetPracticeMode().IsActive());
    modeMgr.Reset();
    assert(!modeMgr.GetPracticeMode().IsActive());

    // 5. Offline Bots
    assert(modeMgr.SelectMode("offline_bots"));
    assert(modeMgr.GetActiveMode().botsAllowed);
    modeMgr.Reset();

    std::cout << "  -> 5 sequential mode transitions with clean state resets verified." << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 34: GAME MODES TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestGameModeRegistry();
    TestSoloMode();
    TestDuosMode();
    TestSquadsMode();
    TestPracticeMode();
    TestBotDifficultyProfiles();
    TestCombatFriendlyFireFiltering();
    TestTeamSpawnClustering();
    TestOfflineBotLifecycleIntegration();
    TestEndToEndModeSwitchingScenario();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 PHASE 34 GAME MODE TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
