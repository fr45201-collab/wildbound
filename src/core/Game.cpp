#include "Game.hpp"
#include "Logger.hpp"
#include "../scripting/LuaManager.hpp"
#include "../ui/DevUI.hpp"
#include "../ui/UIManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../world/WorldItem.hpp"
#include "../world/LootContainer.hpp"
#include "../input/InputManager.hpp"
#include "../interaction/InteractionManager.hpp"
#include "../customization/CharacterCustomizationManager.hpp"
#include "../cosmetics/CosmeticsStoreManager.hpp"
#include "../ai/AIManager.hpp"
#include "../storm/StormManager.hpp"

#include <raylib.h>
#include <algorithm>

namespace wildbound::core {

Game::Game(const EngineConfig& initialConfig)
    : m_config(initialConfig)
    , m_renderer(std::make_unique<graphics::Renderer>())
    , m_camera(std::make_unique<graphics::Camera2DSystem>(m_config.internalWidth, m_config.internalHeight))
    , m_testScene(std::make_unique<graphics::TestScene>())
    , m_assetManager(std::make_unique<resources::AssetManager>())
    , m_world(std::make_unique<world::World>())
    , m_worldRenderer(std::make_unique<world::WorldRenderer>())
    , m_player(std::make_unique<entities::Player>(1, "Explorer"))
    , m_combatSystem(std::make_unique<combat::CombatSystem>())
    , m_enemySpawner(std::make_unique<world::EnemySpawner>())
    , m_buildingManager(std::make_unique<world::BuildingManager>())
    , m_biomeManager(std::make_unique<world::BiomeManager>())
    , m_worldManager(std::make_unique<world::WorldManager>())
    , m_weaponManager(std::make_unique<weapons::WeaponManager>())
    , m_itemManager(std::make_unique<items::ItemManager>())
    , m_audioSystem(std::make_unique<audio::AudioSystem>())
    , m_uiManager(std::make_unique<ui::UIManager>())
    , m_envAnimationSystem(std::make_unique<world::environment::EnvironmentalAnimationSystem>())
    , m_collisionManager(std::make_unique<physics::CollisionManager>())
    , m_collisionDebugRenderer(std::make_unique<physics::CollisionDebugRenderer>())
    , m_inputManager(std::make_unique<input::InputManager>())
    , m_interactionManager(std::make_unique<interaction::InteractionManager>())
    , m_customizationManager(std::make_unique<customization::CharacterCustomizationManager>())
    , m_cosmeticsStoreManager(std::make_unique<cosmetics::CosmeticsStoreManager>())
    , m_aiManager(std::make_unique<ai::AIManager>())
    , m_stormManager(std::make_unique<storm::StormManager>())
    , m_luaManager(std::make_unique<scripting::LuaManager>())
    , m_devUI(std::make_unique<ui::DevUI>())
{
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    Logger::Info("ENGINE", "Initialization started.");

    // 1. Initialize Lua runtime
    if (!m_luaManager->Initialize()) {
        Logger::Error("ENGINE", "Failed to initialize Lua scripting runtime.");
        return false;
    }

    // 2. Load engine configuration from Lua
    if (!m_luaManager->LoadEngineConfig("scripts/config/engine.lua", m_config)) {
        Logger::Warn("ENGINE", "Could not load engine.lua, continuing with default configuration.");
    }

    // 3. Initialize Renderer (Window + Virtual Canvas + Viewport)
    if (!m_renderer->Initialize(m_config)) {
        Logger::Error("ENGINE", "Failed to initialize Renderer subsystem.");
        return false;
    }

    // 4. Initialize Asset Management System
    if (!m_assetManager->Initialize("assets")) {
        Logger::Error("ENGINE", "Failed to initialize Asset Management System.");
        return false;
    }

    // Load registered spritesheet metadata and animations
    m_assetManager->LoadSpriteSheetMetadata("characters/player/player_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("ui/icons/item_icons.json");
    m_assetManager->LoadAnimationMetadata("characters/player/player_walk.json");
    m_assetManager->LoadAnimationMetadata("characters/player/player_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/enemies/goblin/goblin_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/enemies/goblin/goblin_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/enemies/skeleton/skeleton_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/enemies/skeleton/skeleton_animations.json");

    // Phase 9 Creature and Wildlife Assets (Hostile & Ambient)
    m_assetManager->LoadSpriteSheetMetadata("characters/enemies/wolf/wolf_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/enemies/wolf/wolf_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/enemies/wild_boar/wild_boar_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/enemies/wild_boar/wild_boar_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/enemies/bat/bat_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/enemies/bat/bat_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/wildlife/birds/small_bird/small_bird_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/wildlife/birds/small_bird/small_bird_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/wildlife/butterflies/forest_butterfly/forest_butterfly_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/wildlife/butterflies/forest_butterfly/forest_butterfly_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/wildlife/butterflies/blue_butterfly/blue_butterfly_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/wildlife/butterflies/blue_butterfly/blue_butterfly_animations.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/wildlife/butterflies/golden_butterfly/golden_butterfly_spritesheet.json");
    m_assetManager->LoadAnimationMetadata("characters/wildlife/butterflies/golden_butterfly/golden_butterfly_animations.json");

    // Load Modular Character Cosmetic Spritesheets (Phase 7)
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/skin/skin_default_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/skin/skin_tan_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/hair/hair_short_brown_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/hair/hair_ponytail_blonde_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/shirts/shirt_forest_tunic_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/shirts/shirt_adventurer_vest_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/pants/pants_traveler_trousers_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/pants/pants_scout_shorts_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/shoes/shoes_leather_boots_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/shoes/shoes_iron_greaves_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/hats/hat_ranger_hood_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/hats/hat_iron_helm_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/face/face_eyepatch_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/face/face_explorer_goggles_spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/cosmetics/back/back_traveler_backpack_spritesheet.json");

    // Phase 8 Production Cosmetics Collection (32 Designs)
    // Hair (5)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hair/hair_short_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hair/hair_long_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hair/hair_messy_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hair/hair_ponytail_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hair/hair_curly_001/spritesheet.json");

    // Shirts (5)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shirts/shirt_tunic_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shirts/shirt_leather_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shirts/shirt_armor_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shirts/shirt_traveler_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shirts/shirt_mage_001/spritesheet.json");

    // Pants (5)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/pants/pants_traveler_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/pants/pants_leather_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/pants/pants_armored_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/pants/pants_short_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/pants/pants_mage_001/spritesheet.json");

    // Shoes (3)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shoes/shoes_boots_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shoes/shoes_armored_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/shoes/shoes_light_001/spritesheet.json");

    // Hats (5)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hats/hat_adventurer_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hats/hat_hood_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hats/hat_helmet_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hats/hat_wizard_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/hats/hat_cap_001/spritesheet.json");

    // Goggles (3)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/goggles/goggles_adventure_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/goggles/goggles_steampunk_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/goggles/goggles_snow_001/spritesheet.json");

    // Masks (3)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/masks/mask_cloth_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/masks/mask_leather_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/masks/mask_mystic_001/spritesheet.json");

    // Backpacks (3)
    m_assetManager->LoadSpriteSheetMetadata("characters/player/backpacks/backpack_traveler_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/backpacks/backpack_leather_001/spritesheet.json");
    m_assetManager->LoadSpriteSheetMetadata("characters/player/backpacks/backpack_adventure_001/spritesheet.json");

    // 5. Initialize World and Load Forest_01 Map (Phase 3)
    if (!m_world->LoadMap("maps/forest/forest_01.json", *m_assetManager)) {
        Logger::Warn("ENGINE", "Could not load forest_01.json, attempting fallback to TMX format.");
        m_world->LoadMap("maps/forest/forest_01.tmx", *m_assetManager);
    }

    // Populate CollisionManager with static world geometry (Phase 20)
    if (m_collisionManager && m_world && m_world->IsLoaded()) {
        m_world->PopulateCollisionManager(*m_collisionManager);
    }

    // Initialize Village Building & Interior Manager (Phase 13)
    m_buildingManager->Initialize("data/buildings/village_buildings.json", "maps/forest/forest_01.json");

    // Initialize Biome Manager (Phase 14)
    m_biomeManager->Initialize("data/biomes");
    m_biomeManager->SetActiveBiomeId("forest");

    // Initialize Battle Royale World Manager (Phase 30)
    m_worldManager->Initialize("data/world");

    // Initialize Weapon Manager (Phase 15)
    m_weaponManager->Initialize("data/weapons/weapon_catalog.json");

    // Initialize Item Manager (Phase 16 & Phase 23)
    m_itemManager->Initialize("data/items/item_catalog.json");
    m_itemManager->RegisterDefaultAmmunition();

    // Initialize Audio System (Phase 18)
    if (m_audioSystem) {
        m_audioSystem->Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");
        m_audioSystem->LoadBiomeConfigs("data/audio/biome_audio.json");
        m_audioSystem->SetBiome("forest");
    }

    // Initialize Player-Facing UI Management System (Phase 18)
    if (m_uiManager) {
        m_uiManager->Initialize(*m_assetManager, m_audioSystem.get());
        m_uiManager->SetScreen(ui::UIScreen::None);
    }

    // 6. Initialize Combat System & AI Scripts (Phase 5)
    m_combatSystem->Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");
    m_luaManager->LoadScriptFile("scripts/gameplay/ai/enemy_base.lua");

    // 7. Initialize Player Entity & Spawn (Phase 4 & Phase 7)
    auto playerCfg = entities::CharacterConfig::LoadFromFile("data/characters/player.json");
    m_player->ApplyConfig(playerCfg);
    m_player->GetModularAppearance().LoadAppearance("data/characters/player_appearance.json", *m_assetManager);

    Vector2 spawnPos = m_world->GetSpawnPosition("player_spawn");
    m_player->Spawn(spawnPos);
    m_player->SetInputManager(m_inputManager.get());
    m_combatSystem->RegisterCombatant(m_player.get());

    // Register Player Collider with CollisionManager (Phase 20)
    if (m_collisionManager) {
        physics::Collider playerCol(
            0, m_player->GetId(), physics::CollisionLayer::Player,
            physics::GetDefaultCollisionMask(physics::CollisionLayer::Player),
            Vector2{ 16.0f, 12.0f }, Vector2{ 0.0f, 0.0f },
            physics::ColliderType::Physical, "player"
        );
        playerCol.UpdateWorldBounds(spawnPos);
        m_player->SetColliderId(m_collisionManager->RegisterCollider(playerCol));
    }

    // Phase 19/24: Populate Starter Inventory & Equipment
    if (m_itemManager) {
        m_player->GetInventory().AddItem("col_gold_coin_001", 50, *m_itemManager);
        m_player->GetInventory().AddItem("weapon_sword_iron_001", 1, *m_itemManager);
        m_player->GetInventory().AddItem("weapon_bow_hunting_001", 1, *m_itemManager);
        m_player->GetInventory().AddItem("arrow_iron_001", 30, *m_itemManager);
        m_player->GetInventory().AddItem("con_health_potion_small_001", 5, *m_itemManager);
        m_player->GetInventory().AddItem("con_bread_001", 3, *m_itemManager);
        m_player->GetInventory().AddItem("hat_iron_helm", 1, *m_itemManager);
        m_player->GetInventory().AddItem("shirt_armor_001", 1, *m_itemManager);

        m_player->GetEquipmentSystem().Equip("weapon_sword_iron_001", *m_itemManager, m_player.get(), m_weaponManager.get());
        m_player->GetEquipmentSystem().Equip("hat_iron_helm", *m_itemManager, m_player.get());
        m_player->GetEquipmentSystem().Equip("shirt_armor_001", *m_itemManager, m_player.get());
    }

    // Phase 25: Initialize Character Customization Manager
    if (m_customizationManager) {
        m_customizationManager->Initialize(*m_assetManager);
        m_customizationManager->SetPlayer(m_player.get());
    }

    // Phase 26: Initialize Cosmetics Store & Wardrobe System
    if (m_cosmeticsStoreManager && m_customizationManager) {
        m_cosmeticsStoreManager->Initialize(*m_customizationManager, m_audioSystem.get());
    }

    // Phase 19: Spawn Starter World Pickups & Loot Container in Forest
    if (m_world && m_world->IsLoaded()) {
        m_world->SpawnWorldItem("con_health_potion_small_001", Vector2{ spawnPos.x + 40.0f, spawnPos.y - 20.0f }, 2);
        m_world->SpawnWorldItem("con_apple_001", Vector2{ spawnPos.x + 60.0f, spawnPos.y + 10.0f }, 5);
        m_world->SpawnWorldItem("mat_iron_ore_001", Vector2{ spawnPos.x - 50.0f, spawnPos.y - 30.0f }, 3);
        m_world->SpawnWorldItem("mat_wood_001", Vector2{ spawnPos.x - 40.0f, spawnPos.y + 35.0f }, 4);

        auto* chest = m_world->SpawnLootContainer("starter_chest_01", Vector2{ spawnPos.x + 70.0f, spawnPos.y - 50.0f });
        if (chest && m_itemManager) {
            chest->GetInventory().AddItem("con_health_potion_small_001", 3, *m_itemManager);
            chest->GetInventory().AddItem("con_bread_001", 2, *m_itemManager);
            chest->GetInventory().AddItem("col_gold_coin_001", 25, *m_itemManager);
            chest->GetInventory().AddItem("col_ancient_coin_001", 1, *m_itemManager);
        }

        // Phase 22: Register Sample World Doors & Mechanisms
        if (m_interactionManager) {
            // 1. Village Outskirts Wooden Gate Door
            Vector2 doorPos{ spawnPos.x + 120.0f, spawnPos.y + 10.0f };
            interaction::Door villageGate("village_gate_01", "Village Outpost Gate", doorPos, Rectangle{ doorPos.x - 16.0f, doorPos.y - 24.0f, 32.0f, 48.0f });
            if (m_collisionManager) {
                physics::Collider col = physics::Collider::CreateStatic(
                    0, physics::CollisionLayer::Interactable, villageGate.GetBounds(), "door_village_gate_01");
                auto colId = m_collisionManager->RegisterCollider(col);
                villageGate.SetSolidColliderId(colId);
            }
            m_interactionManager->RegisterDoor(std::move(villageGate));

            // 2. Locked Ancient Treasury Door
            Vector2 lockedDoorPos{ spawnPos.x - 120.0f, spawnPos.y - 80.0f };
            interaction::Door treasuryDoor("treasury_door_01", "Ancient Crypt Door", lockedDoorPos,
                                          Rectangle{ lockedDoorPos.x - 16.0f, lockedDoorPos.y - 24.0f, 32.0f, 48.0f },
                                          interaction::DoorState::Locked, "spe_celestial_key_001");
            if (m_collisionManager) {
                physics::Collider col = physics::Collider::CreateStatic(
                    0, physics::CollisionLayer::Interactable, treasuryDoor.GetBounds(), "door_treasury_01");
                auto colId = m_collisionManager->RegisterCollider(col);
                treasuryDoor.SetSolidColliderId(colId);
            }
            m_interactionManager->RegisterDoor(std::move(treasuryDoor));

            // 3. Lever Mechanism
            Vector2 leverPos{ spawnPos.x + 90.0f, spawnPos.y + 40.0f };
            m_interactionManager->RegisterMechanism(interaction::Mechanism(
                "lever_gate_01", "Gate Winch Lever", leverPos, interaction::MechanismType::Lever
            ));

            // 4. Pressure Plate Trigger Mechanism
            Vector2 platePos{ spawnPos.x - 80.0f, spawnPos.y + 60.0f };
            m_interactionManager->RegisterMechanism(interaction::Mechanism(
                "plate_temple_01", "Temple Pressure Plate", platePos, interaction::MechanismType::PressurePlate, false, 0.5f, true
            ));
        }
    }

    // 8. Initialize Enemy Spawner (Phase 5)
    m_enemySpawner->Initialize("data/spawning/enemy_spawning.json", *m_assetManager);

    // Initialize AI & Navigation Subsystem (Phase 29)
    if (m_aiManager) {
        m_aiManager->Initialize();
        if (m_world && m_world->IsLoaded()) {
            m_aiManager->InitializeFromWorld(*m_world);
        }
    }

    // 9. Configure Camera system
    m_camera->SetVirtualDimensions(m_config.internalWidth, m_config.internalHeight);
    if (m_world->IsLoaded()) {
        m_camera->SetWorldBounds(m_world->GetWorldBounds());
    }
    m_camera->SetTarget(spawnPos);
    m_camera->SetFollowTarget(spawnPos);
    m_camera->SetZoomLimits(0.5f, 4.0f);
    m_camera->SetFollowSpeed(5.0f);
    m_camera->SetMode(graphics::CameraMode::Follow);

    // 10. Initialize Environmental Animation System (Phase 11 & 12)
    if (m_envAnimationSystem) {
        m_envAnimationSystem->Initialize(*m_assetManager);
        if (m_world && m_world->IsLoaded()) {
            m_envAnimationSystem->PopulateFromWorld(*m_world, *m_assetManager);
        }
    }

    // Phase 31: Initialize Corruption Storm & Safe Zone System
    if (m_stormManager) {
        Rectangle stormBounds = (m_worldManager && m_worldManager->GetPlayableBoundary().width > 0.0f)
            ? m_worldManager->GetPlayableBoundary()
            : (m_world && m_world->IsLoaded() ? m_world->GetWorldBounds() : Rectangle{ 768.0f, 768.0f, 8704.0f, 8704.0f });
        m_stormManager->Initialize("data/storm/storm_config.json", stormBounds);
    }

    // 11. Initialize DevUI (Dear ImGui + rlImGui)
    if (m_config.debugMode) {
        if (!m_devUI->Initialize()) {
            Logger::Warn("ENGINE", "Failed to initialize Developer ImGui overlay.");
        }
    }

    m_running = true;
    m_runningTime = 0.0f;
    Logger::Info("ENGINE", "Initialization completed.");
    return true;
}

void Game::Run() {
    if (!m_running) {
        Logger::Error("ENGINE", "Cannot run: Engine is not initialized.");
        return;
    }

    Logger::Info("ENGINE", "Entering main game loop.");

    constexpr float fixedDeltaTime = 1.0f / 60.0f;
    float accumulator = 0.0f;

    while (m_running && !m_renderer->IsWindowClosed()) {
        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }

        m_runningTime += frameTime;

        ProcessInput(frameTime);

        accumulator += frameTime;
        while (accumulator >= fixedDeltaTime) {
            FixedUpdate(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }

        Update(frameTime);
        Render(frameTime);
    }

    Logger::Info("ENGINE", "Exiting main game loop.");
}

void Game::ProcessInput(float dt) {
    if (m_inputManager) {
        // 1. Synchronize input context based on active UI screen
        if (m_uiManager) {
            auto scr = m_uiManager->GetCurrentScreen();
            if (scr == ui::UIScreen::MainMenu ||
                scr == ui::UIScreen::MatchResults || scr == ui::UIScreen::GameOver) {
                m_inputManager->SetContext(input::InputContextType::Menu);
            } else if (scr == ui::UIScreen::PauseMenu) {
                m_inputManager->SetContext(input::InputContextType::Paused);
            } else if (scr == ui::UIScreen::Inventory) {
                m_inputManager->SetContext(input::InputContextType::Inventory);
            } else if (scr == ui::UIScreen::Wardrobe || scr == ui::UIScreen::CharacterSelect) {
                m_inputManager->SetContext(input::InputContextType::Wardrobe);
            } else if (scr == ui::UIScreen::Settings) {
                m_inputManager->SetContext(input::InputContextType::Settings);
            } else {
                m_inputManager->SetContext(input::InputContextType::Gameplay);
            }
        }

        // 2. Poll hardware devices and update buffers
        m_inputManager->Update(dt, &m_renderer->GetViewport(), m_camera.get());
    }

    bool pausePressed = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::Pause)
                                       : IsKeyPressed(KEY_ESCAPE);
    if (pausePressed) {
        if (m_uiManager) {
            if (m_uiManager->GetCurrentScreen() == ui::UIScreen::None) {
                m_uiManager->SetScreen(ui::UIScreen::PauseMenu);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
                return;
            } else if (m_uiManager->GetCurrentScreen() == ui::UIScreen::PauseMenu) {
                m_uiManager->SetScreen(ui::UIScreen::None);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelClose");
                return;
            } else if (m_uiManager->GetCurrentScreen() == ui::UIScreen::MainMenu) {
                Stop();
                return;
            } else {
                m_uiManager->SetScreen(ui::UIScreen::PauseMenu);
                return;
            }
        } else {
            Stop();
            return;
        }
    }

    // Diagnostic & Developer Hotkeys
    bool toggleDevUI = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleDevUI)
                                      : IsKeyPressed(KEY_F1);
    if (toggleDevUI) {
        if (m_devUI) {
            m_devUI->ToggleVisible();
        }
    }

    bool toggleCam = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleCameraMode)
                                    : IsKeyPressed(KEY_F2);
    if (toggleCam) {
        if (m_camera->GetMode() == graphics::CameraMode::Follow) {
            m_camera->SetMode(graphics::CameraMode::Free);
            Logger::Info("CAMERA", "Camera mode switched to FREE (WASD/Arrows to pan)");
        } else {
            m_camera->SetMode(graphics::CameraMode::Follow);
            Logger::Info("CAMERA", "Camera mode switched to FOLLOW (Tracking Player)");
        }
    }

    bool toggleCol = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleCollisionDebug)
                                    : IsKeyPressed(KEY_F3);
    if (toggleCol) {
        if (m_worldRenderer) {
            m_worldRenderer->SetShowCollision(!m_worldRenderer->GetShowCollision());
            Logger::Info("DEBUG", std::string("Collision Bounds: ") + (m_worldRenderer->GetShowCollision() ? "ON" : "OFF"));
        }
    }

    bool toggleOrigin = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleOriginDebug)
                                       : IsKeyPressed(KEY_F4);
    if (toggleOrigin) {
        if (m_worldRenderer) {
            m_worldRenderer->SetShowOrigins(!m_worldRenderer->GetShowOrigins());
            Logger::Info("DEBUG", std::string("Origin Anchors: ") + (m_worldRenderer->GetShowOrigins() ? "ON" : "OFF"));
        }
    }

    bool toggleAnim = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleAnimDebug)
                                     : IsKeyPressed(KEY_F5);
    if (toggleAnim) {
        if (m_devUI) {
            m_devUI->ToggleAnimDebug();
            Logger::Info("DEBUG", std::string("Animation HUD: ") + (m_devUI->IsAnimDebug() ? "ON" : "OFF"));
        }
    }

    bool toggleCombat = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleCombatDebug)
                                       : IsKeyPressed(KEY_F6);
    if (toggleCombat) {
        ToggleCombatDebug();
        Logger::Info("DEBUG", std::string("Combat Hitboxes: ") + (IsCombatDebug() ? "ON" : "OFF"));
    }

    bool toggleSpawner = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ToggleEnemySpawner)
                                        : IsKeyPressed(KEY_F7);
    if (toggleSpawner) {
        if (m_enemySpawner) {
            m_enemySpawner->SetEnabled(!m_enemySpawner->IsEnabled());
            Logger::Info("DEBUG", std::string("Enemy Spawner: ") + (m_enemySpawner->IsEnabled() ? "ENABLED" : "PAUSED"));
        }
    }

    bool resetPlayer = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::ResetPlayer)
                                      : IsKeyPressed(KEY_R);
    if (resetPlayer) {
        if (m_world && m_world->IsLoaded() && m_player) {
            Vector2 spawnPos = m_world->GetSpawnPosition("player_spawn");
            m_player->Spawn(spawnPos);
            m_camera->SetTarget(spawnPos);
            m_camera->SetFollowTarget(spawnPos);
            m_camera->SetZoom(1.0f);
            Logger::Info("PLAYER", "Player and camera reset to spawn position");
        } else {
            m_camera->SetTarget(Vector2{ 800.0f, 600.0f });
            m_camera->SetZoom(1.0f);
            Logger::Info("CAMERA", "Camera reset to center (800, 600) @ 1.0x zoom");
        }
    }

    // UI Screen Toggles & Hotbar Inputs
    bool toggleInv = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::OpenInventory)
                                    : (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_TAB));
    if (toggleInv) {
        if (m_uiManager) {
            if (m_uiManager->GetCurrentScreen() == ui::UIScreen::Inventory) {
                m_uiManager->SetScreen(ui::UIScreen::None);
                if (m_inputManager) m_inputManager->PopContext();
            } else {
                m_uiManager->SetScreen(ui::UIScreen::Inventory);
                if (m_inputManager) m_inputManager->PushContext(input::InputContextType::Inventory);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
            }
        }
    }

    // Keep Input Context synced with UI screen state
    if (m_uiManager && m_inputManager) {
        bool isInvScreen = (m_uiManager->GetCurrentScreen() == ui::UIScreen::Inventory);
        bool hasInvContext = (m_inputManager->GetContextManager().GetActiveContextType() == input::InputContextType::Inventory);
        if (isInvScreen && !hasInvContext) {
            m_inputManager->PushContext(input::InputContextType::Inventory);
        } else if (!isInvScreen && hasInvContext) {
            m_inputManager->PopContext();
        }
    }

    bool toggleWardrobe = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::OpenWardrobe)
                                         : IsKeyPressed(KEY_C);
    if (toggleWardrobe) {
        if (m_uiManager) {
            if (m_uiManager->GetCurrentScreen() == ui::UIScreen::Wardrobe) {
                m_uiManager->SetScreen(ui::UIScreen::None);
            } else {
                m_uiManager->SetScreen(ui::UIScreen::Wardrobe);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
            }
        }
    }

    if (m_player && m_itemManager) {
        auto tryUseOrEquipSlot = [this](size_t slotIdx) {
            auto& inv = m_player->GetInventory();
            if (slotIdx < inv.GetCapacity()) {
                const auto& slot = inv.GetSlot(slotIdx);
                if (slot.has_value() && m_itemManager) {
                    const auto* def = m_itemManager->GetItem(slot->definitionId);
                    if (def) {
                        if (def->category == items::ItemCategory::Consumable) {
                            m_player->UseConsumable(slotIdx, m_itemManager.get(), m_audioSystem.get());
                        } else if (def->category == items::ItemCategory::Weapon ||
                                   def->category == items::ItemCategory::Armor ||
                                   def->category == items::ItemCategory::Clothing) {
                            m_player->GetEquipmentSystem().Equip(def->id, *m_itemManager, m_player.get());
                            if (m_audioSystem) m_audioSystem->DispatchEvent("Item.Equip");
                        }
                    }
                }
            }
        };

        bool hot1 = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::Hotbar1) : IsKeyPressed(KEY_ONE);
        bool hot2 = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::Hotbar2) : IsKeyPressed(KEY_TWO);
        bool hot3 = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::Hotbar3) : IsKeyPressed(KEY_THREE);
        bool hot4 = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::Hotbar4) : IsKeyPressed(KEY_FOUR);

        if (hot1) tryUseOrEquipSlot(0);
        if (hot2) tryUseOrEquipSlot(1);
        if (hot3) tryUseOrEquipSlot(2);
        if (hot4) tryUseOrEquipSlot(3);
    }

    // World Interaction System (Phase 22)
    bool interact = m_inputManager ? m_inputManager->ConsumeAction(input::InputAction::Interact)
                                   : (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_F));
    if (interact) {
        if (m_interactionManager) {
            interaction::InteractionContext interactContext{
                m_player.get(),
                m_world.get(),
                m_buildingManager.get(),
                m_collisionManager.get(),
                m_itemManager.get(),
                m_audioSystem.get(),
                m_camera.get(),
                m_assetManager.get()
            };
            m_interactionManager->TryInteract(interactContext);
        }
    }

    if (m_camera->GetMode() == graphics::CameraMode::Free) {
        float zoom = m_camera->GetZoom();
        float moveSpeed = (350.0f / (zoom > 0.1f ? zoom : 1.0f)) * dt;
        Vector2 delta{ 0.0f, 0.0f };

        bool panUp = m_inputManager ? m_inputManager->IsActionHeld(input::InputAction::CameraPanUp) : IsKeyDown(KEY_W);
        bool panDown = m_inputManager ? m_inputManager->IsActionHeld(input::InputAction::CameraPanDown) : IsKeyDown(KEY_S);
        bool panLeft = m_inputManager ? m_inputManager->IsActionHeld(input::InputAction::CameraPanLeft) : IsKeyDown(KEY_A);
        bool panRight = m_inputManager ? m_inputManager->IsActionHeld(input::InputAction::CameraPanRight) : IsKeyDown(KEY_D);

        if (panUp) delta.y -= moveSpeed;
        if (panDown) delta.y += moveSpeed;
        if (panLeft) delta.x -= moveSpeed;
        if (panRight) delta.x += moveSpeed;

        if (delta.x != 0.0f || delta.y != 0.0f) {
            m_camera->Move(delta);
        }
    }

    float zoomSpeed = 1.2f * dt;
    bool zoomOut = m_inputManager ? m_inputManager->IsActionHeld(input::InputAction::CameraZoomOut) : IsKeyDown(KEY_Q);
    bool zoomIn = m_inputManager ? m_inputManager->IsActionHeld(input::InputAction::CameraZoomIn) : IsKeyDown(KEY_E);
    if (zoomOut) {
        m_camera->ZoomBy(-zoomSpeed);
    }
    if (zoomIn) {
        m_camera->ZoomBy(zoomSpeed);
    }
}

void Game::FixedUpdate(float /*fixedDt*/) {
}

void Game::Update(float dt) {
    if (m_audioSystem) {
        m_audioSystem->Update(dt);
    }

    if (m_uiManager) {
        if (m_inputManager) {
            m_uiManager->Update(dt, m_inputManager.get());
        } else {
            Vector2 mouseScreen = GetMousePosition();
            Vector2 mouseVirtual = m_renderer->GetViewport().ScreenToVirtual(mouseScreen);
            bool mouseLeftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool mouseLeftDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            m_uiManager->Update(dt, mouseVirtual, mouseLeftPressed, mouseLeftDown);
        }
    }

    bool uiBlocksGameplay = (m_uiManager && m_uiManager->IsScreenActive());

    if (!uiBlocksGameplay) {
        if (m_combatSystem) {
            m_combatSystem->Update(dt, m_audioSystem.get());
        }

        if (m_world && m_world->IsLoaded()) {
            m_world->UpdateWorldItems(dt);
        }

        if (m_player && m_world && m_world->IsLoaded()) {
            if (m_audioSystem) {
                m_audioSystem->SetListenerPosition(m_player->GetPosition());
            }
            m_player->UpdateWithWorld(dt, *m_world, *m_assetManager, m_combatSystem.get(), m_audioSystem.get());
        } else {
            m_testScene->Update(dt);
        }

        if (m_aiManager && m_enemySpawner && m_world && m_world->IsLoaded() && m_combatSystem) {
            m_aiManager->Update(dt, *m_world, *m_enemySpawner, *m_combatSystem, m_player.get(), m_audioSystem.get(), m_luaManager.get());
        }

        if (m_enemySpawner && m_world && m_world->IsLoaded() && m_combatSystem) {
            if (!m_buildingManager || !m_buildingManager->IsIndoors()) {
                Vector2 pPos = m_player ? m_player->GetPosition() : Vector2{ 0.0f, 0.0f };
                m_enemySpawner->Update(dt, *m_world, *m_combatSystem, *m_assetManager, m_luaManager.get(), pPos);
            }
        }

        // 4. Update Collision Subsystem (Phase 20)
        if (m_collisionManager) {
            if (m_player && m_player->GetColliderId() != 0) {
                m_collisionManager->SetColliderTransform(m_player->GetColliderId(), m_player->GetPosition());
            }
            m_collisionManager->Update(dt);
        }

        // 5. Update World Interaction System (Phase 22)
        if (m_interactionManager) {
            interaction::InteractionContext interactContext{
                m_player.get(),
                m_world.get(),
                m_buildingManager.get(),
                m_collisionManager.get(),
                m_itemManager.get(),
                m_audioSystem.get(),
                m_camera.get(),
                m_assetManager.get()
            };
            m_interactionManager->SyncWithWorld(interactContext);
            m_interactionManager->Update(dt, interactContext, m_inputManager.get());

            if (m_interactionManager->GetPickupPolicy() == interaction::PickupPolicy::Automatic && m_player && m_world) {
                m_world->ProcessPickups(*m_player, m_itemManager.get(), m_audioSystem.get());
            }
        } else if (m_player && m_world) {
            m_world->ProcessPickups(*m_player, m_itemManager.get(), m_audioSystem.get());
        }
    }

    if (m_envAnimationSystem) {
        m_envAnimationSystem->Update(dt, *m_camera, m_world.get());
    }

    if (m_camera->GetMode() == graphics::CameraMode::Follow) {
        if (m_player) {
            m_camera->SetFollowTarget(m_player->GetPosition());
        } else {
            m_camera->SetFollowTarget(m_testScene->GetTargetPosPtr());
        }
    }
    if (m_cosmeticsStoreManager) {
        m_cosmeticsStoreManager->Update(dt);
    }

    // Phase 31: Update Corruption Storm & Safe Zone System
    if (m_stormManager && !uiBlocksGameplay) {
        ui::HUDManager* hudMgr = m_uiManager ? &m_uiManager->GetHUDManager() : nullptr;
        m_stormManager->Update(dt, m_player.get(), m_audioSystem.get(), hudMgr);
    }

    m_camera->Update(dt);
}

void Game::Render(float dt) {
    float fps = static_cast<float>(GetFPS());

    // 1. Begin virtual resolution frame
    m_renderer->BeginFrame();

    // 2. Render World Layer with Camera, Depth-Sorted Objects & Entities
    m_renderer->BeginWorld(*m_camera);
    if (m_world && m_world->IsLoaded() && m_worldRenderer) {
        if (m_envAnimationSystem) {
            m_envAnimationSystem->RenderWorldGround(*m_renderer, *m_camera, *m_assetManager);
        }
        std::vector<entities::Entity*> renderEntities;
        if (m_player) renderEntities.push_back(m_player.get());
        if (m_enemySpawner) {
            for (const auto& e : m_enemySpawner->GetEnemies()) {
                if (e && e->IsActive()) renderEntities.push_back(e.get());
            }
        }
        m_worldRenderer->Render(*m_renderer, *m_world, *m_camera, *m_assetManager, renderEntities);

        // Render Loot Containers and World Items (Phase 19)
        m_world->RenderLootContainers(*m_renderer, *m_assetManager);
        m_world->RenderWorldItems(*m_renderer, *m_assetManager, m_itemManager.get());

        // Render Doors & Mechanisms (Phase 22)
        if (m_interactionManager) {
            for (auto& door : const_cast<std::vector<interaction::Door>&>(m_interactionManager->GetAllDoors())) {
                door.Render(*m_renderer, *m_assetManager);
            }
            for (auto& mech : const_cast<std::vector<interaction::Mechanism>&>(m_interactionManager->GetAllMechanisms())) {
                mech.Render(*m_renderer, *m_assetManager);
            }

            // In-world interaction debug visualizer
            if (m_devUI && m_devUI->IsInteractionDebug() && m_player) {
                Vector2 pPos = m_player->GetPosition();
                float r = m_interactionManager->GetDefaultRange();
                ::DrawCircleLines(static_cast<int>(pPos.x), static_cast<int>(pPos.y), r, Color{ 80, 200, 255, 160 });

                const auto& candidates = m_interactionManager->GetCurrentCandidates();
                for (const auto& c : candidates) {
                    ::DrawLine(static_cast<int>(pPos.x), static_cast<int>(pPos.y),
                               static_cast<int>(c.component->position.x), static_cast<int>(c.component->position.y),
                               Color{ 240, 220, 80, 140 });
                }

                const auto* target = m_interactionManager->GetCurrentTarget();
                if (target) {
                    ::DrawRectangleLines(static_cast<int>(target->bounds.x - 2), static_cast<int>(target->bounds.y - 2),
                                         static_cast<int>(target->bounds.width + 4), static_cast<int>(target->bounds.height + 4),
                                         Color{ 60, 255, 90, 220 });
                }
            }
        }

        // Render Flying Projectiles (Phase 19)
        if (m_combatSystem) {
            m_combatSystem->GetProjectileSystem().Render(*m_renderer, *m_assetManager);
        }

        if (m_envAnimationSystem) {
            m_envAnimationSystem->RenderWorldObjects(*m_renderer, *m_camera, *m_assetManager);
            m_envAnimationSystem->RenderWorldOverhead(*m_renderer, *m_camera, *m_assetManager);
        }

        if (m_showCombatDebug && m_combatSystem) {
            m_combatSystem->RenderDebug();
        }

        // Render Collision Overlaps & Triggers (Phase 20)
        if (m_worldRenderer->GetShowCollision() && m_collisionManager && m_collisionDebugRenderer) {
            m_collisionDebugRenderer->Render(*m_collisionManager, m_worldRenderer->GetVisibleWorldArea(), false, true, true, false);
        }

        // Render Battle Royale World Overlays (Phase 30)
        if (m_worldManager) {
            m_worldManager->RenderDebugOverlays(*m_camera);
        }

        // Render Corruption Storm World Layer (Phase 31)
        if (m_stormManager) {
            m_stormManager->RenderWorld(*m_camera, m_assetManager.get());
        }
    } else {
        m_testScene->RenderWorld(*m_renderer, *m_camera, m_renderer->GetDrawContext(), m_assetManager.get());
    }
    m_renderer->EndWorld(*m_camera);

    // 3. Render In-Game UI Layer
    m_renderer->BeginUI();
    m_testScene->RenderUI(m_renderer->GetDrawContext(), *m_camera, m_renderer->GetViewport(), m_assetManager.get());
    if (m_envAnimationSystem) {
        m_envAnimationSystem->RenderScreenAtmosphere(*m_renderer, m_renderer->GetViewport(), *m_assetManager);
    }

    // Render Player-Facing UI System (Phase 18 & Phase 22 Prompt & Phase 25 Customization)
    if (m_uiManager) {
        const auto* prompt = m_interactionManager ? &m_interactionManager->GetCurrentPrompt() : nullptr;
        m_uiManager->Render(*m_renderer, *m_assetManager, m_player.get(), m_itemManager.get(), prompt, m_world.get(), m_customizationManager.get(), m_cosmeticsStoreManager.get());

        // Phase 31: Render Storm onto Minimap Radar
        if (m_stormManager) {
            const auto& mm = m_uiManager->GetHUDManager().GetMinimap();
            if (mm.IsVisible()) {
                Rectangle mmBounds = mm.GetBounds();
                Vector2 mmCenter{ mmBounds.x + mmBounds.width * 0.5f, mmBounds.y + mmBounds.height * 0.5f };
                float mmRadius = mmBounds.width * 0.5f;
                Vector2 pPos = m_player ? m_player->GetPosition() : Vector2{ 0.0f, 0.0f };
                Rectangle wBounds = m_world && m_world->IsLoaded() ? m_world->GetWorldBounds() : Rectangle{ 768.0f, 768.0f, 8704.0f, 8704.0f };
                m_stormManager->RenderMinimap(mmCenter, mmRadius, pPos, mm.GetZoom(), wBounds);
            }
        }
    }

    // Phase 31: Screen-space Storm Hazards & Directional HUD Warning
    if (m_stormManager) {
        m_stormManager->RenderScreenEffects(*m_camera, m_player.get());
        m_stormManager->RenderHUDWarning(m_assetManager.get(), m_player.get());
    }

    m_renderer->EndUI();

    // 4. End virtual canvas render target
    m_renderer->EndFrame();

    // 5. Present scaled canvas to window
    m_renderer->Present();

    // 6. Developer overlay (Dear ImGui with Asset Browser, Validation, World & Player Tools)
    if (m_devUI && (m_devUI->IsVisible() || m_devUI->IsAnimDebug())) {
        Vector2 mouseScreen = GetMousePosition();
        Vector2 mouseVirtual = m_renderer->GetViewport().ScreenToVirtual(mouseScreen);
        Vector2 mouseWorld = m_camera->ScreenToWorld(mouseScreen, m_renderer->GetViewport());

        m_devUI->BeginFrame();
        m_devUI->RenderDebugOverlay(
            m_config,
            fps,
            dt,
            *m_camera,
            m_renderer->GetViewport(),
            mouseScreen,
            mouseVirtual,
            mouseWorld,
            m_assetManager.get(),
            m_world.get(),
            m_worldRenderer.get(),
            m_player.get(),
            m_combatSystem.get(),
            m_enemySpawner.get(),
            m_envAnimationSystem.get(),
            m_buildingManager.get(),
            m_biomeManager.get(),
            m_weaponManager.get(),
            m_itemManager.get(),
            m_audioSystem.get(),
            m_collisionManager.get(),
            m_inputManager.get(),
            m_interactionManager.get(),
            m_customizationManager.get(),
            m_cosmeticsStoreManager.get(),
            m_uiManager.get(),
            m_aiManager.get(),
            m_worldManager.get(),
            m_stormManager.get()
        );
        m_devUI->EndFrame();
    }

    EndDrawing();
}

void Game::Shutdown() {
    if (m_isShutdown) return;
    m_isShutdown = true;
    m_running = false;

    if (m_stormManager) {
        m_stormManager->Reset();
    }

    if (m_uiManager) {
        m_uiManager->Shutdown();
    }

    if (m_audioSystem) {
        m_audioSystem->Shutdown();
    }

    if (m_enemySpawner && m_combatSystem) {
        m_enemySpawner->ClearAllEnemies(*m_combatSystem);
    }

    if (m_player && m_combatSystem) {
        m_combatSystem->UnregisterCombatant(m_player->GetCombatantId());
    }

    if (m_world) {
        m_world->UnloadMap();
    }

    if (m_devUI) {
        m_devUI->Shutdown();
    }

    if (m_assetManager) {
        m_assetManager->Shutdown();
    }

    if (m_renderer) {
        m_renderer->Shutdown();
    }

    if (m_luaManager) {
        m_luaManager->Shutdown();
    }

    Logger::Info("ENGINE", "Engine shutdown completed cleanly.");
}

} // namespace wildbound::core
