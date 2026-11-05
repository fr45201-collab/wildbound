#include "../../src/combat/CombatSystem.hpp"
#include "../../src/combat/HealthComponent.hpp"
#include "../../src/combat/DamageSystem.hpp"
#include "../../src/combat/AttackSystem.hpp"
#include "../../src/combat/HitDetection.hpp"
#include "../../src/combat/CombatRules.hpp"
#include "../../src/combat/DeathHandler.hpp"
#include "../../src/entities/Player.hpp"
#include "../../src/entities/Enemy.hpp"
#include "../../src/entities/EnemyConfig.hpp"
#include "../../src/world/EnemySpawner.hpp"
#include "../../src/world/World.hpp"
#include "../../src/graphics/animation/Direction.hpp"
#include "../../src/scripting/LuaManager.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>

using wildbound::core::Logger;
using namespace wildbound::combat;
using namespace wildbound::entities;
using namespace wildbound::world;
using namespace wildbound::resources;
using namespace wildbound::scripting;
using namespace wildbound::graphics::animation;
namespace graphics = wildbound::graphics;
namespace combat = wildbound::combat;
namespace entities = wildbound::entities;
namespace world = wildbound::world;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

static bool ApproxEqual(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) <= epsilon;
}

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound Combat & AI System Tests (Phase 5) ===" << std::endl;

    // Headless raylib context
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundCombatTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");
    assetMgr.LoadSpriteSheetMetadata("characters/player/player_spritesheet.json");
    assetMgr.LoadAnimationMetadata("characters/player/player_animations.json");
    assetMgr.LoadSpriteSheetMetadata("characters/enemies/goblin/goblin_spritesheet.json");
    assetMgr.LoadAnimationMetadata("characters/enemies/goblin/goblin_animations.json");

    // ========================================================================
    // Group 1: Combatant & Shared Interface Tests
    // ========================================================================
    {
        std::cout << "[TEST] 1. Combatant initialization & shared interface..." << std::endl;
        Player player(1, "PlayerHero");
        Enemy enemy(100, "GoblinScout", "goblin");

        // Verify shared Combatant pointer polymorphism
        Combatant* cPlayer = &player;
        Combatant* cEnemy = &enemy;

        TEST_ASSERT(cPlayer->GetCombatantId() == 1, "Player ID should be 1");
        TEST_ASSERT(cEnemy->GetCombatantId() == 100, "Enemy ID should be 100");
        TEST_ASSERT(cPlayer->GetCombatantType() == CombatantType::Player, "Player type should match");
        TEST_ASSERT(cEnemy->GetCombatantType() == CombatantType::Enemy, "Enemy type should match");
        TEST_ASSERT(cPlayer->GetFaction() == Faction::Players, "Player faction should be Players");
        TEST_ASSERT(cEnemy->GetFaction() == Faction::Monsters, "Enemy faction should be Monsters");
        TEST_ASSERT(cPlayer->IsAlive(), "Player should be alive initially");
        TEST_ASSERT(cEnemy->IsAlive(), "Enemy should be alive initially");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 2: Health Component Tests
    // ========================================================================
    {
        std::cout << "[TEST] 2. HealthComponent mechanics & validation..." << std::endl;
        HealthComponent hp(100.0f, 100.0f, false, 0.0f);
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 100.0f), "Initial HP should be 100");
        TEST_ASSERT(ApproxEqual(hp.GetHealthPercent(), 1.0f), "Health percent should be 1.0");

        // Valid damage
        bool damaged = hp.ApplyDamage(30.0f);
        TEST_ASSERT(damaged, "Damage should apply successfully");
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 70.0f), "HP should be 70 after 30 dmg");

        // Negative damage rejection
        bool negDmg = hp.ApplyDamage(-15.0f);
        TEST_ASSERT(!negDmg, "Negative damage should be rejected");
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 70.0f), "HP should remain unchanged");

        // Healing
        bool healed = hp.ApplyHeal(20.0f);
        TEST_ASSERT(healed, "Heal should apply");
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 90.0f), "HP should be 90 after healing");

        // Healing overflow prevention
        hp.ApplyHeal(50.0f);
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 100.0f), "HP should not exceed max health (100)");

        // Lethal damage & zero health
        hp.ApplyDamage(150.0f);
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 0.0f), "HP must not become negative");
        TEST_ASSERT(hp.IsDead(), "Component should register dead");
        TEST_ASSERT(!hp.IsAlive(), "Component should not be alive");

        // Dead entity damage rejection & repeated death prevention
        bool postMortemDmg = hp.ApplyDamage(20.0f);
        TEST_ASSERT(!postMortemDmg, "Cannot damage dead entity");
        bool postMortemHeal = hp.ApplyHeal(20.0f);
        TEST_ASSERT(!postMortemHeal, "Cannot heal dead entity without reset");

        // Reset
        hp.Reset();
        TEST_ASSERT(hp.IsAlive(), "Health reset should revive entity");
        TEST_ASSERT(ApproxEqual(hp.GetCurrentHealth(), 100.0f), "HP should be reset to max");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 3: Damage System & Calculation Tests
    // ========================================================================
    {
        std::cout << "[TEST] 3. Damage calculations, types & modifiers..." << std::endl;
        DamageSystem dmgSys;
        Player target(2, "Dummy");

        // Physical damage calculation
        DamageRequest reqPhys;
        reqPhys.sourceId = 1;
        reqPhys.targetId = 2;
        reqPhys.damageType = DamageType::Physical;
        reqPhys.baseDamage = 40.0f;
        reqPhys.damageMultiplier = 1.25f;

        DamageCalculationResult resPhys = dmgSys.CalculateDamage(reqPhys, &target);
        TEST_ASSERT(resPhys.valid, "Damage calculation should be valid");
        TEST_ASSERT(ApproxEqual(resPhys.finalDamage, 50.0f), "40 * 1.25 should equal 50.0");
        TEST_ASSERT(resPhys.damageType == DamageType::Physical, "Damage type should be Physical");

        // Magical damage
        DamageRequest reqMag;
        reqMag.sourceId = 1;
        reqMag.targetId = 2;
        reqMag.damageType = DamageType::Magical;
        reqMag.baseDamage = 30.0f;
        DamageCalculationResult resMag = dmgSys.CalculateDamage(reqMag, &target);
        TEST_ASSERT(resMag.damageType == DamageType::Magical, "Damage type should be Magical");

        // Environmental damage
        DamageRequest reqEnv;
        reqEnv.sourceId = 0;
        reqEnv.targetId = 2;
        reqEnv.damageType = DamageType::Environmental;
        reqEnv.baseDamage = 15.0f;
        DamageCalculationResult resEnv = dmgSys.CalculateDamage(reqEnv, &target);
        TEST_ASSERT(resEnv.damageType == DamageType::Environmental, "Damage type should be Environmental");

        // Invalid base damage rejection
        DamageRequest reqInvalid;
        reqInvalid.baseDamage = -10.0f;
        DamageCalculationResult resInv = dmgSys.CalculateDamage(reqInvalid, &target);
        TEST_ASSERT(!resInv.valid, "Negative base damage calculation must be invalid");

        // Critical hit multiplier
        DamageRequest reqCrit;
        reqCrit.baseDamage = 20.0f;
        reqCrit.criticalChance = 1.0f; // 100% crit
        reqCrit.criticalMultiplier = 2.0f;
        DamageCalculationResult resCrit = dmgSys.CalculateDamage(reqCrit, &target);
        TEST_ASSERT(resCrit.isCritical, "Critical hit should be triggered");
        TEST_ASSERT(ApproxEqual(resCrit.finalDamage, 40.0f), "Critical damage should be 20 * 2 = 40");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 4: Attack System & Cooldown Tests
    // ========================================================================
    {
        std::cout << "[TEST] 4. AttackSystem cooldown & execution..." << std::endl;
        AttackSystem atkSys;
        AttackDefinition slash;
        slash.id = "slash";
        slash.baseDamage = 25.0f;
        slash.cooldown = 0.5f;
        slash.range = 32.0f;
        atkSys.RegisterAttack(slash);

        TEST_ASSERT(atkSys.CanAttack("slash"), "Attack should be ready initially");
        TEST_ASSERT(!atkSys.CanAttack("non_existent"), "Unregistered attack cannot be executed");

        bool executed = atkSys.ExecuteAttack("slash");
        TEST_ASSERT(executed, "Execute attack should succeed");
        TEST_ASSERT(!atkSys.CanAttack("slash"), "Attack cannot be executed while on cooldown");
        TEST_ASSERT(ApproxEqual(atkSys.GetCooldownRemaining(), 0.5f), "Cooldown should be 0.5s");

        // Advance time by 0.3s (still cooling down)
        atkSys.Update(0.3f);
        TEST_ASSERT(!atkSys.CanAttack("slash"), "Attack still cooling down after 0.3s");
        TEST_ASSERT(ApproxEqual(atkSys.GetCooldownRemaining(), 0.2f), "Cooldown should be 0.2s");

        // Advance time by another 0.25s (cooldown expires)
        atkSys.Update(0.25f);
        TEST_ASSERT(atkSys.CanAttack("slash"), "Attack should be ready after cooldown expires");
        TEST_ASSERT(ApproxEqual(atkSys.GetCooldownRemaining(), 0.0f), "Cooldown should be 0");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 5: Directional Hitboxes & Hit Detection
    // ========================================================================
    {
        std::cout << "[TEST] 5. Directional attack hitboxes & AABB detection..." << std::endl;
        AttackDefinition atk;
        atk.hitboxWidth = 30.0f;
        atk.hitboxHeight = 20.0f;
        atk.forwardOffset = 15.0f;

        Vector2 pos{ 100.0f, 100.0f };

        // Test Downwards Hitbox
        Rectangle boxDown = HitDetection::ComputeAttackHitbox(pos, graphics::animation::Direction::Down, atk);
        TEST_ASSERT(ApproxEqual(boxDown.x, 85.0f), "Down hitbox X should be centered at 85");
        TEST_ASSERT(boxDown.y >= 100.0f, "Down hitbox Y should be in front of attacker");

        // Test Upwards Hitbox
        Rectangle boxUp = HitDetection::ComputeAttackHitbox(pos, graphics::animation::Direction::Up, atk);
        TEST_ASSERT(ApproxEqual(boxUp.x, 85.0f), "Up hitbox X should be centered at 85");
        TEST_ASSERT(boxUp.y < 100.0f, "Up hitbox Y should be behind/above attacker");

        // Test Rightwards Hitbox
        Rectangle boxRight = HitDetection::ComputeAttackHitbox(pos, graphics::animation::Direction::Right, atk);
        TEST_ASSERT(boxRight.x >= 115.0f, "Right hitbox X should extend to the right");

        // Intersection test
        Rectangle targetHurtbox{ 120.0f, 60.0f, 32.0f, 48.0f };
        bool hitsRight = HitDetection::Intersects(boxRight, targetHurtbox);
        TEST_ASSERT(hitsRight, "Right attack hitbox must intersect target positioned to the right");

        Rectangle farTarget{ 300.0f, 300.0f, 32.0f, 48.0f };
        bool hitsFar = HitDetection::Intersects(boxRight, farTarget);
        TEST_ASSERT(!hitsFar, "Attack must not hit distant target outside range");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 6: Unified CombatSystem Execution & Rules
    // ========================================================================
    {
        std::cout << "[TEST] 6. Unified CombatSystem attack resolution & faction rules..." << std::endl;
        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        Player player(1, "Hero");
        player.SetPosition(Vector2{ 200.0f, 200.0f });

        Enemy goblin(2, "Goblin1", "goblin");
        goblin.SetPosition(Vector2{ 225.0f, 200.0f }); // 25 px to the right (within range)

        combatSys.RegisterCombatant(&player);
        combatSys.RegisterCombatant(&goblin);

        float initialGoblinHp = goblin.GetHealth().GetCurrentHealth();

        // Player attacks Goblin (Right direction)
        bool attackProcessed = combatSys.ProcessAttack(&player, "slash", graphics::animation::Direction::Right);
        TEST_ASSERT(attackProcessed, "ProcessAttack should succeed");
        TEST_ASSERT(goblin.GetHealth().GetCurrentHealth() < initialGoblinHp, "Goblin must take damage from player attack");

        // Attacking again immediately should be blocked by cooldown
        bool rapidAttack = combatSys.ProcessAttack(&player, "slash", graphics::animation::Direction::Right);
        TEST_ASSERT(!rapidAttack, "Rapid attack must be blocked by cooldown");

        // Dead attacker cannot attack
        player.GetHealth().ApplyDamage(500.0f); // Kill player
        TEST_ASSERT(!player.IsAlive(), "Player should be dead");
        bool deadAttack = combatSys.ProcessAttack(&player, "slash", graphics::animation::Direction::Right);
        TEST_ASSERT(!deadAttack, "Dead player cannot initiate attacks");

        // Revive player for next tests
        player.GetHealth().Reset();
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 7: Friendly Fire & Target Filtering
    // ========================================================================
    {
        std::cout << "[TEST] 7. Friendly fire & faction filtering..." << std::endl;
        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        Player player1(1, "P1");
        player1.SetPosition(Vector2{ 100.0f, 100.0f });
        Player player2(2, "P2");
        player2.SetPosition(Vector2{ 120.0f, 100.0f }); // Beside player1

        combatSys.RegisterCombatant(&player1);
        combatSys.RegisterCombatant(&player2);

        float p2HpBefore = player2.GetHealth().GetCurrentHealth();

        // Player1 attacks right into Player2 (Friendly fire disabled)
        combatSys.ProcessAttack(&player1, "slash", graphics::animation::Direction::Right);
        TEST_ASSERT(ApproxEqual(player2.GetHealth().GetCurrentHealth(), p2HpBefore), "Player2 must NOT take damage when PvP is disabled");

        // Enable PvP in rules
        combatSys.GetRules().allowPvP = true;
        player1.GetAttackSystem().ResetCooldown();
        combatSys.ProcessAttack(&player1, "slash", graphics::animation::Direction::Right);
        TEST_ASSERT(player2.GetHealth().GetCurrentHealth() < p2HpBefore, "Player2 takes damage when PvP is enabled");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 8: Enemy Entity & Config Loading
    // ========================================================================
    {
        std::cout << "[TEST] 8. Enemy entity & configuration loading..." << std::endl;
        EnemyConfig cfg = EnemyConfig::LoadFromFile("data/characters/enemies/goblin.json");
        TEST_ASSERT(cfg.id == "goblin", "Config ID should be 'goblin'");
        TEST_ASSERT(ApproxEqual(cfg.health.maxHealth, 50.0f), "Goblin max HP should be 50");
        TEST_ASSERT(ApproxEqual(cfg.stats.walkSpeed, 65.0f), "Goblin walk speed should be 65");
        TEST_ASSERT(ApproxEqual(cfg.ai.detectionRadius, 140.0f), "Detection radius should be 140");

        Enemy goblin(5, "Scout", "goblin");
        goblin.ApplyConfig(cfg);
        goblin.Spawn(Vector2{ 300.0f, 300.0f });

        TEST_ASSERT(goblin.GetCombatantId() == 5, "Goblin ID should be 5");
        TEST_ASSERT(ApproxEqual(goblin.GetHealth().GetMaxHealth(), 50.0f), "HP should match config");
        TEST_ASSERT(goblin.GetAIState() == AIState::Idle, "Initial AI state should be Idle");
        TEST_ASSERT(ApproxEqual(goblin.GetPosition().x, 300.0f), "Spawn X should be 300");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 9: Enemy AI State Machine (Chase, Attack, Retreat)
    // ========================================================================
    {
        std::cout << "[TEST] 9. Enemy AI decision state transitions..." << std::endl;
        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        Enemy goblin(10, "GoblinFighter", "goblin");
        EnemyConfig cfg = EnemyConfig::LoadFromFile("data/characters/enemies/goblin.json");
        goblin.ApplyConfig(cfg);
        goblin.Spawn(Vector2{ 200.0f, 200.0f });

        Player player(1, "PlayerHero");
        player.Spawn(Vector2{ 280.0f, 200.0f }); // 80px away (within 140px detection radius)

        combatSys.RegisterCombatant(&goblin);
        combatSys.RegisterCombatant(&player);

        World world; // Empty world for testing

        // Update goblin AI (should transition from Idle -> Chase)
        goblin.UpdateWithWorld(0.3f, world, combatSys, assetMgr, nullptr);
        TEST_ASSERT(goblin.GetAIState() == AIState::Chase, "Goblin should enter Chase state when target in detection radius");
        TEST_ASSERT(goblin.GetVelocity().x > 0.0f, "Goblin should move towards player (positive X)");

        // Move player to within attack radius (25 px)
        player.SetPosition(Vector2{ goblin.GetPosition().x + 25.0f, 200.0f });
        goblin.UpdateWithWorld(0.3f, world, combatSys, assetMgr, nullptr);
        TEST_ASSERT(goblin.GetAIState() == AIState::Attack, "Goblin should enter Attack state within attack radius");

        // Damage goblin below retreat threshold (<= 20% of 50 HP = 10 HP)
        goblin.GetHealth().ApplyDamage(42.0f); // Remaining HP = 8.0f (< 10.0f)
        goblin.UpdateWithWorld(0.3f, world, combatSys, assetMgr, nullptr);
        TEST_ASSERT(goblin.GetAIState() == AIState::Retreat, "Goblin should enter Retreat state when health drops below threshold");
        TEST_ASSERT(goblin.GetVelocity().x < 0.0f, "Goblin should flee away from player (negative X)");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 10: Lua Script AI Execution & Safety Handling
    // ========================================================================
    {
        std::cout << "[TEST] 10. Lua AI scripting integration & fallback safety..." << std::endl;
        LuaManager luaMgr;
        bool luaInit = luaMgr.Initialize();
        TEST_ASSERT(luaInit, "LuaManager should initialize");

        bool scriptLoaded = luaMgr.LoadScriptFile("scripts/gameplay/ai/enemy_base.lua");
        TEST_ASSERT(scriptLoaded, "enemy_base.lua should load cleanly");

        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        Enemy goblin(20, "LuaGoblin", "goblin");
        EnemyConfig cfg = EnemyConfig::LoadFromFile("data/characters/enemies/goblin.json");
        goblin.ApplyConfig(cfg);
        goblin.Spawn(Vector2{ 100.0f, 100.0f });

        Player player(1, "PlayerTarget");
        player.Spawn(Vector2{ 150.0f, 100.0f }); // 50px away

        combatSys.RegisterCombatant(&goblin);
        combatSys.RegisterCombatant(&player);

        World world;
        goblin.UpdateWithWorld(0.3f, world, combatSys, assetMgr, &luaMgr);
        TEST_ASSERT(goblin.GetAIState() == AIState::Chase, "Lua script should set state to CHASE");

        // Test robustness against invalid Lua code / corrupt function
        luaMgr.ExecuteString("wildbound.ai.update_enemy = function() error('Simulated AI crash') end");
        goblin.UpdateWithWorld(0.3f, world, combatSys, assetMgr, &luaMgr);
        // Engine must NOT crash; C++ fallback takes over safely!
        TEST_ASSERT(goblin.IsAlive(), "Enemy remains functional despite Lua error");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 11: Enemy Death & Deferred Destruction
    // ========================================================================
    {
        std::cout << "[TEST] 11. Enemy death sequence & deferred lifecycle removal..." << std::endl;
        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        Enemy enemy(30, "DyingEnemy", "goblin");
        enemy.Spawn(Vector2{ 200.0f, 200.0f });
        combatSys.RegisterCombatant(&enemy);

        // Apply lethal damage
        combat::DamageRequest lethal;
        lethal.baseDamage = 100.0f;
        combatSys.GetDamageSystem().ApplyDamage(lethal, &enemy, &combatSys.GetEvents());

        TEST_ASSERT(enemy.GetHealth().IsDead(), "Enemy should be dead");
        TEST_ASSERT(enemy.GetAIState() == AIState::Dead, "Enemy AI state should be Dead");
        TEST_ASSERT(ApproxEqual(enemy.GetVelocity().x, 0.0f) && ApproxEqual(enemy.GetVelocity().y, 0.0f), "Dead enemy must stop moving");
        TEST_ASSERT(!enemy.IsPendingDestruction(), "Enemy should not be immediately destroyed (death anim timer active)");

        // Advance time through death animation (1.5 seconds)
        enemy.Update(0.8f);
        TEST_ASSERT(!enemy.IsPendingDestruction(), "Still playing death animation after 0.8s");
        enemy.Update(0.8f); // 1.6s total
        TEST_ASSERT(enemy.IsPendingDestruction(), "Enemy should be marked for deferred destruction after 1.5s");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 12: Enemy Spawner Configuration & Constraints
    // ========================================================================
    {
        std::cout << "[TEST] 12. EnemySpawner configuration & active count limits..." << std::endl;
        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        EnemySpawner spawner;
        spawner.Initialize("data/spawning/enemy_spawning.json", assetMgr);

        TEST_ASSERT(spawner.IsEnabled(), "Spawner should be enabled by default");
        TEST_ASSERT(spawner.GetConfig().maxActiveEnemies == 6, "Max active enemies should be 6");

        // Spawn explicit enemies
        auto* e1 = spawner.SpawnEnemy("goblin", Vector2{ 500.0f, 500.0f }, combatSys);
        auto* e2 = spawner.SpawnEnemy("skeleton", Vector2{ 550.0f, 500.0f }, combatSys);

        TEST_ASSERT(e1 != nullptr && e2 != nullptr, "Explicit spawns should succeed");
        TEST_ASSERT(spawner.GetActiveEnemyCount() == 2, "Active count should be 2");
        TEST_ASSERT(combatSys.GetCombatant(e1->GetCombatantId()) != nullptr, "Spawned enemy registered in CombatSystem");

        // Clean up
        spawner.ClearAllEnemies(combatSys);
        TEST_ASSERT(spawner.GetActiveEnemyCount() == 0, "Active count should be 0 after clear");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 13: Collision-Aware Spawning Validation
    // ========================================================================
    {
        std::cout << "[TEST] 13. Collision-aware spawn location validation..." << std::endl;
        World world;
        world.LoadMap("maps/forest/forest_01.json", assetMgr);

        EnemySpawner spawner;
        spawner.Initialize("data/spawning/enemy_spawning.json", assetMgr);

        // Position inside solid border (e.g. x=10, y=10)
        Vector2 solidTilePos{ 10.0f, 10.0f };
        bool validSolid = spawner.IsValidSpawnLocation(solidTilePos, world);
        TEST_ASSERT(!validSolid, "Spawn inside solid border must be invalid");

        // Position outside map boundaries
        Vector2 offMapPos{ -100.0f, 500.0f };
        bool validOffMap = spawner.IsValidSpawnLocation(offMapPos, world);
        TEST_ASSERT(!validOffMap, "Spawn outside map bounds must be invalid");

        // Valid clearing position (e.g. courtyard or pathway)
        Vector2 clearPos{ 256.0f, 960.0f }; // Player spawn area
        bool validClear = spawner.IsValidSpawnLocation(clearPos, world);
        TEST_ASSERT(validClear, "Spawn in open clearing must be valid");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 14: Enemy Movement & Obstacle Collision
    // ========================================================================
    {
        std::cout << "[TEST] 14. Enemy movement & wall sliding..." << std::endl;
        World world;
        world.LoadMap("maps/forest/forest_01.json", assetMgr);

        Enemy enemy(40, "SlidingGoblin", "goblin");
        EnemyConfig cfg = EnemyConfig::CreateDefaultGoblin();
        enemy.ApplyConfig(cfg);

        // Place adjacent to cabin wall (X = 1850, Y = 410)
        enemy.Spawn(Vector2{ 1850.0f, 410.0f });

        // Move diagonally into the wall (+20 X, +30 Y)
        enemy.ResolveMovement(Vector2{ 20.0f, 30.0f }, world);

        // X should be blocked by cabin wall (< 1865), Y slides freely downwards (410 + 30 = 440)
        TEST_ASSERT(enemy.GetPosition().x < 1865.0f, "Enemy X movement must be blocked at obstacle");
        TEST_ASSERT(ApproxEqual(enemy.GetPosition().y, 440.0f), "Enemy Y movement must slide freely downwards");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Group 15: Full Combat Engagement Flow (Player vs Goblin to Death)
    // ========================================================================
    {
        std::cout << "[TEST] 15. End-to-end combat engagement (Player kills Goblin)..." << std::endl;
        CombatSystem combatSys;
        combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

        Player player(1, "Hero");
        player.Spawn(Vector2{ 500.0f, 500.0f });
        player.SetFacing(graphics::animation::Direction::Right);

        Enemy goblin(99, "BossGoblin", "goblin");
        EnemyConfig cfg = EnemyConfig::LoadFromFile("data/characters/enemies/goblin.json");
        goblin.ApplyConfig(cfg); // 50 HP
        goblin.Spawn(Vector2{ 525.0f, 500.0f }); // 25px away

        combatSys.RegisterCombatant(&player);
        combatSys.RegisterCombatant(&goblin);

        int damageEventsReceived = 0;
        int deathEventsReceived = 0;

        combatSys.GetEvents().SubscribeDamage([&](const DamageEvent&) { damageEventsReceived++; });
        combatSys.GetEvents().SubscribeDeath([&](const DeathEvent&) { deathEventsReceived++; });

        // Strike 1: 25 damage -> 25 HP remaining
        player.TryAttack(combatSys, "slash");
        TEST_ASSERT(ApproxEqual(goblin.GetHealth().GetCurrentHealth(), 25.0f), "Goblin HP should be 25 after 1st hit");
        TEST_ASSERT(damageEventsReceived == 1, "Damage event should be dispatched");

        // Reset cooldown & Strike 2 (Lethal strike: 25 damage -> 0 HP)
        player.GetAttackSystem().ResetCooldown();
        player.TryAttack(combatSys, "slash");

        TEST_ASSERT(goblin.GetHealth().IsDead(), "Goblin should be dead after 2nd hit");
        TEST_ASSERT(damageEventsReceived == 2, "Second damage event dispatched");
        TEST_ASSERT(deathEventsReceived == 1, "Death event dispatched exactly once");
        TEST_ASSERT(goblin.GetAIState() == AIState::Dead, "Goblin AI state must be DEAD");

        // Further strikes against dead goblin must not dispatch damage or death events
        player.GetAttackSystem().ResetCooldown();
        player.TryAttack(combatSys, "slash");
        TEST_ASSERT(damageEventsReceived == 2, "Dead goblin cannot receive further damage events");
        TEST_ASSERT(deathEventsReceived == 1, "No duplicate death events");
        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "\n========================================================" << std::endl;
    std::cout << "All Wildbound Combat & AI tests passed! (15/15)" << std::endl;
    std::cout << "========================================================" << std::endl;

    CloseWindow();
    return 0;
}

