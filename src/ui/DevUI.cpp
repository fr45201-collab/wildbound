#include "DevUI.hpp"
#include "../core/Logger.hpp"
#include "../resources/AssetManager.hpp"
#include "../resources/AssetValidator.hpp"
#include "../world/World.hpp"
#include "../world/WorldItem.hpp"
#include "../world/WorldRenderer.hpp"
#include "../world/EnemySpawner.hpp"
#include "../entities/Player.hpp"
#include "../entities/Enemy.hpp"
#include "../combat/CombatSystem.hpp"
#include "../world/environment/EnvironmentalAnimationSystem.hpp"
#include "../world/BuildingManager.hpp"
#include "../world/BiomeManager.hpp"
#include "../weapons/WeaponManager.hpp"
#include "../items/ItemManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../physics/CollisionManager.hpp"
#include "../input/InputManager.hpp"
#include "../interaction/InteractionManager.hpp"
#include "../customization/CharacterCustomizationManager.hpp"
#include "../cosmetics/CosmeticsStoreManager.hpp"
#include "../cosmetics/CosmeticsRarity.hpp"
#include "UIManager.hpp"
#include "../ai/AIManager.hpp"
#include "../world/WorldManager.hpp"
#include "../storm/StormManager.hpp"
#include "../match/MatchManager.hpp"
#include "../gamemodes/GameModeManager.hpp"
#include "../network/NetworkManager.hpp"
#include "../network/replication/ReplicationManager.hpp"
#include "../scripting/LuaManager.hpp"

#include <imgui.h>
#include <rlImGui.h>
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::ui {

DevUI::DevUI() = default;

DevUI::~DevUI() {
    Shutdown();
}

bool DevUI::Initialize() {
    if (m_initialized) {
        return true;
    }

    try {
        rlImGuiSetup(true);
        m_initialized = true;
        Logger::Info("DEV_UI", "Dear ImGui and rlImGui initialized for Phase 2 dev tools.");
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("DEV_UI", std::string("Failed to initialize rlImGui: ") + ex.what());
        return false;
    }
}

void DevUI::Shutdown() {
    if (m_initialized) {
        Logger::Info("DEV_UI", "Shutting down rlImGui dev tools.");
        rlImGuiShutdown();
        m_initialized = false;
    }
}

void DevUI::BeginFrame() {
    if (m_initialized) {
        rlImGuiBegin();
    }
}

void DevUI::EndFrame() {
    if (m_initialized) {
        rlImGuiEnd();
    }
}

void DevUI::RenderDebugOverlay(
    const core::EngineConfig& config,
    float fps,
    float dt,
    const graphics::Camera2DSystem& camera,
    const graphics::Viewport& viewport,
    Vector2 mouseScreen,
    Vector2 mouseVirtual,
    Vector2 mouseWorld,
    resources::AssetManager* assetMgr,
    world::World* world,
    world::WorldRenderer* worldRenderer,
    entities::Player* player,
    combat::CombatSystem* combatSys,
    world::EnemySpawner* spawner,
    world::environment::EnvironmentalAnimationSystem* envAnimSys,
    world::BuildingManager* buildingMgr,
    world::BiomeManager* biomeMgr,
    weapons::WeaponManager* weaponMgr,
    items::ItemManager* itemMgr,
    audio::AudioSystem* audioSys,
    physics::CollisionManager* colMgr,
    input::InputManager* inputMgr,
    interaction::InteractionManager* interactionMgr,
    customization::CharacterCustomizationManager* customMgr,
    cosmetics::CosmeticsStoreManager* storeMgr,
    ui::UIManager* uiMgr,
    ai::AIManager* aiMgr,
    world::WorldManager* worldMgr,
    storm::StormManager* stormMgr,
    match::MatchManager* matchMgr,
    network::NetworkManager* netMgr,
    scripting::LuaManager* luaMgr)
{
    if (m_showAnimDebug) {
        RenderAnimationHUD(player);
    }

    if (!m_initialized || !m_visible) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(540, 580), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Wildbound Developer Suite (F1)", &m_visible)) {
        if (ImGui::BeginTabBar("DevSuiteTabs")) {
            // Tab 1: Engine Metrics & Camera
            if (ImGui::BeginTabItem("Metrics & Camera")) {
                ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "%s (v%s)", config.gameTitle.c_str(), config.gameVersion.c_str());
                ImGui::Separator();

                // Timing
                float frameTimeMs = dt * 1000.0f;
                ImGui::Text("FPS: %.1f | Frame Time: %.2f ms", fps, frameTimeMs);
                ImGui::Separator();

                // Viewport
                ImGui::Text("Virtual Resolution: %d x %d", viewport.GetVirtualWidth(), viewport.GetVirtualHeight());
                ImGui::Text("Window Resolution:  %d x %d", viewport.GetWindowWidth(), viewport.GetWindowHeight());
                Rectangle dst = viewport.GetDestRect();
                ImGui::Text("Viewport: [%.0f, %.0f, %.0f, %.0f] (Scale: %.2fx)", dst.x, dst.y, dst.width, dst.height, viewport.GetScale());
                ImGui::Separator();

                // Camera
                const char* modeStr = (camera.GetMode() == graphics::CameraMode::Follow) ? "FOLLOW [Target]" : "FREE [WASD]";
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Camera Mode: %s", modeStr);
                Vector2 camPos = camera.GetTarget();
                ImGui::Text("Camera Pos:  (%.1f, %.1f) | Zoom: %.2fx", camPos.x, camPos.y, camera.GetZoom());
                ImGui::Separator();

                // Coordinates
                ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.5f, 1.0f), "Coordinates:");
                ImGui::Text("Screen (Window):  (%.1f, %.1f)", mouseScreen.x, mouseScreen.y);
                ImGui::Text("Virtual (Canvas): (%.1f, %.1f)", mouseVirtual.x, mouseVirtual.y);
                ImGui::Text("World Space:      (%.1f, %.1f)", mouseWorld.x, mouseWorld.y);
                ImGui::Separator();

                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Controls: WASD=Move, Shift=Sprint, Space/LClick=Attack, F1=DevUI, F2=Follow, F3=Col, F4=Origin, F5=AnimHUD, F6=Hitbox, F7=Spawner");
                ImGui::EndTabItem();
            }

            // Tab 2: Combat & AI (Phase 5 & 29)
            if (ImGui::BeginTabItem("Combat & AI")) {
                RenderCombatAndAI(combatSys, spawner, player, aiMgr);
                ImGui::EndTabItem();
            }

            // Tab 3: Player & Character Controller (Phase 4)
            if (ImGui::BeginTabItem("Player & Character")) {
                RenderPlayerDebug(player);
                ImGui::EndTabItem();
            }

            // Tab 3: World & Map System (Phase 3, 12, 14, 30)
            if (ImGui::BeginTabItem("World & Map")) {
                RenderWorldAndMap(world, worldRenderer, mouseWorld, player, buildingMgr, &camera, assetMgr, biomeMgr, envAnimSys, worldMgr);
                ImGui::EndTabItem();
            }

            // Tab 4: Asset Browser
            if (ImGui::BeginTabItem("Asset Browser")) {
                RenderAssetBrowser(assetMgr);
                ImGui::EndTabItem();
            }

            // Tab 5: Asset Validation & Licensing
            if (ImGui::BeginTabItem("Asset Validation")) {
                RenderAssetValidation(assetMgr);
                ImGui::EndTabItem();
            }

            // Tab 6: Modular Wardrobe (Phase 7 & 8 & 25)
            if (ImGui::BeginTabItem("Wardrobe")) {
                RenderWardrobe(player, assetMgr, customMgr);
                ImGui::EndTabItem();
            }

            // Tab 7: Creatures & Wildlife Preview (Phase 9)
            if (ImGui::BeginTabItem("Creatures")) {
                RenderCreaturePreview(assetMgr);
                ImGui::EndTabItem();
            }

            // Tab 8: Environment Asset Collection (Phase 10)
            if (ImGui::BeginTabItem("Environment")) {
                RenderEnvironmentPreview(assetMgr);
                ImGui::EndTabItem();
            }

            // Tab 9: Living World & Environmental Animations (Phase 11)
            if (ImGui::BeginTabItem("Living World")) {
                Vector2 pPos = player ? player->GetPosition() : mouseWorld;
                RenderLivingWorld(envAnimSys, assetMgr, pPos);
                ImGui::EndTabItem();
            }

            // Tab 10: Weapons & Equipment (Phase 15/24)
            if (ImGui::BeginTabItem("Weapons & Equipment")) {
                RenderWeaponPreview(weaponMgr, assetMgr, player);
                ImGui::EndTabItem();
            }

            // Tab 11: Items & Collectibles (Phase 16)
            if (ImGui::BeginTabItem("Items")) {
                RenderItemPreview(itemMgr, assetMgr);
                ImGui::EndTabItem();
            }

            // Tab 12: Audio & Sound System (Phase 18)
            if (ImGui::BeginTabItem("Audio & Sound")) {
                RenderAudioDebug(audioSys);
                ImGui::EndTabItem();
            }

            // Tab 13: Physics & Collision System (Phase 20)
            if (ImGui::BeginTabItem("Physics & Collision")) {
                RenderPhysicsAndCollision(colMgr);
                ImGui::EndTabItem();
            }

            // Tab 14: Input & Controls (Phase 21)
            if (ImGui::BeginTabItem("Input & Controls")) {
                RenderInputDebug(inputMgr);
                ImGui::EndTabItem();
            }

            // Tab 15: World Interactions (Phase 22)
            if (ImGui::BeginTabItem("World Interactions")) {
                RenderInteractionDebug(interactionMgr, player);
                ImGui::EndTabItem();
            }

            // Tab 16: Loot & Inventory (Phase 23)
            if (ImGui::BeginTabItem("Loot & Inventory")) {
                RenderLootAndInventory(player, world, itemMgr, audioSys);
                ImGui::EndTabItem();
            }

            // Tab 17: Cosmetics Store & Wardrobe (Phase 26)
            if (ImGui::BeginTabItem("Cosmetics Store")) {
                RenderCosmeticsStore(storeMgr, customMgr);
                ImGui::EndTabItem();
            }

            // Tab 18: UI & HUD Debugger (Phase 27)
            if (ImGui::BeginTabItem("UI & HUD")) {
                RenderUIAndHUD(uiMgr, player);
                ImGui::EndTabItem();
            }

            // Tab 19: Corruption Storm & Safe Zone (Phase 31)
            if (ImGui::BeginTabItem("Corruption Storm")) {
                RenderStormDebug(stormMgr, player);
                ImGui::EndTabItem();
            }

            // Tab 20: Match Lifecycle (Phase 32)
            if (ImGui::BeginTabItem("Match Lifecycle")) {
                RenderMatchDebug(matchMgr, player);
                ImGui::EndTabItem();
            }

            // Tab 21: Network Diagnostics (Phase 36)
            if (ImGui::BeginTabItem("Network Diagnostics")) {
                RenderNetworkDiagnostics(netMgr);
                ImGui::EndTabItem();
            }

            // Tab 22: Lua & Scripting (Phase 46)
            if (ImGui::BeginTabItem("Lua & Scripting")) {
                RenderLuaDiagnostics(luaMgr, spawner);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void DevUI::RenderWorldAndMap(world::World* world, world::WorldRenderer* worldRenderer, Vector2 mouseWorld, entities::Player* player, world::BuildingManager* buildingMgr, const graphics::Camera2DSystem* camera, resources::AssetManager* assetMgr, world::BiomeManager* biomeMgr, world::environment::EnvironmentalAnimationSystem* envAnim, world::WorldManager* worldMgr) {
    if (!world || !world->IsLoaded()) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "No world map is currently loaded.");
        return;
    }

    world::Tilemap* tilemap = world->GetTilemap();
    if (!tilemap) return;

    // 1. Debug Overlays
    if (worldRenderer) {
        if (ImGui::CollapsingHeader("World Visualizers & Overlays", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool showGrid = worldRenderer->GetShowGrid();
            if (ImGui::Checkbox("Show Tile Grid (32x32)", &showGrid)) {
                worldRenderer->SetShowGrid(showGrid);
            }
            ImGui::SameLine();
            bool showCollision = worldRenderer->GetShowCollision();
            if (ImGui::Checkbox("Show Collision Bounds", &showCollision)) {
                worldRenderer->SetShowCollision(showCollision);
            }

            bool showBounds = worldRenderer->GetShowBounds();
            if (ImGui::Checkbox("Show World Bounds", &showBounds)) {
                worldRenderer->SetShowBounds(showBounds);
            }
            ImGui::SameLine();
            bool showOrigins = worldRenderer->GetShowOrigins();
            if (ImGui::Checkbox("Show Object Origins", &showOrigins)) {
                worldRenderer->SetShowOrigins(showOrigins);
            }
        }
    }

    // 2. Layer Visibility & Opacity
    if (ImGui::CollapsingHeader("Map Layers & Visibility", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& layers = tilemap->GetLayers();
        for (size_t i = 0; i < layers.size(); ++i) {
            auto& layer = layers[i];
            ImGui::PushID(static_cast<int>(i));

            bool visible = layer.IsVisible();
            if (ImGui::Checkbox(layer.GetName().c_str(), &visible)) {
                layer.SetVisible(visible);
            }

            ImGui::SameLine(200);
            float opacity = layer.GetOpacity();
            ImGui::SetNextItemWidth(120);
            if (ImGui::SliderFloat("##Opacity", &opacity, 0.0f, 1.0f, "%.2f")) {
                layer.SetOpacity(opacity);
            }

            ImGui::PopID();
        }
    }

    // 3. Map Statistics & Performance
    if (ImGui::CollapsingHeader("Map Statistics & Culling", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Map Name:      %s", tilemap->GetName().c_str());
        ImGui::Text("Dimensions:    %d x %d tiles (%d x %d px)",
                    tilemap->GetWidth(), tilemap->GetHeight(),
                    tilemap->GetWorldWidth(), tilemap->GetWorldHeight());
        ImGui::Text("Tile Size:     %d x %d px", tilemap->GetTileWidth(), tilemap->GetTileHeight());
        ImGui::Text("Tilesets:      %zu loaded", tilemap->GetTilesets().size());
        ImGui::Text("Total Objects: %zu props", tilemap->GetObjects().size());

        if (worldRenderer) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "Camera Culling Performance:");
            size_t totalTiles = static_cast<size_t>(tilemap->GetWidth() * tilemap->GetHeight() * tilemap->GetLayerCount());
            size_t culledTiles = worldRenderer->GetRenderedTileCount();
            ImGui::Text("Rendered Tiles:   %zu / %zu (%.1f%% culled)",
                        culledTiles, totalTiles,
                        totalTiles > 0 ? (1.0f - static_cast<float>(culledTiles) / totalTiles) * 100.0f : 0.0f);
            ImGui::Text("Rendered Objects: %zu / %zu",
                        worldRenderer->GetRenderedObjectCount(), tilemap->GetObjects().size());
            Rectangle visArea = worldRenderer->GetVisibleWorldArea();
            ImGui::Text("Visible Area:     [%.0f, %.0f, %.0f, %.0f]",
                        visArea.x, visArea.y, visArea.width, visArea.height);
        }
    }

    // 4. Cursor Tile Inspector
    if (ImGui::CollapsingHeader("Cursor Tile Inspector", ImGuiTreeNodeFlags_DefaultOpen)) {
        int tw = tilemap->GetTileWidth();
        int th = tilemap->GetTileHeight();
        int col = static_cast<int>(std::floor(mouseWorld.x / tw));
        int row = static_cast<int>(std::floor(mouseWorld.y / th));

        ImGui::Text("World Position: (%.1f, %.1f)", mouseWorld.x, mouseWorld.y);
        ImGui::Text("Tile Cell:      Col %d, Row %d", col, row);

        if (col >= 0 && col < tilemap->GetWidth() && row >= 0 && row < tilemap->GetHeight()) {
            ImGui::Separator();
            ImGui::Text("Tile GIDs at Cursor:");
            for (const auto& layer : tilemap->GetLayers()) {
                int gid = layer.GetTileGid(col, row);
                if (gid > 0) {
                    ImGui::BulletText("%s: GID %d", layer.GetName().c_str(), gid);
                } else {
                    ImGui::BulletText("%s: <Empty>", layer.GetName().c_str());
                }
            }

            Rectangle mousePointRec{ mouseWorld.x - 1, mouseWorld.y - 1, 2, 2 };
            bool hasCollision = world->CheckCollision(mousePointRec);
            ImGui::Separator();
            if (hasCollision) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Collision at Cursor: SOLID / BLOCKED");
            } else {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Collision at Cursor: WALKABLE");
            }
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Cursor is outside map bounds.");
        }
    }

    // 5. Biome Selector & World Explorer (Phase 14)
    if (biomeMgr && ImGui::CollapsingHeader("Biome Selector & World Explorer (Phase 14)", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto* activeBiome = biomeMgr->GetActiveBiome();
        std::string activeName = activeBiome ? activeBiome->displayName : "Unknown";
        std::string activeId = activeBiome ? activeBiome->id : "unknown";

        ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "Active Biome: %s [%s]", activeName.c_str(), activeId.c_str());
        if (activeBiome) {
            ImGui::TextWrapped("%s", activeBiome->description.c_str());
            ImGui::TextDisabled("Tileset: %s | Weather: %s | Fog: %.2f | Clouds: %s",
                                activeBiome->tileset.c_str(),
                                activeBiome->atmosphere.weather.c_str(),
                                activeBiome->atmosphere.fogDensity,
                                activeBiome->atmosphere.cloudLayerEnabled ? "ENABLED" : "DISABLED");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Available Biomes:");

        const auto& allBiomes = biomeMgr->GetAllBiomes();
        for (size_t i = 0; i < allBiomes.size(); ++i) {
            const auto& b = allBiomes[i];
            ImGui::PushID(static_cast<int>(i + 5000));

            bool isCurrent = (b.id == biomeMgr->GetActiveBiomeId());
            if (isCurrent) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "* %s", b.displayName.c_str());
            } else {
                ImGui::Text("%s", b.displayName.c_str());
            }

            ImGui::SameLine();
            if (ImGui::SmallButton(isCurrent ? "Reload Map" : "Load Biome Map")) {
                if (world && player && camera && assetMgr) {
                    auto* cam = const_cast<graphics::Camera2DSystem*>(camera);
                    biomeMgr->SwitchBiome(b.id, *world, *player, *cam, *assetMgr, envAnim, 0);
                }
            }

            ImGui::TextDisabled("  Maps: %zu | Terrain: %zu tiles | Objects: %zu",
                                b.maps.size(), b.terrain.size(), b.environmentObjects.size());
            ImGui::PopID();
        }
    }

    // 6. Forest Biome Subregions & Landmark Teleport (Phase 12)
    if (ImGui::CollapsingHeader("Forest Biome Subregions & Teleport", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Expanded Forest Biome (160x120 tiles, 5120x3840 px)");
        ImGui::TextDisabled("Click any location to teleport character and inspect subregions.");

        struct Landmark {
            const char* name;
            const char* category;
            Vector2 pos;
        };

        const Landmark landmarks[] = {
            // Settlements
            { "Greenwood Hamlet (Town Hall & Well)", "Settlement", { 4280.0f, 750.0f } },
            { "Riverwood Outpost (Shop & Crossing)", "Settlement", { 3520.0f, 1750.0f } },
            { "Highland Homestead (Pioneer Cabin)", "Settlement", { 800.0f, 750.0f } },

            // Natural Features & Water
            { "Central Crossroads & Emerald Grove", "Heartland", { 2560.0f, 1800.0f } },
            { "Silver Lake & Sunken Islet", "Lake / Island", { 832.0f, 1792.0f } },
            { "Northern Ridge & Waterfall Cove", "Waterfall", { 2432.0f, 480.0f } },
            { "Serpentine River South Basin", "River", { 2800.0f, 3000.0f } },
            { "Sunlit Meadows & Rolling Hills", "Meadow", { 800.0f, 3000.0f } },
            { "Whispering Pines (Dense Woods)", "Dense Forest", { 1000.0f, 500.0f } },

            // Abandoned Structures
            { "Ruined Stone Watchtower", "Abandoned", { 4380.0f, 3180.0f } },
            { "Overgrown Cabin in Grove", "Abandoned", { 1920.0f, 480.0f } },
            { "Sunken Shrine (Silver Lake)", "Abandoned", { 830.0f, 1790.0f } },
            { "Destroyed Bridge & Smoldering Camp", "Abandoned", { 2950.0f, 2980.0f } },
            { "Overgrown Hermit's Shed", "Abandoned", { 4680.0f, 2080.0f } },

            // Hidden Locations
            { "Hidden: Secluded Waterfall Cove", "Secret", { 2400.0f, 460.0f } },
            { "Hidden: Concealed Meadow Clearing", "Secret", { 1350.0f, 850.0f } },
            { "Hidden: Smuggler's Rock Cache", "Secret", { 480.0f, 3300.0f } },
            { "Hidden: Ancient Woodland Altar", "Secret", { 3900.0f, 2700.0f } }
        };

        for (int i = 0; i < 18; ++i) {
            ImGui::PushID(i);
            if (ImGui::Button(landmarks[i].name)) {
                if (player) {
                    player->Spawn(landmarks[i].pos);
                }
            }
            ImGui::SameLine();
            ImGui::TextDisabled("[%s] (%.0f, %.0f)", landmarks[i].category, landmarks[i].pos.x, landmarks[i].pos.y);
            ImGui::PopID();
        }
    }

    // 6. Greenwood Village Buildings & Interiors (Phase 13)
    if (buildingMgr && ImGui::CollapsingHeader("Greenwood Village Buildings & Interiors", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Settlement: %s (%s)",
                           buildingMgr->GetVillageName().c_str(), buildingMgr->GetRegion().c_str());
        if (buildingMgr->IsIndoors()) {
            const auto* curr = buildingMgr->GetCurrentInteriorBuilding();
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Current Location: INDOORS -> %s", curr ? curr->name.c_str() : "Interior");
            if (ImGui::Button("Exit to Exterior (Village Square)") && world && player && camera && assetMgr) {
                auto* cam = const_cast<graphics::Camera2DSystem*>(camera);
                buildingMgr->ExitBuilding(*world, *player, *cam, *assetMgr);
            }
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Current Location: OUTDOORS (Forest Biome)");
        }
        ImGui::Separator();

        ImGui::Text("Total Buildings: %d (%d enterable interiors)",
                    buildingMgr->GetTotalBuildingCount(), buildingMgr->GetEnterableBuildingCount());

        const auto& buildings = buildingMgr->GetBuildings();
        for (size_t i = 0; i < buildings.size(); ++i) {
            const auto& b = buildings[i];
            ImGui::PushID(static_cast<int>(i + 1000));

            bool isCurrent = (buildingMgr->IsIndoors() && buildingMgr->GetCurrentInteriorId() == b.id);
            if (isCurrent) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "* %s [%s]", b.name.c_str(), b.district.c_str());
            } else {
                ImGui::Text("%s [%s]", b.name.c_str(), b.district.c_str());
            }

            ImGui::SameLine();
            if (ImGui::SmallButton("Teleport Outside")) {
                if (player) {
                    player->Spawn(b.entrancePosition);
                }
            }

            if (b.isEnterable) {
                ImGui::SameLine();
                if (!buildingMgr->IsIndoors()) {
                    if (ImGui::SmallButton("Enter Interior")) {
                        if (world && player && camera && assetMgr) {
                            auto* cam = const_cast<graphics::Camera2DSystem*>(camera);
                            buildingMgr->EnterBuilding(b.id, *world, *player, *cam, *assetMgr);
                        }
                    }
                }
            }

            ImGui::TextDisabled("  Type: %s | Door: (%.0f, %.0f) | Enterable: %s",
                                b.buildingType.c_str(), b.entrancePosition.x, b.entrancePosition.y,
                                b.isEnterable ? "YES" : "NO");

            ImGui::PopID();
        }
    }

    // 7. Battle Royale World Debugger (Phase 30)
    if (worldMgr && ImGui::CollapsingHeader("Battle Royale World Debugger (Phase 30)", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto& wdef = worldMgr->GetWorldDef();
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.0f), "World: %s [%s]", wdef.name.c_str(), wdef.worldId.c_str());
        ImGui::Text("Map Dimensions:    %d x %d tiles (%.0f x %.0f px)", wdef.widthTiles, wdef.heightTiles, wdef.worldWidth, wdef.worldHeight);

        Vector2 pPos = player ? player->GetPosition() : mouseWorld;
        const auto* curRegion = worldMgr->GetBiomeRegionAt(pPos);
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "Current Region:    %s (Biome: %s)",
                           curRegion ? curRegion->name.c_str() : "Ocean Coastline",
                           curRegion ? curRegion->biomeId.c_str() : "coast");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "World Entities & Metrics:");
        ImGui::BulletText("Settlements:       %zu major villages", worldMgr->GetSettlements().size());
        ImGui::BulletText("Strategic POIs:    %zu locations", worldMgr->GetPointsOfInterest().size());
        ImGui::BulletText("Spawn Regions:     %zu zones", worldMgr->GetSpawnRegions().size());
        ImGui::BulletText("Loot Containers:   %zu markers", worldMgr->GetLootLocations().size());
        ImGui::BulletText("Traversal Routes:  %zu primary highways", worldMgr->GetPrimaryRoutes().size());
        ImGui::BulletText("Chokepoints:       %zu bridges & passes", worldMgr->GetChokepoints().size());
        ImGui::BulletText("Landmarks:         %zu natural / ruins", worldMgr->GetLandmarks().size());

        // Validation status
        std::vector<std::string> validationErrors;
        bool isValid = worldMgr->ValidateWorldData(validationErrors);
        if (isValid) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[STATUS] World Data Integrity: PASSED (0 Errors)");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[STATUS] World Data Errors: %zu detected", validationErrors.size());
            for (const auto& err : validationErrors) {
                ImGui::BulletText("%s", err.c_str());
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Battle Royale World Overlays:");

        bool showPlayable = worldMgr->GetShowPlayableBounds();
        if (ImGui::Checkbox("Playable Boundary (Red)", &showPlayable)) worldMgr->SetShowPlayableBounds(showPlayable);
        ImGui::SameLine();
        bool showBiomeBounds = worldMgr->GetShowBiomeBounds();
        if (ImGui::Checkbox("Biome Bounds (Blue)", &showBiomeBounds)) worldMgr->SetShowBiomeBounds(showBiomeBounds);

        bool showSettlementBounds = worldMgr->GetShowSettlementBounds();
        if (ImGui::Checkbox("Settlement Circles (Gold)", &showSettlementBounds)) worldMgr->SetShowSettlementBounds(showSettlementBounds);
        ImGui::SameLine();
        bool showPois = worldMgr->GetShowPois();
        if (ImGui::Checkbox("POI Markers (Purple)", &showPois)) worldMgr->SetShowPois(showPois);

        bool showSpawns = worldMgr->GetShowSpawnRegions();
        if (ImGui::Checkbox("Spawn Markers (Green)", &showSpawns)) worldMgr->SetShowSpawnRegions(showSpawns);
        ImGui::SameLine();
        bool showLoot = worldMgr->GetShowLootMarkers();
        if (ImGui::Checkbox("Loot Markers (Yellow)", &showLoot)) worldMgr->SetShowLootMarkers(showLoot);

        bool showRoutes = worldMgr->GetShowRoutes();
        if (ImGui::Checkbox("Highway Routes (Yellow Line)", &showRoutes)) worldMgr->SetShowRoutes(showRoutes);
        ImGui::SameLine();
        bool showChokes = worldMgr->GetShowChokepoints();
        if (ImGui::Checkbox("Chokepoints (Red Circle)", &showChokes)) worldMgr->SetShowChokepoints(showChokes);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Island Navigation & Teleport Inspector:");

        // Load Island Map Button
        if (ImGui::Button("Load Battle Royale Island Map")) {
            if (world && player && camera && assetMgr) {
                world->LoadMap("maps/world/battle_royale_island.json", *assetMgr);
                player->Spawn(wdef.defaultSpawn);
                auto* cam = const_cast<graphics::Camera2DSystem*>(camera);
                cam->SetTarget(wdef.defaultSpawn);
            }
        }

        // Teleport to settlements
        ImGui::Text("Teleport to Settlement:");
        for (const auto& st : worldMgr->GetSettlements()) {
            if (ImGui::Button(st.name.c_str())) {
                if (player && camera) {
                    player->Spawn(st.center);
                    auto* cam = const_cast<graphics::Camera2DSystem*>(camera);
                    cam->SetTarget(st.center);
                }
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(%.0f, %.0f)", st.center.x, st.center.y);
        }

        // Teleport to POIs
        ImGui::Separator();
        ImGui::Text("Teleport to Strategic POI:");
        for (const auto& poi : worldMgr->GetPointsOfInterest()) {
            if (ImGui::Button(poi.name.c_str())) {
                if (player && camera) {
                    player->Spawn(poi.position);
                    auto* cam = const_cast<graphics::Camera2DSystem*>(camera);
                    cam->SetTarget(poi.position);
                }
            }
            ImGui::SameLine();
            ImGui::TextDisabled("[%s] (%.0f, %.0f)", poi.type.c_str(), poi.position.x, poi.position.y);
        }
    }
}

void DevUI::RenderAssetBrowser(resources::AssetManager* assetMgr) {
    if (!assetMgr) {
        ImGui::Text("AssetManager is not active.");
        return;
    }

    auto& registry = assetMgr->GetRegistry();
    auto allAssets = registry.GetAll();

    // Category filter
    auto categories = registry.GetCategories();
    std::vector<std::string> catFilterOptions;
    catFilterOptions.push_back("All");
    for (const auto& c : categories) catFilterOptions.push_back(c);

    if (ImGui::BeginCombo("Category", m_selectedCategory.c_str())) {
        for (const auto& opt : catFilterOptions) {
            bool isSelected = (m_selectedCategory == opt);
            if (ImGui::Selectable(opt.c_str(), isSelected)) {
                m_selectedCategory = opt;
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Search input
    ImGui::InputTextWithHint("Search ID", "Type to search...", m_searchBuffer, sizeof(m_searchBuffer));
    std::string searchFilter(m_searchBuffer);
    std::transform(searchFilter.begin(), searchFilter.end(), searchFilter.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    ImGui::Separator();

    // Split view: Left list, Right preview
    ImGui::Columns(2, "AssetBrowserColumns", true);
    ImGui::SetColumnWidth(0, 180.0f);

    // Left List
    ImGui::BeginChild("AssetListScroll", ImVec2(0, 260), true);
    for (const auto& meta : allAssets) {
        // Apply category filter
        if (m_selectedCategory != "All" && meta.category != m_selectedCategory) {
            continue;
        }

        // Apply search filter
        if (!searchFilter.empty()) {
            std::string idLower = meta.id;
            std::transform(idLower.begin(), idLower.end(), idLower.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (idLower.find(searchFilter) == std::string::npos) {
                continue;
            }
        }

        bool isSelected = (m_selectedAssetId == meta.id);
        ImVec4 color = (meta.status == resources::AssetStatus::Approved)
                       ? ImVec4(0.4f, 0.9f, 0.5f, 1.0f)
                       : ImVec4(0.9f, 0.7f, 0.3f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        if (ImGui::Selectable(meta.id.c_str(), isSelected)) {
            m_selectedAssetId = meta.id;
            m_previewFrameIndex = 0;
        }
        ImGui::PopStyleColor();

        if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right Details & Preview
    ImGui::BeginChild("AssetDetailScroll", ImVec2(0, 260), true);
    const resources::AssetMetadata* selectedMeta = registry.GetMetadata(m_selectedAssetId);
    if (selectedMeta) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "ID: %s", selectedMeta->id.c_str());
        ImGui::Text("File: %s", selectedMeta->file.c_str());
        ImGui::Text("Type: %s", std::string(resources::AssetMetadata::TypeToString(selectedMeta->type)).c_str());
        ImGui::Text("Category: %s", selectedMeta->category.c_str());
        ImGui::Text("License: %s", selectedMeta->license.c_str());
        ImGui::Text("Author: %s (%s)", selectedMeta->author.c_str(), selectedMeta->source.c_str());
        ImGui::Text("Commercial: %s | Attribution: %s",
                    selectedMeta->commercialUse ? "Yes" : "No",
                    selectedMeta->attributionRequired ? "Yes" : "No");

        // Texture Thumbnail Preview
        if (selectedMeta->type == resources::AssetType::Texture) {
            Texture2D tex = assetMgr->GetTexture(selectedMeta->id);
            if (tex.id != 0) {
                ImGui::Separator();
                ImGui::Text("Dimensions: %d x %d px", tex.width, tex.height);
                float previewScale = std::min(128.0f / tex.width, 128.0f / tex.height);
                if (previewScale <= 0.0f || previewScale > 4.0f) previewScale = 2.0f;
                int pw = static_cast<int>(tex.width * previewScale);
                int ph = static_cast<int>(tex.height * previewScale);
                rlImGuiImageSize(&tex, pw, ph);
            }
        }
        // Spritesheet Preview
        else if (selectedMeta->type == resources::AssetType::SpriteSheet) {
            const resources::SpriteSheet* sheet = assetMgr->GetSpriteSheet(selectedMeta->id);
            if (sheet) {
                Texture2D tex = assetMgr->GetTexture(sheet->GetTextureId());
                ImGui::Separator();
                ImGui::Text("Frames: %d (%d x %d)", sheet->GetFrameCount(), sheet->GetFrameWidth(), sheet->GetFrameHeight());
                if (sheet->GetFrameCount() > 1) {
                    ImGui::SliderInt("Frame", &m_previewFrameIndex, 0, sheet->GetFrameCount() - 1);
                }
                if (tex.id != 0) {
                    Rectangle srcRect = sheet->GetFrameRect(m_previewFrameIndex);
                    rlImGuiImageRect(&tex, sheet->GetFrameWidth() * 2, sheet->GetFrameHeight() * 2, srcRect);
                }
            }
        }
    } else {
        ImGui::TextDisabled("Select an asset on the left to inspect.");
    }
    ImGui::EndChild();

    ImGui::Columns(1);
}

void DevUI::RenderAssetValidation(resources::AssetManager* assetMgr) {
    if (!assetMgr) return;

    if (ImGui::Button("Run Asset Validation Scan")) {
        // Trigger scan
    }

    resources::ValidationReport report = assetMgr->ValidateAssets();
    ImGui::Separator();

    ImGui::Text("Total Registered Assets: %zu", report.totalAssetsChecked);
    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Passed: %zu", report.passedCount);
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Warnings: %zu", report.warningsCount);
    ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Errors: %zu", report.errorsCount);
    ImGui::Separator();

    ImGui::BeginChild("ValidationIssuesList", ImVec2(0, 200), true);
    if (report.issues.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "All manifest assets verified clean and valid.");
    } else {
        for (const auto& issue : report.issues) {
            if (issue.severity == resources::ValidationSeverity::Error) {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "[ERROR] %s: %s", issue.assetId.c_str(), issue.message.c_str());
            } else if (issue.severity == resources::ValidationSeverity::Warning) {
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "[WARN] %s: %s", issue.assetId.c_str(), issue.message.c_str());
            } else {
                ImGui::Text("[INFO] %s: %s", issue.assetId.c_str(), issue.message.c_str());
            }
        }
    }
    ImGui::EndChild();
}

void DevUI::RenderPlayerDebug(entities::Player* player) {
    if (!player) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Player entity is not active.");
        return;
    }

    // 1. Transform & Motion
    if (ImGui::CollapsingHeader("Transform & Motion", ImGuiTreeNodeFlags_DefaultOpen)) {
        Vector2 pos = player->GetPosition();
        Vector2 vel = player->GetVelocity();
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

        ImGui::Text("Position (Feet): (%.2f, %.2f)", pos.x, pos.y);
        ImGui::Text("Velocity:        (%.2f, %.2f)", vel.x, vel.y);
        ImGui::Text("Current Speed:   %.1f px/s", speed);

        const auto& controller = player->GetController();
        Vector2 inputDir = controller.GetMovementDirection();
        ImGui::Text("Input Vector:    (%.2f, %.2f) [%s]",
                    inputDir.x, inputDir.y, controller.IsMoving() ? "MOVING" : "IDLE");
        ImGui::Text("Running Status:  %s", controller.IsRunning() ? "ACTIVE (Shift)" : "WALKING");
    }

    // 2. Animation State Machine
    if (ImGui::CollapsingHeader("Animation State Machine", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto& animCtrl = player->GetAnimController();
        ImGui::Text("State:           %s", graphics::animation::MovementStateToString(player->GetMovementState()).data());
        ImGui::Text("Facing:          %s", graphics::animation::DirectionToString(player->GetFacing()).data());
        ImGui::Text("Current Clip:    %s", animCtrl.GetCurrentClipName().c_str());
        ImGui::Text("Clip Frame:      #%d", animCtrl.GetCurrentFrameIndex());
        ImGui::Text("Sheet Frame GID: #%d", animCtrl.GetSpritesheetFrameIndex());
        ImGui::Text("Elapsed Time:    %.3f s", animCtrl.GetElapsedTime());
        ImGui::Text("Playback Speed:  %.2fx", animCtrl.GetPlaybackSpeed());
    }

    // 3. Collision Bounds
    if (ImGui::CollapsingHeader("Collision Bounds", ImGuiTreeNodeFlags_DefaultOpen)) {
        Rectangle col = player->GetCollisionBounds();
        const auto& cfg = player->GetCollisionConfig();
        ImGui::Text("Status:          %s", cfg.enabled ? "ENABLED" : "DISABLED");
        ImGui::Text("Bounds Rect:     [%.1f, %.1f, %.1f, %.1f]", col.x, col.y, col.width, col.height);
        ImGui::Text("Dimensions:      %.1f x %.1f px", cfg.width, cfg.height);
        ImGui::Text("Offset:          (%.1f, %.1f)", cfg.offsetX, cfg.offsetY);
    }

    // 4. Movement Tuning & Configuration
    if (ImGui::CollapsingHeader("Movement Tuning", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& stats = player->GetStats();
        ImGui::SliderFloat("Walk Speed", &stats.walkSpeed, 30.0f, 250.0f, "%.1f px/s");
        ImGui::SliderFloat("Run Speed", &stats.runSpeed, 60.0f, 350.0f, "%.1f px/s");
        ImGui::Checkbox("Normalize Diagonal", &stats.normalizeDiagonal);

        auto& ctrl = player->GetController();
        ctrl.SetNormalizeDiagonal(stats.normalizeDiagonal);
    }
}

void DevUI::RenderAnimationHUD(entities::Player* player) {
    if (!player) return;

    ImGui::SetNextWindowPos(ImVec2(16, 50), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.70f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                            ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("Animation HUD (F5)", nullptr, flags)) {
        const auto& animCtrl = player->GetAnimController();
        Vector2 vel = player->GetVelocity();
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "PLAYER ANIMATION HUD (F5)");
        ImGui::Separator();
        ImGui::Text("State:  %s (%s)",
                    graphics::animation::MovementStateToString(player->GetMovementState()).data(),
                    graphics::animation::DirectionToString(player->GetFacing()).data());
        ImGui::Text("Clip:   %s", animCtrl.GetCurrentClipName().c_str());
        ImGui::Text("Frame:  #%d (Sheet #%d)", animCtrl.GetCurrentFrameIndex(), animCtrl.GetSpritesheetFrameIndex());
        ImGui::Text("Speed:  %.1f px/s (Anim %.2fx)", speed, animCtrl.GetPlaybackSpeed());
    }
    ImGui::End();
}

void DevUI::RenderCombatAndAI(combat::CombatSystem* combatSys, world::EnemySpawner* spawner, entities::Player* player, ai::AIManager* aiMgr) {
    if (!combatSys) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "CombatSystem is not active.");
        return;
    }

    // 1. Player Combat Stats
    if (ImGui::CollapsingHeader("Player Combat Status", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (player) {
            auto& hp = player->GetHealth();
            float hpPct = hp.GetHealthPercent();
            char hpBuf[32];
            snprintf(hpBuf, sizeof(hpBuf), "%.1f / %.1f HP", hp.GetCurrentHealth(), hp.GetMaxHealth());

            ImVec4 barColor = hpPct > 0.5f ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f) :
                              (hpPct > 0.2f ? ImVec4(0.9f, 0.7f, 0.2f, 1.0f) : ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
            ImGui::ProgressBar(hpPct, ImVec2(-1, 20), hpBuf);
            ImGui::PopStyleColor();

            ImGui::Text("State: %s", hp.IsAlive() ? "ALIVE" : "FALLEN IN BATTLE");
            ImGui::Text("Faction: %s", combat::FactionToString(player->GetFaction()).c_str());

            const auto& atkSys = player->GetAttackSystem();
            float cd = atkSys.GetCooldownRemaining();
            ImGui::Text("Primary Attack: [slash] | Cooldown: %.2fs (%s)", cd, cd <= 0.0f ? "READY" : "COOLING DOWN");

            if (ImGui::Button("Heal Player (+25 HP)")) {
                hp.ApplyHeal(25.0f);
            }
            ImGui::SameLine();
            if (ImGui::Button("Damage Player (-20 HP)")) {
                combat::DamageRequest req;
                req.sourceId = 0;
                req.targetId = player->GetCombatantId();
                req.damageType = combat::DamageType::Environmental;
                req.baseDamage = 20.0f;
                combatSys->GetDamageSystem().ApplyDamage(req, player, &combatSys->GetEvents());
            }
            ImGui::SameLine();
            if (ImGui::Button("Revive Player")) {
                hp.Reset();
            }
        }
    }

    // 2. Combat System Rules & Metrics
    if (ImGui::CollapsingHeader("Combat Rules & Hitboxes", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& rules = combatSys->GetRules();
        ImGui::Checkbox("PvE Enabled", &rules.allowPvE);
        ImGui::SameLine();
        ImGui::Checkbox("PvP Enabled", &rules.allowPvP);
        ImGui::SameLine();
        ImGui::Checkbox("Friendly Fire", &rules.friendlyFire);

        ImGui::Text("Registered Combatants: %zu", combatSys->GetAllCombatants().size());
        ImGui::Text("Active Hitbox Overlay Elements: %zu (F6 to toggle)", combatSys->GetDebugHits().size());
        if (ImGui::Button("Clear Hitbox Overlays")) {
            combatSys->ClearDebugHits();
        }
    }

    // 3. Enemy Spawner & Active AI Entities
    if (ImGui::CollapsingHeader("Enemy Spawner & AI Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (spawner) {
            bool spawnerEnabled = spawner->IsEnabled();
            if (ImGui::Checkbox("Spawner Enabled (F7)", &spawnerEnabled)) {
                spawner->SetEnabled(spawnerEnabled);
            }
            ImGui::SameLine();
            ImGui::Text("| Active: %zu / %zu", spawner->GetActiveEnemyCount(), spawner->GetConfig().maxActiveEnemies);
            ImGui::Text("Next Spawn Interval: %.1fs (Timer: %.1fs)", spawner->GetConfig().spawnInterval, spawner->GetSpawnTimer());

            if (player) {
                Vector2 pPos = player->GetPosition();
                if (ImGui::Button("Spawn Goblin Nearby")) {
                    spawner->SpawnEnemy("goblin", Vector2{ pPos.x + 80.0f, pPos.y }, *combatSys);
                }
                ImGui::SameLine();
                if (ImGui::Button("Spawn Skeleton Nearby")) {
                    spawner->SpawnEnemy("skeleton", Vector2{ pPos.x - 80.0f, pPos.y }, *combatSys);
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear All Enemies")) {
                    spawner->ClearAllEnemies(*combatSys);
                }
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.5f, 1.0f), "Active Enemies Table:");

            if (ImGui::BeginTable("EnemiesTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableSetupColumn("AI State", ImGuiTableColumnFlags_WidthFixed, 80);
                ImGui::TableSetupColumn("Health", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, 90);
                ImGui::TableSetupColumn("Target", ImGuiTableColumnFlags_WidthFixed, 50);
                ImGui::TableHeadersRow();

                for (const auto& e : spawner->GetEnemies()) {
                    if (!e) continue;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%u", e->GetCombatantId());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", e->GetName().c_str());

                    ImGui::TableSetColumnIndex(2);
                    std::string stateStr = entities::AIStateToString(e->GetAIState());
                    ImVec4 sCol = (e->GetAIState() == entities::AIState::Dead) ? ImVec4(0.6f, 0.6f, 0.6f, 1.0f) :
                                  (e->GetAIState() == entities::AIState::Attack ? ImVec4(0.9f, 0.3f, 0.3f, 1.0f) :
                                  (e->GetAIState() == entities::AIState::Chase ? ImVec4(0.9f, 0.7f, 0.2f, 1.0f) : ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
                    ImGui::TextColored(sCol, "%s", stateStr.c_str());

                    ImGui::TableSetColumnIndex(3);
                    float eHp = e->GetHealth().GetCurrentHealth();
                    float eMax = e->GetHealth().GetMaxHealth();
                    char eBuf[24];
                    snprintf(eBuf, sizeof(eBuf), "%.0f/%.0f", eHp, eMax);
                    ImGui::ProgressBar(e->GetHealth().GetHealthPercent(), ImVec2(-1, 14), eBuf);

                    ImGui::TableSetColumnIndex(4);
                    Vector2 pos = e->GetPosition();
                    ImGui::Text("%.0f, %.0f", pos.x, pos.y);

                    ImGui::TableSetColumnIndex(5);
                    if (e->GetTargetId() != 0) {
                        ImGui::Text("#%u", e->GetTargetId());
                    } else {
                        ImGui::TextDisabled("-");
                    }
                }
                ImGui::EndTable();
            }
        }
    }

    // 4. AI & Navigation Architecture (Phase 29)
    if (ImGui::CollapsingHeader("AI & Navigation Architecture (Phase 29)", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (!aiMgr) {
            ImGui::TextDisabled("AIManager instance not attached.");
        } else {
            auto& cfg = aiMgr->GetConfig();
            ImGui::Checkbox("AI Simulation Enabled", &cfg.aiEnabled);
            ImGui::SameLine();
            bool isPaused = aiMgr->IsPaused();
            if (ImGui::Checkbox("Pause AI", &isPaused)) {
                aiMgr->SetPaused(isPaused);
            }
            ImGui::SameLine();
            if (ImGui::Button("Step 1 Frame")) {
                aiMgr->StepSingleFrame();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload AI Configs")) {
                aiMgr->ReloadConfigurations();
            }

            // Navigation Grid Telemetry
            const auto& grid = aiMgr->GetNavigationGrid();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Navigation Grid:");
            ImGui::Text("  Resolution: %dx%d cells (%.1fpx cell size)", grid.GetWidthInCells(), grid.GetHeightInCells(), grid.GetCellSize());
            ImGui::Text("  Total Cells: %zu | Blocked: %zu (%.1f%%)",
                        grid.GetTotalCellCount(),
                        grid.GetBlockedCellCount(),
                        grid.GetTotalCellCount() > 0 ? (100.0f * grid.GetBlockedCellCount() / grid.GetTotalCellCount()) : 0.0f);

            // Overlay visualization checkboxes
            ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.4f, 1.0f), "Visual Overlays (In-Game World):");
            ImGui::Checkbox("Show Nav Grid", &cfg.showNavGrid);
            ImGui::SameLine();
            ImGui::Checkbox("Show Paths", &cfg.showPaths);
            ImGui::SameLine();
            ImGui::Checkbox("Show FOV", &cfg.showFOV);
            ImGui::SameLine();
            ImGui::Checkbox("Show Avoidance", &cfg.showAvoidance);

            // Active AI Controller Inspector
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.5f, 1.0f), "AI Controller Inspector (%zu Active):", aiMgr->GetActiveControllerCount());

            for (const auto& [eId, ctrl] : aiMgr->GetControllers()) {
                if (!ctrl) continue;
                char headerBuf[64];
                snprintf(headerBuf, sizeof(headerBuf), "Entity #%u [%s] - %s",
                         eId,
                         ctrl->GetProfile().archetype.c_str(),
                         wildbound::entities::AIStateToString(ctrl->GetState()).c_str());

                if (ImGui::TreeNode(headerBuf)) {
                    ImGui::Text("AI State: %s", wildbound::entities::AIStateToString(ctrl->GetState()).c_str());
                    ImGui::Text("Archetype: %s | Aggression: %.1f", ctrl->GetProfile().archetype.c_str(), ctrl->GetProfile().aggressionLevel);
                    ImGui::Text("Target ID: #%u", ctrl->GetTargetId());
                    Vector2 tPos = ctrl->GetTargetPosition();
                    ImGui::Text("Target Pos: (%.1f, %.1f)", tPos.x, tPos.y);

                    const auto& mem = ctrl->GetTargetMemory();
                    ImGui::Text("Memory: %s (Last seen %.1fs ago at (%.1f, %.1f))",
                                mem.HasActiveMemory() ? "ACTIVE" : "NONE",
                                mem.GetTimeSinceLastSeen(),
                                mem.GetLastKnownPosition().x,
                                mem.GetLastKnownPosition().y);

                    const auto& mv = ctrl->GetMovementController();
                    ImGui::Text("Waypoints: %zu / %zu (Has Path: %s, Arrived: %s)",
                                mv.GetCurrentWaypointIndex(),
                                mv.GetTotalWaypoints(),
                                mv.HasActivePath() ? "YES" : "NO",
                                mv.HasReachedDestination() ? "YES" : "NO");

                    const auto& stuck = ctrl->GetStuckDetector();
                    ImGui::Text("Stuck: %s (Retries: %d)", stuck.IsStuck() ? "STUCK" : "CLEAR", stuck.GetRetryCount());

                    // Manual testing controls
                    if (ImGui::Button("Force Idle")) {
                        ctrl->SetState(wildbound::entities::AIState::Idle);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Force Return")) {
                        ctrl->SetState(wildbound::entities::AIState::Return);
                    }
                    ImGui::SameLine();
                    if (player && ImGui::Button("Force Chase Player")) {
                        ctrl->ForceTarget(player->GetCombatantId(), player->GetPosition());
                    }

                    ImGui::TreePop();
                }
            }
        }
    }
}

void DevUI::RenderWardrobe(entities::Player* player, resources::AssetManager* assetMgr, customization::CharacterCustomizationManager* customMgr) {
    if (!player) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Player entity not available.");
        return;
    }

    auto& modularChar = player->GetModularAppearance();
    auto& equip = modularChar.GetEquipment();

    ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.3f, 1.0f), "Modular Character Wardrobe (Phase 17: 50 Costumes & Armor + Phase 8)");
    ImGui::Text("Full real-time customization suite supporting modular clothing, armor, and accessories.");
    ImGui::Separator();

    // 1. Representative Configurations
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Representative Test Configurations:");
    
    // (1) Default Character
    if (ImGui::Button("1. Default")) {
        modularChar.ApplyDefaultAppearance();
    }
    ImGui::SameLine();

    // (2) Explorer Appearance
    if (ImGui::Button("2. Explorer")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "hair_messy_001", "Messy Explorer", entities::CosmeticCategory::Hair, "sheet_hair_messy_001" });
        equip.Equip(entities::CosmeticItem{ "shirt_leather_001", "Leather Jacket", entities::CosmeticCategory::Shirt, "sheet_shirt_leather_001" });
        equip.Equip(entities::CosmeticItem{ "pants_traveler_001", "Traveler Pants", entities::CosmeticCategory::Pants, "sheet_pants_traveler_001" });
        equip.Equip(entities::CosmeticItem{ "shoes_boots_001", "Leather Boots", entities::CosmeticCategory::Shoes, "sheet_shoes_boots_001" });
        equip.Equip(entities::CosmeticItem{ "hat_adventurer_001", "Adventurer Hat", entities::CosmeticCategory::Headwear, "sheet_hat_adventurer_001" });
        equip.Equip(entities::CosmeticItem{ "goggles_adventure_001", "Adventurer Goggles", entities::CosmeticCategory::Goggles, "sheet_goggles_adventure_001" });
        equip.Equip(entities::CosmeticItem{ "backpack_leather_001", "Leather Backpack", entities::CosmeticCategory::Back, "sheet_backpack_leather_001" });
    }
    ImGui::SameLine();

    // (3) Armored Appearance
    if (ImGui::Button("3. Armored")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "shirt_armor_001", "Light Armor", entities::CosmeticCategory::Shirt, "sheet_shirt_armor_001" });
        equip.Equip(entities::CosmeticItem{ "pants_armored_001", "Armored Pants", entities::CosmeticCategory::Pants, "sheet_pants_armored_001" });
        equip.Equip(entities::CosmeticItem{ "shoes_armored_001", "Armored Boots", entities::CosmeticCategory::Shoes, "sheet_shoes_armored_001" });
        entities::CosmeticItem helm{ "hat_helmet_001", "Iron Helmet", entities::CosmeticCategory::Headwear, "sheet_hat_helmet_001" };
        helm.occlusionFlags = { "hair_hide_all" };
        equip.Equip(helm);
        equip.Equip(entities::CosmeticItem{ "mask_leather_001", "Leather Mask", entities::CosmeticCategory::Mask, "sheet_mask_leather_001" });
        equip.Equip(entities::CosmeticItem{ "backpack_adventure_001", "Adventure Pack", entities::CosmeticCategory::Back, "sheet_backpack_adventure_001" });
    }
    ImGui::SameLine();

    // (4) Mystic Appearance
    if (ImGui::Button("4. Mystic")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "hair_long_001", "Long Wanderer", entities::CosmeticCategory::Hair, "sheet_hair_long_001" });
        equip.Equip(entities::CosmeticItem{ "shirt_mage_001", "Mystic Robe", entities::CosmeticCategory::Shirt, "sheet_shirt_mage_001" });
        equip.Equip(entities::CosmeticItem{ "pants_mage_001", "Mystic Trousers", entities::CosmeticCategory::Pants, "sheet_pants_mage_001" });
        equip.Equip(entities::CosmeticItem{ "shoes_light_001", "Light Shoes", entities::CosmeticCategory::Shoes, "sheet_shoes_light_001" });
        equip.Equip(entities::CosmeticItem{ "hat_wizard_001", "Mystic Hat", entities::CosmeticCategory::Headwear, "sheet_hat_wizard_001" });
        entities::CosmeticItem mask{ "mask_mystic_001", "Mystic Mask", entities::CosmeticCategory::Mask, "sheet_mask_mystic_001" };
        mask.occlusionFlags = { "mask_hide_goggles" };
        equip.Equip(mask);
    }
    ImGui::SameLine();

    // (5) Forest Guardian (Phase 17)
    if (ImGui::Button("5. Forest Guardian")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "var_skin_olive_001", "Olive Skin", entities::CosmeticCategory::Skin, "sheet_var_skin_olive_001" });
        equip.Equip(entities::CosmeticItem{ "var_hair_braided_red_001", "Braided Red", entities::CosmeticCategory::Hair, "sheet_var_hair_braided_red_001" });
        equip.Equip(entities::CosmeticItem{ "clo_forest_tunic_001", "Forest Tunic", entities::CosmeticCategory::Shirt, "sheet_clo_forest_tunic_001" });
        equip.Equip(entities::CosmeticItem{ "arm_forest_guardian_001", "Guardian Armor", entities::CosmeticCategory::Armor, "sheet_arm_forest_guardian_001" });
        equip.Equip(entities::CosmeticItem{ "clo_p_traveler_pants_001", "Traveler Pants", entities::CosmeticCategory::Pants, "sheet_clo_p_traveler_pants_001" });
        equip.Equip(entities::CosmeticItem{ "sho_leather_boots_001", "Leather Boots", entities::CosmeticCategory::Shoes, "sheet_sho_leather_boots_001" });
        equip.Equip(entities::CosmeticItem{ "hat_forest_ranger_hood_001", "Ranger Hood", entities::CosmeticCategory::Headwear, "sheet_hat_forest_ranger_hood_001" });
        equip.Equip(entities::CosmeticItem{ "var_face_warpaint_001", "Azure Warpaint", entities::CosmeticCategory::Face, "sheet_var_face_warpaint_001" });
        equip.Equip(entities::CosmeticItem{ "bag_explorer_backpack_001", "Explorer Pack", entities::CosmeticCategory::Back, "sheet_bag_explorer_backpack_001" });
    }
    ImGui::SameLine();

    // (6) Desert Nomad (Phase 17)
    if (ImGui::Button("6. Desert Nomad")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "var_skin_dark_001", "Umber Skin", entities::CosmeticCategory::Skin, "sheet_var_skin_dark_001" });
        equip.Equip(entities::CosmeticItem{ "clo_desert_robe_001", "Desert Robe", entities::CosmeticCategory::Shirt, "sheet_clo_desert_robe_001" });
        equip.Equip(entities::CosmeticItem{ "arm_desert_armor_001", "Desert Armor", entities::CosmeticCategory::Armor, "sheet_arm_desert_armor_001" });
        equip.Equip(entities::CosmeticItem{ "clo_p_desert_trousers_001", "Desert Trousers", entities::CosmeticCategory::Pants, "sheet_clo_p_desert_trousers_001" });
        equip.Equip(entities::CosmeticItem{ "sho_desert_sandals_001", "Desert Sandals", entities::CosmeticCategory::Shoes, "sheet_sho_desert_sandals_001" });
        equip.Equip(entities::CosmeticItem{ "hat_adventurer_hat_001", "Adventurer Hat", entities::CosmeticCategory::Headwear, "sheet_hat_adventurer_hat_001" });
        equip.Equip(entities::CosmeticItem{ "fac_cloth_mask_001", "Cloth Mask", entities::CosmeticCategory::Mask, "sheet_fac_cloth_mask_001" });
        equip.Equip(entities::CosmeticItem{ "fac_adventurer_goggles_001", "Adventurer Goggles", entities::CosmeticCategory::Goggles, "sheet_fac_adventurer_goggles_001" });
        equip.Equip(entities::CosmeticItem{ "bag_leather_backpack_001", "Leather Pack", entities::CosmeticCategory::Back, "sheet_bag_leather_backpack_001" });
    }
    ImGui::SameLine();

    // (7) Frost Knight (Phase 17)
    if (ImGui::Button("7. Frost Knight")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "var_skin_pale_001", "Pale Skin", entities::CosmeticCategory::Skin, "sheet_var_skin_pale_001" });
        equip.Equip(entities::CosmeticItem{ "clo_snow_jacket_001", "Snow Jacket", entities::CosmeticCategory::Shirt, "sheet_clo_snow_jacket_001" });
        equip.Equip(entities::CosmeticItem{ "arm_frost_armor_001", "Frost Armor", entities::CosmeticCategory::Armor, "sheet_arm_frost_armor_001" });
        equip.Equip(entities::CosmeticItem{ "clo_p_snow_trousers_001", "Snow Trousers", entities::CosmeticCategory::Pants, "sheet_clo_p_snow_trousers_001" });
        equip.Equip(entities::CosmeticItem{ "sho_snow_boots_001", "Snow Boots", entities::CosmeticCategory::Shoes, "sheet_sho_snow_boots_001" });
        entities::CosmeticItem helm{ "hat_steel_helmet_001", "Steel Helm", entities::CosmeticCategory::Headwear, "sheet_hat_steel_helmet_001" };
        helm.occlusionFlags = { "hair_hide_all" };
        equip.Equip(helm);
        equip.Equip(entities::CosmeticItem{ "bag_military_backpack_001", "Military Pack", entities::CosmeticCategory::Back, "sheet_bag_military_backpack_001" });
    }
    ImGui::SameLine();

    // (8) Ancient Runebound (Phase 17)
    if (ImGui::Button("8. Runebound")) {
        equip.Clear();
        equip.Equip(entities::CosmeticItem{ "var_hair_spiky_black_001", "Spiky Hair", entities::CosmeticCategory::Hair, "sheet_var_hair_spiky_black_001" });
        equip.Equip(entities::CosmeticItem{ "clo_mystic_robe_001", "Mystic Robe", entities::CosmeticCategory::Shirt, "sheet_clo_mystic_robe_001" });
        equip.Equip(entities::CosmeticItem{ "arm_ancient_rune_001", "Rune Armor", entities::CosmeticCategory::Armor, "sheet_arm_ancient_rune_001" });
        equip.Equip(entities::CosmeticItem{ "clo_p_mystic_pants_001", "Mystic Pants", entities::CosmeticCategory::Pants, "sheet_clo_p_mystic_pants_001" });
        equip.Equip(entities::CosmeticItem{ "sho_traveler_shoes_001", "Traveler Shoes", entities::CosmeticCategory::Shoes, "sheet_sho_traveler_shoes_001" });
        equip.Equip(entities::CosmeticItem{ "hat_mystic_hat_001", "Mystic Hat", entities::CosmeticCategory::Headwear, "sheet_hat_mystic_hat_001" });
        equip.Equip(entities::CosmeticItem{ "var_face_warpaint_001", "Azure Warpaint", entities::CosmeticCategory::Face, "sheet_var_face_warpaint_001" });
        equip.Equip(entities::CosmeticItem{ "bag_small_traveler_001", "Small Pack", entities::CosmeticCategory::Back, "sheet_bag_small_traveler_001" });
    }
    ImGui::SameLine();

    if (ImGui::Button("Strip All")) {
        equip.Clear();
    }
    ImGui::Separator();

    // 2. Facing Direction & Animation Controls
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Preview Facing Direction:");
    if (ImGui::Button("Down (South)")) {
        player->SetFacing(graphics::animation::Direction::Down);
    }
    ImGui::SameLine();
    if (ImGui::Button("Up (North)")) {
        player->SetFacing(graphics::animation::Direction::Up);
    }
    ImGui::SameLine();
    if (ImGui::Button("Left (West)")) {
        player->SetFacing(graphics::animation::Direction::Left);
    }
    ImGui::SameLine();
    if (ImGui::Button("Right (East)")) {
        player->SetFacing(graphics::animation::Direction::Right);
    }
    ImGui::Separator();

    // 3. Category Slot Equipment Selectors (Phase 17 + Phase 8 Cosmetics)
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Equipped Layers (All 50 Phase 17 + Phase 8 Cosmetics):");

    auto drawSlot = [&](const char* label, entities::CosmeticCategory cat,
                        const std::vector<std::pair<std::string, std::pair<std::string, std::vector<std::string>>>>& options) {
        const entities::CosmeticItem* current = equip.GetEquipped(cat);
        std::string currentId = current ? current->id : "None";

        if (ImGui::BeginCombo(label, current ? current->name.c_str() : "None")) {
            if (ImGui::Selectable("None", !current)) {
                equip.Unequip(cat);
            }
            for (const auto& opt : options) {
                bool isSelected = (current && current->id == opt.first);
                if (ImGui::Selectable(opt.first.c_str(), isSelected)) {
                    entities::CosmeticItem item;
                    item.id = opt.first;
                    item.name = opt.first;
                    item.category = cat;
                    item.spritesheetId = opt.second.first;
                    item.occlusionFlags = opt.second.second;
                    equip.Equip(item);
                }
            }
            ImGui::EndCombo();
        }
    };

    drawSlot("Skin Tone / Variation", entities::CosmeticCategory::Skin, {
        { "var_skin_pale_001",  { "sheet_var_skin_pale_001", {} } },
        { "var_skin_dark_001",  { "sheet_var_skin_dark_001", {} } },
        { "var_skin_olive_001", { "sheet_var_skin_olive_001", {} } },
        { "skin_default",       { "sheet_skin_default", {} } },
        { "skin_tan",           { "sheet_skin_tan", {} } }
    });

    drawSlot("Hairstyle / Variations", entities::CosmeticCategory::Hair, {
        { "var_hair_braided_red_001", { "sheet_var_hair_braided_red_001", {} } },
        { "var_hair_spiky_black_001", { "sheet_var_hair_spiky_black_001", {} } },
        { "hair_short_001",           { "sheet_hair_short_001", {} } },
        { "hair_long_001",            { "sheet_hair_long_001", {} } },
        { "hair_messy_001",           { "sheet_hair_messy_001", {} } },
        { "hair_ponytail_001",        { "sheet_hair_ponytail_001", {} } },
        { "hair_curly_001",           { "sheet_hair_curly_001", {} } }
    });

    drawSlot("Clothing / Shirt (8 P17)", entities::CosmeticCategory::Shirt, {
        { "clo_basic_traveler_001", { "sheet_clo_basic_traveler_001", {} } },
        { "clo_forest_tunic_001",   { "sheet_clo_forest_tunic_001", {} } },
        { "clo_desert_robe_001",    { "sheet_clo_desert_robe_001", {} } },
        { "clo_snow_jacket_001",    { "sheet_clo_snow_jacket_001", {} } },
        { "clo_leather_jacket_001", { "sheet_clo_leather_jacket_001", {} } },
        { "clo_mystic_robe_001",    { "sheet_clo_mystic_robe_001", {} } },
        { "clo_royal_outfit_001",   { "sheet_clo_royal_outfit_001", {} } },
        { "clo_dark_adventurer_001", { "sheet_clo_dark_adventurer_001", {} } },
        { "shirt_tunic_001",        { "sheet_shirt_tunic_001", {} } },
        { "shirt_leather_001",      { "sheet_shirt_leather_001", {} } },
        { "shirt_armor_001",        { "sheet_shirt_armor_001", {} } },
        { "shirt_traveler_001",     { "sheet_shirt_traveler_001", {} } },
        { "shirt_mage_001",         { "sheet_shirt_mage_001", {} } }
    });

    drawSlot("Armor / Cuirass (8 P17)", entities::CosmeticCategory::Armor, {
        { "arm_leather_armor_001",    { "sheet_arm_leather_armor_001", {} } },
        { "arm_reinforced_leather_001", { "sheet_arm_reinforced_leather_001", {} } },
        { "arm_iron_armor_001",       { "sheet_arm_iron_armor_001", {} } },
        { "arm_steel_armor_001",      { "sheet_arm_steel_armor_001", {} } },
        { "arm_forest_guardian_001",  { "sheet_arm_forest_guardian_001", {} } },
        { "arm_desert_armor_001",     { "sheet_arm_desert_armor_001", {} } },
        { "arm_frost_armor_001",      { "sheet_arm_frost_armor_001", {} } },
        { "arm_ancient_rune_001",     { "sheet_arm_ancient_rune_001", {} } }
    });

    drawSlot("Pants / Legs (6 P17)", entities::CosmeticCategory::Pants, {
        { "clo_p_traveler_pants_001", { "sheet_clo_p_traveler_pants_001", {} } },
        { "clo_p_leather_pants_001",  { "sheet_clo_p_leather_pants_001", {} } },
        { "clo_p_armored_pants_001",  { "sheet_clo_p_armored_pants_001", {} } },
        { "clo_p_desert_trousers_001", { "sheet_clo_p_desert_trousers_001", {} } },
        { "clo_p_snow_trousers_001",  { "sheet_clo_p_snow_trousers_001", {} } },
        { "clo_p_mystic_pants_001",   { "sheet_clo_p_mystic_pants_001", {} } },
        { "pants_traveler_001",       { "sheet_pants_traveler_001", {} } },
        { "pants_leather_001",        { "sheet_pants_leather_001", {} } },
        { "pants_armored_001",        { "sheet_pants_armored_001", {} } },
        { "pants_short_001",          { "sheet_pants_short_001", {} } },
        { "pants_mage_001",           { "sheet_pants_mage_001", {} } }
    });

    drawSlot("Shoes / Boots (5 P17)", entities::CosmeticCategory::Shoes, {
        { "sho_leather_boots_001",  { "sheet_sho_leather_boots_001", {} } },
        { "sho_iron_boots_001",     { "sheet_sho_iron_boots_001", {} } },
        { "sho_traveler_shoes_001", { "sheet_sho_traveler_shoes_001", {} } },
        { "sho_snow_boots_001",     { "sheet_sho_snow_boots_001", {} } },
        { "sho_desert_sandals_001", { "sheet_sho_desert_sandals_001", {} } },
        { "shoes_boots_001",        { "sheet_shoes_boots_001", {} } },
        { "shoes_armored_001",      { "sheet_shoes_armored_001", {} } },
        { "shoes_light_001",        { "sheet_shoes_light_001", {} } }
    });

    drawSlot("Headwear / Hats (6 P17)", entities::CosmeticCategory::Headwear, {
        { "hat_adventurer_hat_001",     { "sheet_hat_adventurer_hat_001", {} } },
        { "hat_explorer_hood_001",      { "sheet_hat_explorer_hood_001", {} } },
        { "hat_iron_helmet_001",        { "sheet_hat_iron_helmet_001", { "hair_hide_all" } } },
        { "hat_steel_helmet_001",       { "sheet_hat_steel_helmet_001", { "hair_hide_all" } } },
        { "hat_mystic_hat_001",         { "sheet_hat_mystic_hat_001", {} } },
        { "hat_forest_ranger_hood_001", { "sheet_hat_forest_ranger_hood_001", {} } },
        { "hat_adventurer_001",         { "sheet_hat_adventurer_001", {} } },
        { "hat_hood_001",               { "sheet_hat_hood_001", {} } },
        { "hat_helmet_001",             { "sheet_hat_helmet_001", { "hair_hide_all" } } },
        { "hat_wizard_001",             { "sheet_hat_wizard_001", {} } },
        { "hat_cap_001",                { "sheet_hat_cap_001", {} } }
    });

    drawSlot("Face Details (Warpaint / Eyepatch)", entities::CosmeticCategory::Face, {
        { "var_face_warpaint_001", { "sheet_var_face_warpaint_001", {} } },
        { "var_face_eyepatch_001", { "sheet_var_face_eyepatch_001", {} } }
    });

    drawSlot("Goggles / Eyewear", entities::CosmeticCategory::Goggles, {
        { "fac_adventurer_goggles_001", { "sheet_fac_adventurer_goggles_001", {} } },
        { "fac_mechanical_goggles_001", { "sheet_fac_mechanical_goggles_001", {} } },
        { "goggles_adventure_001",      { "sheet_goggles_adventure_001", {} } },
        { "goggles_steampunk_001",      { "sheet_goggles_steampunk_001", {} } },
        { "goggles_snow_001",           { "sheet_goggles_snow_001", {} } }
    });

    drawSlot("Masks", entities::CosmeticCategory::Mask, {
        { "fac_cloth_mask_001",  { "sheet_fac_cloth_mask_001", {} } },
        { "fac_leather_mask_001", { "sheet_fac_leather_mask_001", {} } },
        { "fac_mystic_mask_001", { "sheet_fac_mystic_mask_001", { "mask_hide_goggles" } } },
        { "mask_cloth_001",      { "sheet_mask_cloth_001", {} } },
        { "mask_leather_001",    { "sheet_mask_leather_001", {} } },
        { "mask_mystic_001",     { "sheet_mask_mystic_001", { "mask_hide_goggles" } } }
    });

    drawSlot("Backpacks (5 P17)", entities::CosmeticCategory::Back, {
        { "bag_small_traveler_001",   { "sheet_bag_small_traveler_001", {} } },
        { "bag_leather_backpack_001",  { "sheet_bag_leather_backpack_001", {} } },
        { "bag_adventurer_backpack_001", { "sheet_bag_adventurer_backpack_001", {} } },
        { "bag_explorer_backpack_001", { "sheet_bag_explorer_backpack_001", {} } },
        { "bag_military_backpack_001", { "sheet_bag_military_backpack_001", {} } },
        { "backpack_traveler_001",     { "sheet_backpack_traveler_001", {} } },
        { "backpack_leather_001",      { "sheet_backpack_leather_001", {} } },
        { "backpack_adventure_001",    { "sheet_backpack_adventure_001", {} } }
    });

    ImGui::Separator();

    // 4. Occlusion & Compatibility Status
    ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.4f, 1.0f), "Compatibility & Occlusion Status:");
    if (equip.HasOcclusionFlag("hair_hide_all")) {
        ImGui::BulletText("hair_hide_all: ACTIVE (hair layer suppressed by helmet)");
    } else {
        ImGui::BulletText("hair_hide_all: inactive");
    }
    if (equip.HasOcclusionFlag("mask_hide_goggles")) {
        ImGui::BulletText("mask_hide_goggles: ACTIVE (goggles suppressed by full-face mask)");
    } else {
        ImGui::BulletText("mask_hide_goggles: inactive");
    }

    ImGui::Separator();

    // 5. Active Render Stack Order
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Runtime Render Layer Stack (Directional Z-Sort):");
    auto stack = modularChar.BuildLayerStack(player->GetFacing());

    if (ImGui::BeginTable("LayerStackTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Order", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Layer", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Z-Index", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Spritesheet", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        int order = 0;
        for (const auto& layer : stack) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", ++order);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", layer.name.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", layer.zIndex);

            ImGui::TableSetColumnIndex(3);
            if (layer.visible) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "%s", layer.spritesheetId.c_str());
            } else {
                ImGui::TextDisabled("%s (hidden)", layer.spritesheetId.c_str());
            }
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    // 6. Persistence Actions
    if (ImGui::Button("Save to player_appearance.json")) {
        modularChar.SaveAppearance("data/characters/player_appearance.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload from player_appearance.json")) {
        if (assetMgr) {
            modularChar.LoadAppearance("data/characters/player_appearance.json", *assetMgr);
        }
    }

    // 7. Phase 25 Customization Manager & Loadouts Subsystem
    if (customMgr) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.0f), "Phase 25 Character Customization & Loadout System:");

        auto& preview = customMgr->GetPreview();
        bool isPreviewActive = preview.IsActive();
        ImGui::Text("Live Preview State: %s | Unsaved Changes: %s",
                    isPreviewActive ? "ACTIVE" : "INACTIVE",
                    preview.HasUnsavedChanges() ? "YES" : "NO");

        if (!isPreviewActive) {
            if (ImGui::Button("Begin Preview Session")) {
                customMgr->BeginPreview();
            }
        } else {
            if (ImGui::Button("Rotate Left")) {
                preview.RotateCounterClockwise();
            }
            ImGui::SameLine();
            if (ImGui::Button("Rotate Right")) {
                preview.RotateClockwise();
            }
            ImGui::SameLine();
            if (ImGui::Button(preview.IsWalkingAnimation() ? "Anim: WALK" : "Anim: IDLE")) {
                preview.ToggleAnimationMode();
            }
            ImGui::SameLine();
            if (ImGui::Button("Confirm Changes")) {
                customMgr->ConfirmPreview();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel Preview")) {
                customMgr->CancelPreview();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Preview")) {
                customMgr->ResetPreview();
            }
        }

        // Loadouts table
        ImGui::Separator();
        const auto& loadouts = customMgr->GetLoadouts();
        ImGui::Text("Saved Loadouts (%zu / %zu):", loadouts.size(), customMgr->GetMaxLoadouts());

        for (size_t i = 0; i < loadouts.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            ImGui::Text("%zu. %s", i + 1, loadouts[i].name.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                customMgr->LoadLoadout(i);
            }
            ImGui::SameLine();
            if (ImGui::Button("Save Here")) {
                customMgr->SaveCurrentToLoadout(i);
            }
            ImGui::SameLine();
            if (ImGui::Button("Duplicate")) {
                customMgr->DuplicateLoadout(i);
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                customMgr->DeleteLoadout(i);
            }
            ImGui::PopID();
        }

        if (loadouts.size() < customMgr->GetMaxLoadouts()) {
            if (ImGui::Button("Save Current as New Loadout")) {
                customMgr->CreateLoadout("Loadout " + std::to_string(loadouts.size() + 1));
            }
        }
    }
}

void DevUI::RenderCosmeticsStore(cosmetics::CosmeticsStoreManager* storeMgr, customization::CharacterCustomizationManager* customMgr) {
    if (!storeMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "CosmeticsStoreManager is not available.");
        return;
    }

    auto& ownershipMgr = storeMgr->GetOwnershipManager();
    size_t totalCount = ownershipMgr.GetTotalRecordsCount();
    size_t ownedCount = ownershipMgr.GetOwnedRecordsCount();
    size_t lockedCount = ownershipMgr.GetLockedRecordsCount();

    ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.25f, 1.0f), "Phase 26 Cosmetics Store & Wardrobe Debugger");
    ImGui::Text("Total Cosmetics: %zu | Owned: %zu | Locked: %zu", totalCount, ownedCount, lockedCount);
    ImGui::Separator();

    // Prototype Ownership Testing Controls
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 0.95f, 1.0f), "Ownership State Actions (Local Prototype):");
    if (ImGui::Button("Unlock All (Prototype)")) {
        if (customMgr) {
            for (const auto* def : customMgr->GetRegistry().GetAllCosmetics()) {
                if (def) ownershipMgr.UnlockPrototype(def->id, "dev_unlock_all");
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset to Defaults")) {
        ownershipMgr.ResetOwnership();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Ownership")) {
        ownershipMgr.Save();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload Ownership")) {
        ownershipMgr.Load();
    }

    ImGui::Separator();

    // Store / Wardrobe Tab Switcher
    int currentTab = static_cast<int>(storeMgr->GetActiveTab());
    const char* tabs[] = { "Store View", "Wardrobe View" };
    if (ImGui::Combo("Interface Mode", &currentTab, tabs, 2)) {
        storeMgr->SetActiveTab(static_cast<cosmetics::StoreTab>(currentTab));
    }

    // Category Selector
    const auto& categories = cosmetics::CosmeticsStoreManager::GetStandardCategories();
    std::string activeCat = storeMgr->GetCategory();
    if (ImGui::BeginCombo("Category", activeCat.c_str())) {
        for (const auto& cat : categories) {
            bool isSelected = (cat == activeCat);
            if (ImGui::Selectable(cat.c_str(), isSelected)) {
                storeMgr->SetCategory(cat);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Search query
    static char devSearchBuf[64] = "";
    if (ImGui::InputText("Search Filter", devSearchBuf, sizeof(devSearchBuf))) {
        storeMgr->SetSearchQuery(devSearchBuf);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        devSearchBuf[0] = '\0';
        storeMgr->SetSearchQuery("");
    }

    // Live Preview Controls
    if (customMgr && customMgr->GetPreview().IsActive()) {
        auto& preview = customMgr->GetPreview();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.4f, 1.0f), "Character Preview Controls:");
        if (ImGui::Button("Rotate Left")) preview.RotateCounterClockwise();
        ImGui::SameLine();
        if (ImGui::Button("Rotate Right")) preview.RotateClockwise();
        ImGui::SameLine();
        if (ImGui::Button(preview.IsWalkingAnimation() ? "Anim: WALK" : "Anim: IDLE")) {
            preview.ToggleAnimationMode();
        }
        ImGui::SameLine();
        if (ImGui::Button("Confirm Changes")) customMgr->ConfirmPreview();
        ImGui::SameLine();
        if (ImGui::Button("Reset Preview")) customMgr->ResetPreview();
    }

    ImGui::Separator();

    // Catalogue Table
    auto displayed = storeMgr->GetDisplayedCosmetics();
    ImGui::Text("Displayed Cosmetics: %zu", displayed.size());

    if (ImGui::BeginTable("CosmeticsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 260))) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Rarity", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 65.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < displayed.size(); ++i) {
            const auto* item = displayed[i];
            if (!item) continue;

            ImGui::PushID(static_cast<int>(i));
            ImGui::TableNextRow();

            // ID
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item->id.c_str());

            // Name
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item->displayName.c_str());

            // Slot
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(std::string(customization::CustomizationSlotToString(item->slot)).c_str());

            // Rarity
            ImGui::TableNextColumn();
            cosmetics::CosmeticRarity r = cosmetics::StringToRarity(item->rarity);
            Color rc = cosmetics::RarityToColor(r);
            ImGui::TextColored(ImVec4(rc.r / 255.0f, rc.g / 255.0f, rc.b / 255.0f, 1.0f), "%s", std::string(cosmetics::RarityToDisplayName(r)).c_str());

            // Status
            ImGui::TableNextColumn();
            bool isOwned = ownershipMgr.IsOwned(item->id);
            if (isOwned) {
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "OWNED");
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.2f, 1.0f), "LOCKED");
            }

            // Actions
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Preview")) {
                storeMgr->SelectCosmetic(item->id);
                storeMgr->PreviewSelected();
            }
            ImGui::SameLine();
            if (isOwned) {
                if (ImGui::SmallButton("Equip")) {
                    storeMgr->SelectCosmetic(item->id);
                    storeMgr->EquipSelected();
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Revoke")) {
                    ownershipMgr.RevokeCosmetic(item->id);
                }
            } else {
                if (ImGui::SmallButton("Unlock")) {
                    ownershipMgr.UnlockPrototype(item->id, "dev_table_unlock");
                }
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}


void DevUI::RenderCreaturePreview(resources::AssetManager* assetMgr) {
    if (!assetMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "AssetManager not available.");
        return;
    }

    ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.3f, 1.0f), "Creature & Wildlife Animation Preview (Phase 9)");
    ImGui::Text("Inspect combat-capable creatures (Category A) and ambient wildlife (Category B).");
    ImGui::Separator();

    struct CreatureDef {
        const char* displayName;
        const char* category;
        const char* spritesheetId;
        std::vector<const char*> animClips;
        Vector2 frameSize;
        Vector2 collisionSize;
    };

    static const std::vector<CreatureDef> s_creatures = {
        { "Forest Goblin", "Hostile Creature (Combat)", "goblin_spritesheet",
          { "goblin_idle_down", "goblin_idle_up", "goblin_idle_left", "goblin_idle_right",
            "goblin_walk_down", "goblin_walk_up", "goblin_walk_left", "goblin_walk_right",
            "goblin_run_down", "goblin_run_up", "goblin_run_left", "goblin_run_right",
            "goblin_attack_down", "goblin_attack_up", "goblin_attack_left", "goblin_attack_right",
            "goblin_hurt_down", "goblin_death_down" },
          { 32, 48 }, { 16, 12 } },
        { "Undead Skeleton", "Hostile Creature (Combat)", "skeleton_spritesheet",
          { "skeleton_idle_down", "skeleton_idle_up", "skeleton_idle_left", "skeleton_idle_right",
            "skeleton_walk_down", "skeleton_walk_up", "skeleton_walk_left", "skeleton_walk_right",
            "skeleton_run_down", "skeleton_run_up", "skeleton_run_left", "skeleton_run_right",
            "skeleton_attack_down", "skeleton_attack_up", "skeleton_attack_left", "skeleton_attack_right",
            "skeleton_hurt_down", "skeleton_death_down" },
          { 32, 48 }, { 16, 12 } },
        { "Forest Wolf", "Hostile Creature (Combat)", "wolf_spritesheet",
          { "wolf_idle_down", "wolf_idle_up", "wolf_idle_left", "wolf_idle_right",
            "wolf_walk_down", "wolf_walk_up", "wolf_walk_left", "wolf_walk_right",
            "wolf_run_down", "wolf_run_up", "wolf_run_left", "wolf_run_right",
            "wolf_attack_down", "wolf_attack_up", "wolf_attack_left", "wolf_attack_right",
            "wolf_hurt_down", "wolf_death_down" },
          { 48, 40 }, { 22, 14 } },
        { "Wild Boar", "Hostile Creature (Combat)", "wild_boar_spritesheet",
          { "boar_idle_down", "boar_idle_up", "boar_idle_left", "boar_idle_right",
            "boar_walk_down", "boar_walk_up", "boar_walk_left", "boar_walk_right",
            "boar_run_down", "boar_run_up", "boar_run_left", "boar_run_right",
            "boar_attack_down", "boar_attack_up", "boar_attack_left", "boar_attack_right",
            "boar_hurt_down", "boar_death_down" },
          { 48, 40 }, { 26, 16 } },
        { "Dusk Bat", "Hostile Creature (Flying)", "bat_spritesheet",
          { "bat_fly_down", "bat_fly_up", "bat_fly_left", "bat_fly_right",
            "bat_move_down", "bat_move_up", "bat_move_left", "bat_move_right",
            "bat_attack_down", "bat_attack_up", "bat_attack_left", "bat_attack_right",
            "bat_hurt_down", "bat_death_down" },
          { 32, 32 }, { 14, 14 } },
        { "Small Songbird", "Ambient Wildlife", "small_bird_spritesheet",
          { "bird_idle", "bird_fly", "bird_takeoff", "bird_landing" },
          { 24, 24 }, { 8, 8 } },
        { "Forest Butterfly", "Ambient Wildlife", "forest_butterfly_spritesheet",
          { "forest_butterfly_resting", "forest_butterfly_fly" },
          { 16, 16 }, { 6, 6 } },
        { "Blue Butterfly", "Ambient Wildlife", "blue_butterfly_spritesheet",
          { "blue_butterfly_resting", "blue_butterfly_fly" },
          { 16, 16 }, { 6, 6 } },
        { "Golden Butterfly", "Ambient Wildlife", "golden_butterfly_spritesheet",
          { "golden_butterfly_resting", "golden_butterfly_fly" },
          { 16, 16 }, { 6, 6 } }
    };

    if (m_selectedCreatureIdx >= static_cast<int>(s_creatures.size())) {
        m_selectedCreatureIdx = 0;
    }
    const auto& currentCreature = s_creatures[m_selectedCreatureIdx];

    // 1. Creature Dropdown
    if (ImGui::BeginCombo("Select Creature", currentCreature.displayName)) {
        for (int i = 0; i < static_cast<int>(s_creatures.size()); ++i) {
            bool isSelected = (m_selectedCreatureIdx == i);
            if (ImGui::Selectable(s_creatures[i].displayName, isSelected)) {
                m_selectedCreatureIdx = i;
                m_creatureAnimIdx = 0;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Classification: %s", currentCreature.category);
    ImGui::Text("Spritesheet: %s | Frame: %.0fx%.0f | Collision: %.0fx%.0f",
                currentCreature.spritesheetId, currentCreature.frameSize.x, currentCreature.frameSize.y,
                currentCreature.collisionSize.x, currentCreature.collisionSize.y);
    ImGui::Separator();

    // 2. Animation Clip Selector
    if (m_creatureAnimIdx >= static_cast<int>(currentCreature.animClips.size())) {
        m_creatureAnimIdx = 0;
    }
    const char* activeAnimName = currentCreature.animClips[m_creatureAnimIdx];

    if (ImGui::BeginCombo("Animation Clip", activeAnimName)) {
        for (int j = 0; j < static_cast<int>(currentCreature.animClips.size()); ++j) {
            bool isAnimSel = (m_creatureAnimIdx == j);
            if (ImGui::Selectable(currentCreature.animClips[j], isAnimSel)) {
                m_creatureAnimIdx = j;
            }
        }
        ImGui::EndCombo();
    }

    // Clip metadata details
    const auto* clip = assetMgr->GetAnimation(activeAnimName);
    if (clip) {
        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.5f, 1.0f), "Clip: %s", clip->name.c_str());
        ImGui::Text("Duration: %.2fs per frame | Total Frames: %zu | Looping: %s",
                    clip->frameDuration, clip->frames.size(), clip->loop ? "Yes" : "No");

        std::string frameStr = "Frame Indices: [ ";
        for (int f : clip->frames) {
            frameStr += std::to_string(f) + " ";
        }
        frameStr += "]";
        ImGui::TextDisabled("%s", frameStr.c_str());
    } else {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Clip '%s' registered in manifest.", activeAnimName);
    }

    ImGui::Separator();

    // 3. Quick Action Buttons
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Quick Clip Jump:");
    for (int k = 0; k < static_cast<int>(currentCreature.animClips.size()); ++k) {
        if (k > 0 && k % 4 != 0) ImGui::SameLine();
        if (ImGui::Button(currentCreature.animClips[k])) {
            m_creatureAnimIdx = k;
        }
    }
}

void DevUI::RenderEnvironmentPreview(resources::AssetManager* assetMgr) {
    if (!assetMgr) return;

    struct EnvAssetEntry {
        const char* id;
        const char* name;
        const char* category;
        const char* subcategory;
        const char* textureId;
        int width;
        int height;
        int anchorX;
        int anchorY;
        bool hasCollision;
        int colW;
        int colH;
        bool isBuilding;
    };

    static const std::vector<EnvAssetEntry> s_envAssets = {
        // Nature: Oak Trees (4)
        { "tree_oak_001", "Small Oak Tree", "Nature", "trees/oak", "tex_tree_oak_001", 48, 64, 24, 60, true, 16, 10, false },
        { "tree_oak_002", "Medium Oak Tree", "Nature", "trees/oak", "tex_tree_oak_002", 64, 80, 32, 74, true, 20, 12, false },
        { "tree_oak_003", "Large Oak Tree", "Nature", "trees/oak", "tex_tree_oak_003", 80, 96, 40, 90, true, 24, 14, false },
        { "tree_oak_004", "Ancient Oak Tree", "Nature", "trees/oak", "tex_tree_oak_004", 112, 128, 56, 120, true, 36, 18, false },

        // Nature: Pine Trees (3)
        { "tree_pine_001", "Small Pine Tree", "Nature", "trees/pine", "tex_tree_pine_001", 32, 64, 16, 60, true, 12, 10, false },
        { "tree_pine_002", "Medium Pine Tree", "Nature", "trees/pine", "tex_tree_pine_002", 48, 80, 24, 76, true, 16, 12, false },
        { "tree_pine_003", "Large Pine Tree", "Nature", "trees/pine", "tex_tree_pine_003", 64, 96, 32, 90, true, 20, 14, false },

        // Nature: Bushes (5)
        { "bush_small_001", "Small Bush", "Nature", "bushes", "tex_bush_small_001", 24, 24, 12, 22, false, 0, 0, false },
        { "bush_round_001", "Round Bush", "Nature", "bushes", "tex_bush_round_001", 32, 32, 16, 28, true, 16, 10, false },
        { "bush_dense_001", "Dense Bush", "Nature", "bushes", "tex_bush_dense_001", 40, 32, 20, 28, true, 24, 12, false },
        { "bush_flowering_001", "Flowering Bush", "Nature", "bushes", "tex_bush_flowering_001", 32, 32, 16, 28, true, 16, 10, false },
        { "bush_large_001", "Large Bush", "Nature", "bushes", "tex_bush_large_001", 48, 36, 24, 32, true, 30, 14, false },

        // Nature: Grass (6)
        { "grass_small_001", "Small Grass Patch", "Nature", "grass", "tex_grass_small_001", 16, 16, 8, 14, false, 0, 0, false },
        { "grass_dense_001", "Dense Grass Patch", "Nature", "grass", "tex_grass_dense_001", 24, 20, 12, 18, false, 0, 0, false },
        { "grass_tall_001", "Tall Grass Tuft", "Nature", "grass", "tex_grass_tall_001", 24, 32, 12, 30, false, 0, 0, false },
        { "grass_dry_001", "Dry Grass Patch", "Nature", "grass", "tex_grass_dry_001", 20, 20, 10, 18, false, 0, 0, false },
        { "grass_flowering_001", "Flowering Grass", "Nature", "grass", "tex_grass_flowering_001", 24, 24, 12, 22, false, 0, 0, false },
        { "grass_forest_001", "Lush Forest Grass", "Nature", "grass", "tex_grass_forest_001", 32, 24, 16, 22, false, 0, 0, false },

        // Nature: Flowers (5)
        { "flower_white_001", "White Flowers", "Nature", "flowers", "tex_flower_white_001", 16, 16, 8, 14, false, 0, 0, false },
        { "flower_yellow_001", "Yellow Buttercups", "Nature", "flowers", "tex_flower_yellow_001", 16, 16, 8, 14, false, 0, 0, false },
        { "flower_red_001", "Red Poppies", "Nature", "flowers", "tex_flower_red_001", 16, 16, 8, 14, false, 0, 0, false },
        { "flower_blue_001", "Bluebells", "Nature", "flowers", "tex_flower_blue_001", 16, 16, 8, 14, false, 0, 0, false },
        { "flower_wildflower_001", "Wildflower Cluster", "Nature", "flowers", "tex_flower_wildflower_001", 24, 20, 12, 18, false, 0, 0, false },

        // Nature: Rocks (5)
        { "rock_small_001", "Small Stone", "Nature", "rocks", "tex_rock_small_001", 16, 16, 8, 14, false, 0, 0, false },
        { "rock_medium_001", "Medium Rock", "Nature", "rocks", "tex_rock_medium_001", 24, 20, 12, 18, true, 16, 9, false },
        { "rock_boulder_001", "Large Boulder", "Nature", "rocks", "tex_rock_boulder_001", 48, 40, 24, 36, true, 36, 18, false },
        { "rock_flat_001", "Flat Stepping Stone", "Nature", "rocks", "tex_rock_flat_001", 32, 20, 16, 18, false, 0, 0, false },
        { "rock_mossy_001", "Mossy Rock", "Nature", "rocks", "tex_rock_mossy_001", 32, 28, 16, 26, true, 22, 12, false },

        // Nature: Mushrooms (4)
        { "mushroom_small_001", "Small Mushroom", "Nature", "mushrooms", "tex_mushroom_small_001", 16, 16, 8, 14, false, 0, 0, false },
        { "mushroom_red_001", "Red Toadstool", "Nature", "mushrooms", "tex_mushroom_red_001", 16, 20, 8, 18, false, 0, 0, false },
        { "mushroom_brown_001", "Brown Mushroom", "Nature", "mushrooms", "tex_mushroom_brown_001", 20, 20, 10, 18, false, 0, 0, false },
        { "mushroom_cluster_001", "Mushroom Cluster", "Nature", "mushrooms", "tex_mushroom_cluster_001", 24, 20, 12, 18, false, 0, 0, false },

        // Nature: Logs (3)
        { "log_branch_001", "Small Fallen Branch", "Nature", "logs", "tex_log_branch_001", 24, 16, 12, 14, false, 0, 0, false },
        { "log_medium_001", "Medium Fallen Log", "Nature", "logs", "tex_log_medium_001", 48, 24, 24, 20, true, 40, 10, false },
        { "log_trunk_001", "Large Fallen Tree Trunk", "Nature", "logs", "tex_log_trunk_001", 64, 32, 32, 28, true, 54, 14, false },

        // Buildings: Small Houses (3)
        { "house_cottage_001", "Wooden Cottage", "Buildings", "houses", "tex_house_cottage_001", 80, 80, 40, 76, true, 64, 40, true },
        { "house_stone_small_001", "Small Stone House", "Buildings", "houses", "tex_house_stone_small_001", 80, 80, 40, 76, true, 64, 40, true },
        { "house_village_001", "Forest Village House", "Buildings", "houses", "tex_house_village_001", 96, 88, 48, 84, true, 76, 44, true },

        // Buildings: Large Houses (2)
        { "house_residence_large_001", "Large Wooden Residence", "Buildings", "houses", "tex_house_residence_large_001", 112, 96, 56, 92, true, 92, 48, true },
        { "house_twostory_001", "Two-Story Medieval House", "Buildings", "houses", "tex_house_twostory_001", 120, 112, 60, 108, true, 96, 54, true },

        // Buildings: Cabins (3)
        { "cabin_forest_001", "Forest Cabin", "Buildings", "cabins", "tex_cabin_forest_001", 72, 72, 36, 68, true, 56, 36, true },
        { "cabin_hunting_001", "Hunting Cabin", "Buildings", "cabins", "tex_cabin_hunting_001", 72, 72, 36, 68, true, 56, 36, true },
        { "cabin_woodland_001", "Remote Woodland Cabin", "Buildings", "cabins", "tex_cabin_woodland_001", 80, 76, 40, 72, true, 64, 38, true },

        // Buildings: Shops (2)
        { "shop_general_001", "General Store", "Buildings", "shops", "tex_shop_general_001", 96, 88, 48, 84, true, 76, 44, true },
        { "shop_market_001", "Market Shop Stall", "Buildings", "shops", "tex_shop_market_001", 88, 80, 44, 76, true, 70, 40, true },

        // Buildings: Blacksmith (1)
        { "blacksmith_forge_001", "Village Blacksmith Forge", "Buildings", "blacksmith", "tex_blacksmith_forge_001", 112, 96, 56, 92, true, 90, 48, true },

        // Buildings: Storage (2)
        { "storage_shed_001", "Wooden Storage Shed", "Buildings", "storage", "tex_storage_shed_001", 64, 64, 32, 60, true, 50, 32, true },
        { "storage_warehouse_001", "Supply Warehouse", "Buildings", "storage", "tex_storage_warehouse_001", 96, 88, 48, 84, true, 78, 44, true },

        // Buildings: Watchtowers (2)
        { "watchtower_wood_001", "Wooden Watchtower", "Buildings", "watchtowers", "tex_watchtower_wood_001", 64, 112, 32, 108, true, 40, 36, true },
        { "watchtower_stone_001", "Stone Watchtower", "Buildings", "watchtowers", "tex_watchtower_stone_001", 64, 120, 32, 116, true, 44, 40, true },

        // Buildings: Ruins (3)
        { "ruin_wall_broken_001", "Broken Stone Wall", "Buildings", "ruins", "tex_ruin_wall_broken_001", 64, 40, 32, 36, true, 52, 16, true },
        { "ruin_building_abandoned_001", "Abandoned Stone Ruin", "Buildings", "ruins", "tex_ruin_building_abandoned_001", 96, 80, 48, 76, true, 74, 40, true },
        { "ruin_tower_small_001", "Small Ruined Tower", "Buildings", "ruins", "tex_ruin_tower_small_001", 64, 88, 32, 84, true, 44, 36, true },

        // Props: Fences (4)
        { "fence_straight_001", "Wooden Straight Fence", "Props", "fences", "tex_fence_straight_001", 32, 24, 16, 22, true, 30, 8, false },
        { "fence_corner_001", "Wooden Corner Fence", "Props", "fences", "tex_fence_corner_001", 32, 32, 16, 28, true, 24, 24, false },
        { "fence_gate_001", "Wooden Fence Gate", "Props", "fences", "tex_fence_gate_001", 32, 24, 16, 22, true, 30, 8, false },
        { "fence_broken_001", "Broken Wooden Fence", "Props", "fences", "tex_fence_broken_001", 32, 24, 16, 22, true, 18, 8, false },

        // Props: Barrels (3)
        { "barrel_standard_001", "Standard Barrel", "Props", "barrels", "tex_barrel_standard_001", 20, 24, 10, 22, true, 14, 12, false },
        { "barrel_small_001", "Small Keg", "Props", "barrels", "tex_barrel_small_001", 16, 18, 8, 16, true, 12, 10, false },
        { "barrel_banded_001", "Metal-Banded Barrel", "Props", "barrels", "tex_barrel_banded_001", 22, 26, 11, 24, true, 16, 12, false },

        // Props: Crates (3)
        { "crate_small_001", "Small Wooden Crate", "Props", "crates", "tex_crate_small_001", 20, 20, 10, 18, true, 16, 14, false },
        { "crate_large_001", "Large Cargo Crate", "Props", "crates", "tex_crate_large_001", 32, 32, 16, 28, true, 26, 20, false },
        { "crate_reinforced_001", "Reinforced Storage Crate", "Props", "crates", "tex_crate_reinforced_001", 28, 28, 14, 25, true, 22, 18, false },

        // Props: Wells (2)
        { "well_stone_001", "Stone Village Well", "Props", "wells", "tex_well_stone_001", 36, 40, 18, 36, true, 26, 22, false },
        { "well_wooden_001", "Wooden Roofed Well", "Props", "wells", "tex_well_wooden_001", 36, 44, 18, 40, true, 26, 22, false },

        // Props: Lamps (3)
        { "lamp_post_001", "Wooden Post Street Lamp", "Props", "lamps", "tex_lamp_post_001", 20, 48, 10, 46, true, 10, 10, false },
        { "lamp_wall_001", "Wall-Mounted Lantern", "Props", "lamps", "tex_lamp_wall_001", 16, 24, 8, 22, false, 0, 0, false },
        { "lamp_hanging_001", "Hanging Chain Lantern", "Props", "lamps", "tex_lamp_hanging_001", 16, 32, 8, 30, false, 0, 0, false },

        // Props: Signs (4)
        { "sign_direction_001", "Village Direction Signpost", "Props", "signs", "tex_sign_direction_001", 24, 36, 12, 34, true, 8, 8, false },
        { "sign_shop_001", "Hanging Shop Signboard", "Props", "signs", "tex_sign_shop_001", 24, 28, 12, 26, false, 0, 0, false },
        { "sign_noticeboard_001", "Wooden Notice Board", "Props", "signs", "tex_sign_noticeboard_001", 36, 36, 18, 34, true, 28, 10, false },
        { "sign_warning_001", "Warning Signpost", "Props", "signs", "tex_sign_warning_001", 20, 32, 10, 30, true, 8, 8, false },

        // Props: Bridges (3)
        { "bridge_wood_small_001", "Small Wooden Footbridge", "Props", "bridges", "tex_bridge_wood_small_001", 64, 32, 32, 28, false, 0, 0, false },
        { "bridge_wood_large_001", "Large Timber Bridge", "Props", "bridges", "tex_bridge_wood_large_001", 96, 48, 48, 44, false, 0, 0, false },
        { "bridge_stone_001", "Arched Stone Bridge", "Props", "bridges", "tex_bridge_stone_001", 96, 48, 48, 44, false, 0, 0, false },

        // Props: Benches (2)
        { "bench_wooden_001", "Village Wooden Bench", "Props", "benches", "tex_bench_wooden_001", 32, 20, 16, 18, true, 26, 10, false },
        { "bench_stone_001", "Carved Stone Bench", "Props", "benches", "tex_bench_stone_001", 32, 20, 16, 18, true, 26, 10, false }
    };

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Wildbound Environment Asset Collection (77 Assets)");
    ImGui::Separator();

    // Category Filter Buttons
    const char* catNames[] = { "All (77)", "Nature (35)", "Buildings (18)", "Props (24)" };
    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine();
        if (m_selectedEnvCategory == i) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
        }
        if (ImGui::Button(catNames[i])) {
            m_selectedEnvCategory = i;
            m_selectedEnvAssetIdx = 0;
        }
        if (m_selectedEnvCategory == i) {
            ImGui::PopStyleColor();
        }
    }

    // Filter items
    std::vector<const EnvAssetEntry*> filtered;
    for (const auto& a : s_envAssets) {
        if (m_selectedEnvCategory == 1 && std::string(a.category) != "Nature") continue;
        if (m_selectedEnvCategory == 2 && std::string(a.category) != "Buildings") continue;
        if (m_selectedEnvCategory == 3 && std::string(a.category) != "Props") continue;
        filtered.push_back(&a);
    }

    if (m_selectedEnvAssetIdx >= static_cast<int>(filtered.size())) {
        m_selectedEnvAssetIdx = 0;
    }

    ImGui::Separator();
    ImGui::Columns(2, "EnvColumns", true);
    ImGui::SetColumnWidth(0, 200.0f);

    // Left List
    ImGui::BeginChild("EnvListScroll", ImVec2(0, 360), true);
    for (int i = 0; i < static_cast<int>(filtered.size()); ++i) {
        const auto* item = filtered[i];
        bool selected = (m_selectedEnvAssetIdx == i);
        if (ImGui::Selectable(item->name, selected)) {
            m_selectedEnvAssetIdx = i;
        }
        if (selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right Details & Preview
    ImGui::BeginChild("EnvDetailScroll", ImVec2(0, 360), true);
    if (!filtered.empty()) {
        const auto* active = filtered[m_selectedEnvAssetIdx];
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "%s", active->name);
        ImGui::TextDisabled("ID: %s | Texture: %s", active->id, active->textureId);
        ImGui::Text("Category: %s / %s | Biome: Forest", active->category, active->subcategory);
        ImGui::Text("Dimensions: %d x %d px | Ground Anchor: (%d, %d)",
                    active->width, active->height, active->anchorX, active->anchorY);

        if (active->hasCollision) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                               "Collision: Enabled (Rectangle %d x %d px at base)", active->colW, active->colH);
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Collision: None (Decorative / Traversable)");
        }

        if (active->isBuilding) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Building Architecture Metadata:");
            ImGui::Text("Entrance / Door: (%d, %d) | Ground Footprint: %d x %d px",
                        active->anchorX, active->anchorY, active->colW, active->colH);
            ImGui::Text("Interior Compatibility: Supported (interior_%s)", active->id);
            ImGui::Text("Rendering Layer: solid_structures | Y-Sorting: True");
        } else {
            ImGui::Text("Rendering Layer: %s | Y-Sorting: %s",
                        active->hasCollision ? "environment" : "ground_decor",
                        active->hasCollision ? "True" : "False");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.7f, 1.0f), "Texture Preview:");

        Texture2D tex = assetMgr->GetTexture(active->textureId);
        if (tex.id != 0) {
            float previewScale = 2.0f;
            if (tex.width > 96 || tex.height > 96) previewScale = 1.5f;
            int pw = static_cast<int>(tex.width * previewScale);
            int ph = static_cast<int>(tex.height * previewScale);
            rlImGuiImageSize(&tex, pw, ph);
        } else {
            ImGui::TextDisabled("Texture '%s' registered in manifest (Loads on demand).", active->textureId);
        }
    }
    ImGui::EndChild();
    ImGui::Columns(1);
}

void DevUI::RenderLivingWorld(world::environment::EnvironmentalAnimationSystem* envAnimSys, resources::AssetManager* /*assetMgr*/, Vector2 playerPos) {
    if (!envAnimSys) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Environmental Animation System is not active.");
        return;
    }

    ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Living World & Environmental Atmosphere (Phase 11)");
    ImGui::TextDisabled("Real-time procedural & cyclical animation states across 4 environmental categories.");
    ImGui::Separator();

    // 1. System Overview & Time Controls
    if (ImGui::CollapsingHeader("World Simulation Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        float speed = envAnimSys->GetGlobalPlaybackSpeed();
        if (ImGui::SliderFloat("Global Animation Speed", &speed, 0.1f, 3.0f, "%.2fx")) {
            envAnimSys->SetGlobalPlaybackSpeed(speed);
        }
        ImGui::Text("Active Animated Objects: %zu", envAnimSys->GetAnimatedObjectCount());
        ImGui::Text("Active Ambient Creatures: %zu", envAnimSys->GetAmbientCreatureCount());
        ImGui::Text("Drifting Overhead Clouds: %zu", envAnimSys->GetCloudCount());
        const auto& flyover = envAnimSys->GetAirplaneEvent();
        ImGui::Text("Flyover Event Status:    %s", flyover.active ? "ACTIVE (Flying overhead)" : "IDLE (Standby)");
    }

    // 2. Category A: Environmental Object Animations
    if (ImGui::CollapsingHeader("Category A — Environmental Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool water = envAnimSys->IsWaterEnabled();
        if (ImGui::Checkbox("Water Surfaces & Flow", &water)) {
            envAnimSys->SetWaterEnabled(water);
        }
        ImGui::SameLine(220);
        bool fire = envAnimSys->IsFireEnabled();
        if (ImGui::Checkbox("Campfire & Torch Flames", &fire)) {
            envAnimSys->SetFireEnabled(fire);
        }

        bool wind = envAnimSys->IsWindEnabled();
        if (ImGui::Checkbox("Wind Foliage Sway (Trees & Grass)", &wind)) {
            envAnimSys->SetWindEnabled(wind);
        }

        ImGui::TextDisabled("Water: pond ripple, flowing river, lake surface, rapids, droplets.");
        ImGui::TextDisabled("Fire: small torch, medium campfire, forge bonfire.");
        ImGui::TextDisabled("Vegetation: oak canopy breeze, pine branch flex, bush rustle, wind grass.");
    }

    // 3. Category B: Ambient Wildlife
    if (ImGui::CollapsingHeader("Category B — Ambient Wildlife", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool ambient = envAnimSys->IsAmbientEnabled();
        if (ImGui::Checkbox("Ambient Wildlife Enabled", &ambient)) {
            envAnimSys->SetAmbientEnabled(ambient);
        }

        const char* speciesOptions[] = {
            "Small Bird (Flight, Perch, Flap)",
            "Forest Butterfly (Green Flutter)",
            "Blue Butterfly (Azure Flutter)",
            "Golden Butterfly (Amber Flutter)"
        };
        ImGui::Combo("Spawn Species", &m_selectedAmbientSpeciesIdx, speciesOptions, 4);

        if (ImGui::Button("Spawn at Target Position")) {
            Rectangle bounds{ playerPos.x - 400.0f, playerPos.y - 300.0f, 800.0f, 600.0f };
            if (m_selectedAmbientSpeciesIdx == 0) {
                envAnimSys->SpawnAmbientBird(playerPos, bounds);
            } else if (m_selectedAmbientSpeciesIdx == 1) {
                envAnimSys->SpawnAmbientButterfly(playerPos, "forest_butterfly", bounds);
            } else if (m_selectedAmbientSpeciesIdx == 2) {
                envAnimSys->SpawnAmbientButterfly(playerPos, "blue_butterfly", bounds);
            } else {
                envAnimSys->SpawnAmbientButterfly(playerPos, "golden_butterfly", bounds);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Ambient Wildlife")) {
            envAnimSys->ClearAmbientCreatures();
        }
    }

    // 4. Category C: Atmospheric Effects
    if (ImGui::CollapsingHeader("Category C — Atmospheric Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool clouds = envAnimSys->IsCloudsEnabled();
        if (ImGui::Checkbox("Drifting Clouds (Overhead Layer)", &clouds)) {
            envAnimSys->SetCloudsEnabled(clouds);
        }

        bool fog = envAnimSys->IsFogEnabled();
        if (ImGui::Checkbox("Atmospheric Ground Fog Layer", &fog)) {
            envAnimSys->SetFogEnabled(fog);
        }

        if (fog) {
            float fogOp = envAnimSys->GetFogOpacity();
            if (ImGui::SliderFloat("Fog Opacity / Density", &fogOp, 0.0f, 1.0f, "%.2f")) {
                envAnimSys->SetFogOpacity(fogOp);
            }
        }

        ImGui::TextDisabled("Smoke Emitters: Active chimney, camp, and ruin smoke plumes.");
    }

    // 5. Category D: Scripted Flyover Events
    if (ImGui::CollapsingHeader("Category D — Scripted Environmental Events", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto& event = envAnimSys->GetAirplaneEvent();
        if (event.active) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Airplane Flight in Progress!");
            ImGui::Text("Current Position: (%.1f, %.1f) | Altitude: %.1f px",
                        event.currentPos.x, event.currentPos.y, event.altitude);
            ImGui::Text("Heading Angle:    %.1f deg | Speed: %.1f px/s",
                        event.rotation, event.speed);
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Recon Airplane: Ready for dispatch");
        }

        if (ImGui::Button("Trigger Recon Airplane Flyover")) {
            Vector2 start{ playerPos.x - 800.0f, playerPos.y - 250.0f };
            Vector2 end{ playerPos.x + 800.0f, playerPos.y + 250.0f };
            envAnimSys->TriggerAirplaneFlyover(start, end, 150.0f);
        }
    }
}

void DevUI::RenderWeaponPreview(weapons::WeaponManager* weaponMgr, resources::AssetManager* assetMgr, entities::Player* player) {
    if (!weaponMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "WeaponManager is not active.");
        return;
    }

    const auto& weapons = weaponMgr->GetAllWeapons();
    if (weapons.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "No weapons registered in catalog.");
        return;
    }

    ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Wildbound Weapon Collection (Phase 15)");
    ImGui::TextDisabled("Inspect all 11 weapon designs + projectiles, icons, ground items, held sprites, and animation frames.");
    ImGui::Separator();

    // Weapon selector combo
    const weapons::WeaponConfig* activeWeapon = weaponMgr->GetWeapon(m_selectedWeaponId);
    if (!activeWeapon) {
        activeWeapon = &weapons[0];
        m_selectedWeaponId = activeWeapon->id;
    }

    if (ImGui::BeginCombo("Select Weapon", (activeWeapon->displayName + " (" + activeWeapon->id + ")").c_str())) {
        for (const auto& w : weapons) {
            bool isSelected = (m_selectedWeaponId == w.id);
            std::string label = w.displayName + " [" + std::string(weapons::WeaponCategoryToString(w.category)) + "]";
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                m_selectedWeaponId = w.id;
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Refresh active
    activeWeapon = weaponMgr->GetWeapon(m_selectedWeaponId);
    if (!activeWeapon) return;

    // Overview & Validation status
    std::vector<std::string> valErrors;
    bool isValid = assetMgr ? weaponMgr->ValidateAssets(*assetMgr, valErrors) : true;
    if (isValid) {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[VALIDATED] All production assets registered and resolved in AMS.");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[INVALID] %zu asset resolution errors detected.", valErrors.size());
    }

    ImGui::TextDisabled("Category: %s | Subcategory: %s | Handedness: %s",
                        weapons::WeaponCategoryToString(activeWeapon->category).data(),
                        activeWeapon->subcategory.c_str(),
                        activeWeapon->attachment.hand.c_str());
    ImGui::TextWrapped("%s", activeWeapon->description.c_str());
    ImGui::Separator();

    // Multi-column preview: [Icon (64x64)] | [Ground (32x32)] | [Held Directional] | [Animation Pose]
    ImGui::Columns(4, "WeaponPreviewColumns", true);
    ImGui::SetColumnWidth(0, 130.0f);
    ImGui::SetColumnWidth(1, 130.0f);
    ImGui::SetColumnWidth(2, 140.0f);

    // Column 1: Inventory Icon (64x64)
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Inventory Icon");
    if (assetMgr) {
        Texture2D iconTex = assetMgr->GetTexture(activeWeapon->visuals.iconTextureId);
        if (iconTex.id != 0) {
            rlImGuiImageSize(&iconTex, 80, 80);
        } else {
            ImGui::TextDisabled("Icon: %s", activeWeapon->visuals.iconTextureId.c_str());
        }
    }
    ImGui::TextDisabled("64x64 px (Scaled 1.25x)");

    ImGui::NextColumn();

    // Column 2: Ground Representation (32x32)
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Ground Item");
    if (assetMgr) {
        Texture2D groundTex = assetMgr->GetTexture(activeWeapon->visuals.groundTextureId);
        if (groundTex.id != 0) {
            rlImGuiImageSize(&groundTex, 80, 80);
        } else {
            ImGui::TextDisabled("Ground: %s", activeWeapon->visuals.groundTextureId.c_str());
        }
    }
    ImGui::TextDisabled("32x32 px (Scaled 2.5x)");

    ImGui::NextColumn();

    // Column 3: Held Directional Representation
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Held (4-Way)");
    const char* dirNames[] = { "Down (S)", "Up (N)", "Left (W)", "Right (E)" };
    ImGui::SetNextItemWidth(120.0f);
    ImGui::Combo("##Facing", &m_weaponFacing, dirNames, 4);

    if (assetMgr) {
        Texture2D heldTex = assetMgr->GetTexture(activeWeapon->visuals.heldTextureId);
        if (heldTex.id != 0) {
            Rectangle srcRect{ static_cast<float>(m_weaponFacing * 32), 0.0f, 32.0f, 32.0f };
            rlImGuiImageRect(&heldTex, 80, 80, srcRect);
        }
    }

    auto facingDir = static_cast<graphics::animation::Direction>(m_weaponFacing);
    int currentZ = activeWeapon->attachment.GetRenderOrder(facingDir);
    ImGui::TextDisabled("Z-Order: %d (%s)", currentZ, (currentZ >= 80 ? "Foreground" : "Background"));
    ImGui::TextDisabled("Grip: (%.0f, %.0f)", activeWeapon->attachment.gripAnchor.x, activeWeapon->attachment.gripAnchor.y);

    ImGui::NextColumn();

    // Column 4: Animation Pose
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Action Poses");
    int frameCount = static_cast<int>(activeWeapon->animationFrames.size());
    if (frameCount == 0) frameCount = 4;
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderInt("##PoseFrame", &m_weaponAnimFrame, 0, frameCount - 1);

    std::string frameLabel = (m_weaponAnimFrame < static_cast<int>(activeWeapon->animationFrames.size())) ?
                             activeWeapon->animationFrames[m_weaponAnimFrame] : "Frame";
    ImGui::TextDisabled("Pose: [%s]", frameLabel.c_str());

    if (assetMgr) {
        Texture2D animTex = assetMgr->GetTexture(activeWeapon->visuals.animTextureId);
        if (animTex.id != 0) {
            Rectangle srcRect{ static_cast<float>(m_weaponAnimFrame * 32), 0.0f, 32.0f, 32.0f };
            rlImGuiImageRect(&animTex, 80, 80, srcRect);
        }
    }

    ImGui::Columns(1);
    ImGui::Separator();

    // Attachment & Metadata Table
    if (ImGui::CollapsingHeader("Attachment & Directional Metadata", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Grip Anchor: (X: %.1f, Y: %.1f) | Primary Hand: %s",
                    activeWeapon->attachment.gripAnchor.x, activeWeapon->attachment.gripAnchor.y,
                    activeWeapon->attachment.hand.c_str());
        ImGui::Text("Render Order Matrix:");
        ImGui::BulletText("Facing Down (South):  Z-Index %d (Held in front of torso)", activeWeapon->attachment.renderOrderDown);
        ImGui::BulletText("Facing Up (North):    Z-Index %d (Behind body/shoulders)", activeWeapon->attachment.renderOrderUp);
        ImGui::BulletText("Facing Left (West):   Z-Index %d (In front of side torso)", activeWeapon->attachment.renderOrderLeft);
        ImGui::BulletText("Facing Right (East):  Z-Index %d (In front of side torso)", activeWeapon->attachment.renderOrderRight);
    }

    // Phase 24: Data-Driven Combat Statistics & Weapon Rarity Matrix
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Data-Driven Weapon Statistics & Rarity Scaling (Phase 24)");

    const auto& s = activeWeapon->stats;
    ImGui::Columns(4, "WeaponStatsCols", true);
    ImGui::Text("Base Damage: %.1f", s.baseDamage);
    ImGui::Text("Attack Cooldown: %.2fs", s.attackCooldown);
    ImGui::NextColumn();
    ImGui::Text("Reach / Range: %.1f px", s.range);
    ImGui::Text("Stamina Cost: %.1f", s.staminaCost);
    ImGui::NextColumn();
    ImGui::Text("Crit Chance: %.0f%%", s.critChance * 100.0f);
    ImGui::Text("Crit Multiplier: %.2fx", s.critMultiplier);
    ImGui::NextColumn();
    if (activeWeapon->category == weapons::WeaponCategory::Bow) {
        ImGui::Text("Draw Time: %.2fs", s.drawDuration);
        ImGui::Text("Arrow Speed: %.0f px/s", s.projectileSpeed);
    } else {
        ImGui::Text("Melee Arc: %.0f deg", s.meleeArc);
        ImGui::Text("Hitbox: %.0fx%.0f", s.hitboxWidth, s.hitboxHeight);
    }
    ImGui::Columns(1);

    // Rarity Multiplier Scaling Table
    ImGui::Spacing();
    ImGui::TextDisabled("Effective Damage Across Rarity Tiers:");
    weapons::WeaponRarity rarities[] = {
        weapons::WeaponRarity::Common,
        weapons::WeaponRarity::Uncommon,
        weapons::WeaponRarity::Rare,
        weapons::WeaponRarity::Epic,
        weapons::WeaponRarity::Legendary,
        weapons::WeaponRarity::Mythic
    };

    ImGui::Columns(6, "RarityCols", true);
    for (int r = 0; r < 6; ++r) {
        float mult = weapons::GetRarityDamageMultiplier(rarities[r]);
        Color c = weapons::GetRarityColor(rarities[r]);
        ImVec4 col(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f);

        ImGui::TextColored(col, "%s (%.2fx)", weapons::WeaponRarityToString(rarities[r]).data(), mult);
        ImGui::Text("%.1f DMG", s.baseDamage * mult);
        ImGui::NextColumn();
    }
    ImGui::Columns(1);

    // Live Player Equipment & Combat Status
    if (player) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "Player Live Equipment & Combat State");

        auto& eq = player->GetEquipment();
        entities::EquipmentSlot activeSlot = eq.GetActiveWeaponSlot();
        const auto* wpnMain = eq.GetWeaponConfig(entities::EquipmentSlot::MainHand);
        const auto* wpnOff  = eq.GetWeaponConfig(entities::EquipmentSlot::OffHand);
        const auto* instMain = eq.GetEquippedWeaponInstance(entities::EquipmentSlot::MainHand);
        const auto* instOff  = eq.GetEquippedWeaponInstance(entities::EquipmentSlot::OffHand);

        ImGui::Columns(2, "PlayerEquipCols", true);

        // Main Hand
        bool isMainActive = (activeSlot == entities::EquipmentSlot::MainHand);
        if (isMainActive) ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[ACTIVE] Main Hand (Slot 1):");
        else ImGui::Text("Main Hand (Slot 1):");
        if (wpnMain && instMain) {
            Color c = weapons::GetRarityColor(instMain->GetRarity());
            ImGui::TextColored(ImVec4(c.r/255.0f, c.g/255.0f, c.b/255.0f, 1.0f), "%s [%s]",
                               wpnMain->displayName.c_str(), weapons::WeaponRarityToString(instMain->GetRarity()).data());
            ImGui::Text("Effective DMG: %.1f | Cooldown: %.2fs", instMain->GetEffectiveDamage(), instMain->GetEffectiveAttackCooldown());
        } else {
            ImGui::TextDisabled("Empty");
        }
        if (ImGui::Button("Equip Current to MainHand")) {
            items::ItemInstance itemInst{ 0, activeWeapon->id, 1 };
            eq.Equip(entities::EquipmentSlot::MainHand, itemInst, nullptr, &player->GetModularAppearance(), weaponMgr);
        }

        ImGui::NextColumn();

        // Off Hand
        bool isOffActive = (activeSlot == entities::EquipmentSlot::OffHand);
        if (isOffActive) ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[ACTIVE] Off Hand (Slot 2):");
        else ImGui::Text("Off Hand (Slot 2):");
        if (wpnOff && instOff) {
            Color c = weapons::GetRarityColor(instOff->GetRarity());
            ImGui::TextColored(ImVec4(c.r/255.0f, c.g/255.0f, c.b/255.0f, 1.0f), "%s [%s]",
                               wpnOff->displayName.c_str(), weapons::WeaponRarityToString(instOff->GetRarity()).data());
            ImGui::Text("Effective DMG: %.1f | Cooldown: %.2fs", instOff->GetEffectiveDamage(), instOff->GetEffectiveAttackCooldown());
        } else {
            ImGui::TextDisabled("Empty");
        }
        if (ImGui::Button("Equip Current to OffHand")) {
            items::ItemInstance itemInst{ 0, activeWeapon->id, 1 };
            eq.Equip(entities::EquipmentSlot::OffHand, itemInst, nullptr, &player->GetModularAppearance(), weaponMgr);
        }

        ImGui::Columns(1);

        // Weapon Toggle & Attack State
        ImGui::Spacing();
        if (ImGui::Button("Switch Weapon Slot (Q)")) {
            player->ToggleWeapon(weaponMgr);
        }
        ImGui::SameLine();
        const auto& wpnCtrl = player->GetWeaponController();
        ImGui::Text("Attack State: %s | Cooldown Remaining: %.2fs",
                    weapons::WeaponAttackStateToString(wpnCtrl.GetAttackState()).data(),
                    wpnCtrl.GetCooldownRemaining());

        // Equipment Stat Modifiers
        const auto& mods = eq.GetStatModifiers();
        ImGui::TextDisabled("Equipment Modifiers: Armor: %.0f | Mitigation: %.1f%% | Speed: %.0f%% | HP: +%.0f",
                            mods.totalDefense, mods.damageReduction * 100.0f, mods.speedModifier * 100.0f, mods.healthBonus);
    }
}

void DevUI::RenderItemPreview(items::ItemManager* itemMgr, resources::AssetManager* assetMgr) {
    if (!itemMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "ItemManager is not active.");
        return;
    }

    const auto& allItems = itemMgr->GetAllItems();
    if (allItems.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "No items registered in catalog.");
        return;
    }

    ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Wildbound Items & Collectibles Collection (Phase 16)");
    ImGui::TextDisabled("Inspect all 40 production items across 5 categories: Collectibles, Consumables, Materials, Resources, and Special.");
    ImGui::Separator();

    // Category filter combo
    const char* catOptions[] = {
        "All Items (40)",
        "Collectibles (8)",
        "Consumables (10)",
        "Crafting Materials (8)",
        "Natural Resources (7)",
        "Special Items (7)"
    };

    ImGui::SetNextItemWidth(200.0f);
    ImGui::Combo("Category Filter", &m_selectedItemCategoryFilter, catOptions, 6);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(180.0f);
    ImGui::InputTextWithHint("##ItemSearch", "Search item...", m_itemSearchBuffer, sizeof(m_itemSearchBuffer));

    std::string searchLower = m_itemSearchBuffer;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    ImGui::Separator();

    // 2-Column layout: Item list on left, item details & preview on right
    ImGui::Columns(2, "ItemPreviewColumns", true);
    ImGui::SetColumnWidth(0, 220.0f);

    // Left Column: Filtered item list
    ImGui::BeginChild("ItemListScroll", ImVec2(0, 320), true);

    for (const auto& item : allItems) {
        // Filter by category
        if (m_selectedItemCategoryFilter == 1 && item.category != items::ItemCategory::Collectible) continue;
        if (m_selectedItemCategoryFilter == 2 && item.category != items::ItemCategory::Consumable) continue;
        if (m_selectedItemCategoryFilter == 3 && item.category != items::ItemCategory::CraftingMaterial) continue;
        if (m_selectedItemCategoryFilter == 4 && item.category != items::ItemCategory::NaturalResource) continue;
        if (m_selectedItemCategoryFilter == 5 && item.category != items::ItemCategory::SpecialItem) continue;

        // Filter by search string
        if (!searchLower.empty()) {
            std::string nameLower = item.displayName;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (nameLower.find(searchLower) == std::string::npos && item.id.find(searchLower) == std::string::npos) {
                continue;
            }
        }

        bool isSelected = (m_selectedItemId == item.id);
        std::string label = item.displayName + " [" + item.subcategory + "]";
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            m_selectedItemId = item.id;
        }
        if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right Column: Details & Dual Visual Previews (Icon 64x64, World 32x32)
    const items::ItemConfig* activeItem = itemMgr->GetItem(m_selectedItemId);
    if (!activeItem && !allItems.empty()) {
        activeItem = &allItems[0];
        m_selectedItemId = activeItem->id;
    }

    if (activeItem) {
        // Header
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "%s", activeItem->displayName.c_str());
        ImGui::TextDisabled("ID: %s | Subcategory: %s", activeItem->id.c_str(), activeItem->subcategory.c_str());

        // Rarity color badge
        ImVec4 rarityColor(0.8f, 0.8f, 0.8f, 1.0f);
        if (activeItem->rarityTier == "uncommon") rarityColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        else if (activeItem->rarityTier == "rare") rarityColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
        else if (activeItem->rarityTier == "epic") rarityColor = ImVec4(0.8f, 0.4f, 1.0f, 1.0f);
        else if (activeItem->rarityTier == "legendary") rarityColor = ImVec4(1.0f, 0.6f, 0.1f, 1.0f);

        ImGui::TextColored(rarityColor, "Rarity: %s", activeItem->rarityTier.c_str());
        ImGui::TextWrapped("%s", activeItem->description.c_str());

        ImGui::Separator();

        // Biome Tags
        ImGui::Text("Biome Compatibility:");
        ImGui::SameLine();
        for (const auto& tag : activeItem->biomeTags) {
            ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.4f, 1.0f), "[%s]", tag.c_str());
            ImGui::SameLine();
        }
        ImGui::NewLine();

        ImGui::Separator();

        // Visual Previews: Icon and World
        ImGui::Columns(2, "ItemVisualSubCols", false);
        ImGui::SetColumnWidth(0, 140.0f);

        // Subcol 1: Inventory Icon (64x64, scaled to 96x96)
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Inventory Icon");
        if (assetMgr) {
            Texture2D iconTex = assetMgr->GetTexture(activeItem->visuals.iconTextureId);
            if (iconTex.id != 0) {
                rlImGuiImageSize(&iconTex, 96, 96);
            } else {
                ImGui::TextDisabled("Icon: %s", activeItem->visuals.iconTextureId.c_str());
            }
        }
        ImGui::TextDisabled("64x64 px (Scaled 1.5x)");

        ImGui::NextColumn();

        // Subcol 2: World Representation (32x32, scaled to 96x96 with drop shadow)
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "World Item");
        if (assetMgr) {
            Texture2D worldTex = assetMgr->GetTexture(activeItem->visuals.worldTextureId);
            if (worldTex.id != 0) {
                rlImGuiImageSize(&worldTex, 96, 96);
            } else {
                ImGui::TextDisabled("World: %s", activeItem->visuals.worldTextureId.c_str());
            }
        }
        ImGui::TextDisabled("32x32 px (Scaled 3.0x)");

        ImGui::Columns(1);
        ImGui::Separator();

        // AMS Asset validation status
        if (assetMgr) {
            bool hasIcon = assetMgr->GetRegistry().HasAsset(activeItem->visuals.iconTextureId);
            bool hasWorld = assetMgr->GetRegistry().HasAsset(activeItem->visuals.worldTextureId);
            if (hasIcon && hasWorld) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[VALIDATED] Both Icon & World assets resolved in AMS.");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[ERROR] Missing texture references in AMS.");
            }
        }
    }

    ImGui::Columns(1);
}

void DevUI::RenderAudioDebug(audio::AudioSystem* audioSys) {
    if (!audioSys) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "AudioSystem is not initialized or null.");
        return;
    }

    ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "Wildbound Procedural Audio Subsystem");
    ImGui::TextDisabled("Catalog: %zu loaded assets | Buses: 6 | Pure Procedural Synthesis", audioSys->GetAssetCount());
    ImGui::Separator();

    // 1. Bus Volume Controls & Global Mute
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Audio Buses & Master Controls");

    bool muteAll = audioSys->IsMuteAll();
    if (ImGui::Checkbox("Mute All Audio", &muteAll)) {
        audioSys->SetMuteAll(muteAll);
    }

    float masterVol = audioSys->GetBusVolume(audio::AudioBus::Master);
    if (ImGui::SliderFloat("Master Bus", &masterVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetMasterVolume(masterVol);
    }

    float musicVol = audioSys->GetBusVolume(audio::AudioBus::Music);
    if (ImGui::SliderFloat("Music Bus", &musicVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::Music, musicVol);
    }

    float uiVol = audioSys->GetBusVolume(audio::AudioBus::UI);
    if (ImGui::SliderFloat("UI Bus", &uiVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::UI, uiVol);
    }

    float sfxVol = audioSys->GetBusVolume(audio::AudioBus::SFX);
    if (ImGui::SliderFloat("SFX Bus", &sfxVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::SFX, sfxVol);
    }

    float envVol = audioSys->GetBusVolume(audio::AudioBus::Environment);
    if (ImGui::SliderFloat("Environment Bus", &envVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::Environment, envVol);
    }

    float combatVol = audioSys->GetBusVolume(audio::AudioBus::Combat);
    if (ImGui::SliderFloat("Combat Bus", &combatVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::Combat, combatVol);
    }

    float creaturesVol = audioSys->GetBusVolume(audio::AudioBus::Creatures);
    if (ImGui::SliderFloat("Creatures Bus", &creaturesVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::Creatures, creaturesVol);
    }

    float voiceVol = audioSys->GetBusVolume(audio::AudioBus::Voice);
    if (ImGui::SliderFloat("Voice Bus", &voiceVol, 0.0f, 1.0f, "%.2f")) {
        audioSys->SetBusVolume(audio::AudioBus::Voice, voiceVol);
    }

    if (ImGui::Button("Save Settings to Disk")) {
        audioSys->SaveSettings();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload Settings")) {
        audioSys->LoadSettings();
    }

    ImGui::Separator();

    // 2. Music Streaming Player
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Procedural Music Player");

    std::string currentMusic = audioSys->GetCurrentMusicId();
    bool isPlaying = audioSys->IsMusicPlaying();
    ImGui::Text("Current Track: %s [%s]",
                currentMusic.empty() ? "(None)" : currentMusic.c_str(),
                isPlaying ? "PLAYING" : "STOPPED / PAUSED");

    auto musicAssets = audioSys->GetAssetsByBus(audio::AudioBus::Music);
    if (!musicAssets.empty()) {
        std::vector<const char*> musicNames;
        musicNames.reserve(musicAssets.size());
        for (const auto* m : musicAssets) {
            musicNames.push_back(m->id.c_str());
        }

        if (m_selectedMusicIndex < 0 || m_selectedMusicIndex >= static_cast<int>(musicNames.size())) {
            m_selectedMusicIndex = 0;
        }

        ImGui::Combo("Select Music", &m_selectedMusicIndex, musicNames.data(), static_cast<int>(musicNames.size()));

        if (ImGui::Button("Play Selected")) {
            audioSys->PlayMusic(musicNames[m_selectedMusicIndex], true, 0.5f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            audioSys->PauseMusic();
        }
        ImGui::SameLine();
        if (ImGui::Button("Resume")) {
            audioSys->ResumeMusic();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            audioSys->StopMusic(0.5f);
        }
    }

    ImGui::Separator();

    // 3. Biome Ambience Switcher
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Biome Ambience Preview");
    ImGui::Text("Current Biome: %s", audioSys->GetCurrentBiome().c_str());

    if (ImGui::Button("Forest"))   { audioSys->SetBiome("forest"); }
    ImGui::SameLine();
    if (ImGui::Button("Desert"))   { audioSys->SetBiome("desert"); }
    ImGui::SameLine();
    if (ImGui::Button("Snow"))     { audioSys->SetBiome("snow"); }
    ImGui::SameLine();
    if (ImGui::Button("Swamp"))    { audioSys->SetBiome("swamp"); }
    ImGui::SameLine();
    if (ImGui::Button("Mountain")) { audioSys->SetBiome("mountain"); }
    ImGui::SameLine();
    if (ImGui::Button("Village"))  { audioSys->SetBiome("village"); }

    ImGui::Separator();

    // 4. SFX Soundboard & Event Dispatch
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Event Dispatch & Soundboard");

    if (ImGui::Button("UI Click"))    { audioSys->DispatchEvent("ui.button.click"); }
    ImGui::SameLine();
    if (ImGui::Button("UI Hover"))    { audioSys->DispatchEvent("ui.button.hover"); }
    ImGui::SameLine();
    if (ImGui::Button("UI Confirm"))  { audioSys->DispatchEvent("ui.button.confirm"); }
    ImGui::SameLine();
    if (ImGui::Button("UI Error"))    { audioSys->DispatchEvent("ui.error"); }

    if (ImGui::Button("Grass Step"))  { audioSys->DispatchEvent("player.footstep.grass"); }
    ImGui::SameLine();
    if (ImGui::Button("Stone Step"))  { audioSys->DispatchEvent("player.footstep.stone"); }
    ImGui::SameLine();
    if (ImGui::Button("Wood Step"))   { audioSys->DispatchEvent("player.footstep.wood"); }
    ImGui::SameLine();
    if (ImGui::Button("Water Step"))  { audioSys->DispatchEvent("player.footstep.water"); }

    if (ImGui::Button("Sword Swing")) { audioSys->DispatchEvent("weapon.sword.swing"); }
    ImGui::SameLine();
    if (ImGui::Button("Bow Release")) { audioSys->DispatchEvent("weapon.bow.release"); }
    ImGui::SameLine();
    if (ImGui::Button("Shield Block")){ audioSys->DispatchEvent("weapon.shield.block"); }
    ImGui::SameLine();
    if (ImGui::Button("Parry!"))      { audioSys->DispatchEvent("weapon.parry"); }

    if (ImGui::Button("Goblin Atk"))  { audioSys->DispatchEvent("creature.goblin.attack"); }
    ImGui::SameLine();
    if (ImGui::Button("Goblin Hurt")) { audioSys->DispatchEvent("creature.goblin.hurt"); }
    ImGui::SameLine();
    if (ImGui::Button("Wolf Growl"))  { audioSys->DispatchEvent("creature.wolf.growl"); }
    ImGui::SameLine();
    if (ImGui::Button("Boar Charge")) { audioSys->DispatchEvent("creature.wild_boar.attack"); }
    ImGui::SameLine();
    if (ImGui::Button("Bat Shriek"))  { audioSys->DispatchEvent("creature.bat.attack"); }

    if (ImGui::Button("Pickup Item")) { audioSys->DispatchEvent("item.pickup"); }
    ImGui::SameLine();
    if (ImGui::Button("Drink Potion")){ audioSys->DispatchEvent("item.consume"); }
    ImGui::SameLine();
    if (ImGui::Button("Open Chest"))  { audioSys->DispatchEvent("environment.chest.open"); }
    ImGui::SameLine();
    if (ImGui::Button("Victory!"))    { audioSys->DispatchEvent("match.victory"); }

    ImGui::Separator();

    // 5. Spatial Audio Diagnostic
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Spatial Audio 2D Tester");
    static float s_spatialOffset[2] = { 150.0f, 0.0f };
    static float s_spatialMaxDist = 500.0f;
    ImGui::SliderFloat2("Source Offset (X, Y)", s_spatialOffset, -400.0f, 400.0f);
    ImGui::SliderFloat("Max Distance", &s_spatialMaxDist, 100.0f, 1000.0f);

    Vector2 listener = audioSys->GetListenerPosition();
    Vector2 sourcePos = { listener.x + s_spatialOffset[0], listener.y + s_spatialOffset[1] };
    ImGui::Text("Listener: (%.1f, %.1f) | Source: (%.1f, %.1f)", listener.x, listener.y, sourcePos.x, sourcePos.y);

    if (ImGui::Button("Play Spatial Clang (Metal)")) {
        audioSys->PlaySoundSpatial("sfx_sword_impact_metal", sourcePos, s_spatialMaxDist);
    }
    ImGui::SameLine();
    if (ImGui::Button("Play Spatial Creature Snarl")) {
        audioSys->PlaySoundSpatial("sfx_creature_wolf_growl", sourcePos, s_spatialMaxDist);
    }

    ImGui::Separator();

    // 6. Individual SFX Catalog Preview
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Individual SFX Audition");

    auto sfxAssets = audioSys->GetAssetsByBus(audio::AudioBus::SFX);
    auto uiAssets = audioSys->GetAssetsByBus(audio::AudioBus::UI);
    auto combatAssets = audioSys->GetAssetsByBus(audio::AudioBus::Combat);
    auto creatureAssets = audioSys->GetAssetsByBus(audio::AudioBus::Creatures);
    std::vector<const audio::AudioAssetInfo*> allSfx;
    allSfx.insert(allSfx.end(), sfxAssets.begin(), sfxAssets.end());
    allSfx.insert(allSfx.end(), uiAssets.begin(), uiAssets.end());
    allSfx.insert(allSfx.end(), combatAssets.begin(), combatAssets.end());
    allSfx.insert(allSfx.end(), creatureAssets.begin(), creatureAssets.end());

    if (!allSfx.empty()) {
        std::vector<const char*> sfxNames;
        sfxNames.reserve(allSfx.size());
        for (const auto* s : allSfx) {
            sfxNames.push_back(s->id.c_str());
        }

        if (m_selectedSfxIndex < 0 || m_selectedSfxIndex >= static_cast<int>(sfxNames.size())) {
            m_selectedSfxIndex = 0;
        }

        ImGui::Combo("Select SFX", &m_selectedSfxIndex, sfxNames.data(), static_cast<int>(sfxNames.size()));
        ImGui::SliderFloat("Preview Vol", &m_previewSfxVolume, 0.1f, 1.0f, "%.2f");
        ImGui::SliderFloat("Preview Pitch", &m_previewSfxPitch, 0.5f, 2.0f, "%.2f");

        if (ImGui::Button("Audition SFX")) {
            audioSys->PlaySound(sfxNames[m_selectedSfxIndex], m_previewSfxVolume, m_previewSfxPitch);
        }
    }
}

void DevUI::RenderPhysicsAndCollision(physics::CollisionManager* colMgr) {
    if (!colMgr) {
        ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "CollisionManager not available.");
        return;
    }

    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.6f, 1.0f), "Collision Subsystem (Phase 20)");
    ImGui::Text("Deterministic Update & Bilateral Mask Filtering");
    ImGui::Separator();

    // 1. Diagnostics & Metrics
    if (ImGui::CollapsingHeader("Collision Metrics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Total Colliders:    %zu (Active: %zu)", colMgr->GetTotalColliderCount(), colMgr->GetActiveColliderCount());
        ImGui::Text("Active Overlaps:    %zu", colMgr->GetActiveOverlapCount());
        ImGui::Text("Events Dispatched:  %zu", colMgr->GetEventsDispatchedLastTick());
        ImGui::Text("Pairs Tested:       %zu", colMgr->GetPairsTestedLastTick());
        ImGui::Text("Pairs Filtered Out: %zu", colMgr->GetPairsFilteredLastTick());
    }

    // 2. Layer & Mask Matrix Validator
    if (ImGui::CollapsingHeader("Layer & Bilateral Mask Matrix", ImGuiTreeNodeFlags_DefaultOpen)) {
        static int selectedLayerA = 1; // Player
        static int selectedLayerB = 0; // World

        const char* layerNames[] = {
            "World", "Player", "Enemy", "Wildlife", "Projectile",
            "Pickup", "Trigger", "Building", "Hazard", "Interactable"
        };
        const physics::CollisionLayer layers[] = {
            physics::CollisionLayer::World,
            physics::CollisionLayer::Player,
            physics::CollisionLayer::Enemy,
            physics::CollisionLayer::Wildlife,
            physics::CollisionLayer::Projectile,
            physics::CollisionLayer::Pickup,
            physics::CollisionLayer::Trigger,
            physics::CollisionLayer::Building,
            physics::CollisionLayer::Hazard,
            physics::CollisionLayer::Interactable
        };

        ImGui::Combo("Layer A", &selectedLayerA, layerNames, IM_ARRAYSIZE(layerNames));
        ImGui::Combo("Layer B", &selectedLayerB, layerNames, IM_ARRAYSIZE(layerNames));

        auto layA = layers[selectedLayerA];
        auto layB = layers[selectedLayerB];
        uint32_t maskA = physics::GetDefaultCollisionMask(layA);
        uint32_t maskB = physics::GetDefaultCollisionMask(layB);

        bool willCollide = physics::CollisionFilter::ShouldCollide(layA, maskA, layB, maskB);
        if (willCollide) {
            ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "Result: COLLISION MUTUALLY ACCEPTED (Eligible)");
        } else {
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Result: FILTERED OUT (No interaction)");
        }
    }

    // 3. Active Overlaps Table
    if (ImGui::CollapsingHeader("Active Overlaps", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto& overlaps = colMgr->GetCurrentOverlaps();
        if (overlaps.empty()) {
            ImGui::TextDisabled("No active overlaps this tick.");
        } else {
            if (ImGui::BeginTable("OverlapsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 140))) {
                ImGui::TableSetupColumn("Pair (A, B)");
                ImGui::TableSetupColumn("Layer A");
                ImGui::TableSetupColumn("Layer B");
                ImGui::TableSetupColumn("Trigger?");
                ImGui::TableSetupColumn("Overlap Bounds");
                ImGui::TableHeadersRow();

                for (const auto& [pair, rect] : overlaps) {
                    const auto* colA = colMgr->GetCollider(pair.first);
                    const auto* colB = colMgr->GetCollider(pair.second);
                    if (!colA || !colB) continue;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("[%llu, %llu]", pair.first, pair.second);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", physics::CollisionLayerToString(colA->GetLayer()).data());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", physics::CollisionLayerToString(colB->GetLayer()).data());

                    ImGui::TableSetColumnIndex(3);
                    bool isTrig = colA->IsTrigger() || colB->IsTrigger();
                    ImGui::TextColored(isTrig ? ImVec4(0.4f, 0.7f, 1.0f, 1.0f) : ImVec4(0.9f, 0.9f, 0.9f, 1.0f),
                                       isTrig ? "TRIGGER" : "PHYSICAL");

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("(%.1f, %.1f) %.1fx%.1f", rect.x, rect.y, rect.width, rect.height);
                }
                ImGui::EndTable();
            }
        }
    }
}

void DevUI::RenderInputDebug(input::InputManager* inputMgr) {
    if (!inputMgr) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "InputManager subsystem not initialized.");
        return;
    }

    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "=== ADVANCED INPUT MANAGEMENT (PHASE 21) ===");
    ImGui::Separator();

    // 1. Device & Context Overview
    auto devType = inputMgr->GetActiveDevice();
    const char* devStr = (devType == input::DeviceType::Gamepad) ? "Gamepad / Controller" : "Keyboard & Mouse";
    ImGui::Text("Active Input Device: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", devStr);

    auto& ctxMgr = inputMgr->GetContextManager();
    const auto& activeCtx = ctxMgr.GetActiveContext();
    ImGui::Text("Active Context:      [%s] (Priority: %d, Modal: %s)",
                activeCtx.name.c_str(), activeCtx.priority, activeCtx.isModal ? "YES" : "NO");

    ImGui::Text("Context Stack Depth: %zu", ctxMgr.GetStackDepth());
    ImGui::SameLine();
    if (ImGui::Button("Reset Context Stack")) {
        inputMgr->SetContext(input::InputContextType::Gameplay);
    }

    ImGui::Separator();

    // 2. Hardware Input Telemetry
    if (ImGui::CollapsingHeader("Hardware Telemetry", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(2, "HardwareColumns", true);

        // Column 1: Mouse & Keyboard
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "Mouse & Pointer:");
        Vector2 mScr = inputMgr->GetMouseScreenPosition();
        Vector2 mVirt = inputMgr->GetMouseVirtualPosition();
        Vector2 mWorld = inputMgr->GetMouseWorldPosition();
        ImGui::Text("  Screen (Window):  (%.1f, %.1f)", mScr.x, mScr.y);
        ImGui::Text("  Virtual (Canvas): (%.1f, %.1f)", mVirt.x, mVirt.y);
        ImGui::Text("  World Space:      (%.1f, %.1f)", mWorld.x, mWorld.y);
        ImGui::Text("  Wheel Delta:      %.2f", inputMgr->GetMouseWheelMove());

        ImGui::NextColumn();

        // Column 2: Gamepad
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "Gamepad (Slot 0):");
        const auto& gp = inputMgr->GetGamepad();
        bool gpConn = gp.IsConnected(0);
        ImGui::Text("  Status: %s (%s)", gpConn ? "CONNECTED" : "DISCONNECTED", gp.GetName(0).c_str());
        Vector2 ls = gp.GetLeftStick(0);
        Vector2 rs = gp.GetRightStick(0);
        ImGui::Text("  Left Stick:  (%.2f, %.2f)", ls.x, ls.y);
        ImGui::Text("  Right Stick: (%.2f, %.2f)", rs.x, rs.y);
        ImGui::Text("  Triggers:    LT: %.2f | RT: %.2f", gp.GetLeftTrigger(0), gp.GetRightTrigger(0));

        float deadzone = inputMgr->GetSettings().GetGamepadDeadzone();
        if (ImGui::SliderFloat("Deadzone", &deadzone, 0.05f, 0.50f, "%.2f")) {
            inputMgr->GetSettings().SetGamepadDeadzone(deadzone);
            inputMgr->GetGamepad().SetDeadzone(deadzone);
        }

        ImGui::Columns(1);
    }

    // 3. Movement & Actions
    if (ImGui::CollapsingHeader("Logical Actions & Buffer", ImGuiTreeNodeFlags_DefaultOpen)) {
        Vector2 moveVec = inputMgr->GetMovementVector(false);
        ImGui::Text("Composite Movement Vector: (%.2f, %.2f)", moveVec.x, moveVec.y);

        if (ImGui::BeginTable("ActionsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 200))) {
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Held", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Pressed", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Released", ImGuiTableColumnFlags_WidthFixed, 65.0f);
            ImGui::TableSetupColumn("Buffered", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            const auto& buf = inputMgr->GetBuffer();
            for (size_t i = 0; i < input::ACTION_COUNT; ++i) {
                auto act = static_cast<input::InputAction>(i);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", input::ActionToString(act).data());

                ImGui::TableSetColumnIndex(1);
                auto cat = input::GetActionCategory(act);
                const char* catStr = (cat == input::ActionCategory::Movement) ? "Movement" :
                                     (cat == input::ActionCategory::Combat) ? "Combat" :
                                     (cat == input::ActionCategory::Inventory) ? "Inventory" :
                                     (cat == input::ActionCategory::UI) ? "UI" :
                                     (cat == input::ActionCategory::Debug) ? "Debug" : "General";
                ImGui::Text("%s", catStr);

                ImGui::TableSetColumnIndex(2);
                bool held = inputMgr->IsActionHeld(act);
                ImGui::TextColored(held ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                                   held ? "YES" : "NO");

                ImGui::TableSetColumnIndex(3);
                bool pressed = inputMgr->WasActionPressed(act);
                ImGui::TextColored(pressed ? ImVec4(1.0f, 0.9f, 0.2f, 1.0f) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                                   pressed ? "YES" : "NO");

                ImGui::TableSetColumnIndex(4);
                bool rel = inputMgr->WasActionReleased(act);
                ImGui::TextColored(rel ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                                   rel ? "YES" : "NO");

                ImGui::TableSetColumnIndex(5);
                const auto& bItem = buf.GetBufferedState(act);
                if (!bItem.consumed && bItem.timeRemaining > 0.0f) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "ACTIVE (%.0f ms)", bItem.timeRemaining * 1000.0f);
                } else {
                    ImGui::TextDisabled("Idle");
                }
            }
            ImGui::EndTable();
        }
    }

    // 4. Control Bindings & Rebinding
    if (ImGui::CollapsingHeader("Control Bindings & Remapping", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& settings = inputMgr->GetSettings();
        if (settings.IsRebinding()) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                               "REBINDING IN PROGRESS: Press any key/button to bind to [%s] (Esc to cancel)...",
                               input::ActionToString(settings.GetRebindingAction()).data());
            if (ImGui::Button("Cancel Rebinding")) {
                settings.CancelRebinding();
            }
        }

        if (ImGui::Button("Reset to Defaults")) {
            settings.ResetToDefaults();
        }
        ImGui::SameLine();
        if (ImGui::Button("Save to config/input_bindings.json")) {
            settings.SaveToFile("config/input_bindings.json");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load from config/input_bindings.json")) {
            settings.LoadFromFile("config/input_bindings.json");
        }

        if (ImGui::BeginTable("BindingsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 220))) {
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableSetupColumn("Primary", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Alternate", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Gamepad", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < input::ACTION_COUNT; ++i) {
                auto act = static_cast<input::InputAction>(i);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", input::ActionToString(act).data());

                ImGui::TableSetColumnIndex(1);
                std::string pStr = input::BindingToString(settings.GetPrimaryBinding(act));
                std::string pBtnId = pStr + "##p_" + std::to_string(i);
                if (ImGui::SmallButton(pBtnId.c_str())) {
                    settings.StartRebinding(act, input::DeviceType::Keyboard, false);
                }

                ImGui::TableSetColumnIndex(2);
                std::string aStr = input::BindingToString(settings.GetAlternateBinding(act));
                std::string aBtnId = aStr + "##a_" + std::to_string(i);
                if (ImGui::SmallButton(aBtnId.c_str())) {
                    settings.StartRebinding(act, input::DeviceType::Keyboard, true);
                }

                ImGui::TableSetColumnIndex(3);
                std::string gStr = input::BindingToString(settings.GetGamepadBinding(act));
                std::string gBtnId = gStr + "##g_" + std::to_string(i);
                if (ImGui::SmallButton(gBtnId.c_str())) {
                    settings.StartRebinding(act, input::DeviceType::Gamepad, false);
                }
            }
            ImGui::EndTable();
        }
    }
}

void DevUI::RenderInteractionDebug(interaction::InteractionManager* interactionMgr, entities::Player* player) {
    if (!interactionMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "InteractionManager is not initialized.");
        return;
    }

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "World Interaction System Diagnostics (Phase 22)");
    ImGui::Separator();

    // 1. Toggles & Policies
    ImGui::Checkbox("Show Interaction In-World Visualization", &m_showInteractionDebug);
    
    auto policy = interactionMgr->GetPickupPolicy();
    const char* policyStr = (policy == interaction::PickupPolicy::Manual) ? "Manual (Press [E] to Collect)" : "Automatic (Proximity)";
    if (ImGui::BeginCombo("Item Pickup Policy", policyStr)) {
        if (ImGui::Selectable("Manual (Press [E] to Collect)", policy == interaction::PickupPolicy::Manual)) {
            interactionMgr->SetPickupPolicy(interaction::PickupPolicy::Manual);
        }
        if (ImGui::Selectable("Automatic (Proximity)", policy == interaction::PickupPolicy::Automatic)) {
            interactionMgr->SetPickupPolicy(interaction::PickupPolicy::Automatic);
        }
        ImGui::EndCombo();
    }

    float range = interactionMgr->GetDefaultRange();
    if (ImGui::SliderFloat("Default Interaction Radius", &range, 24.0f, 120.0f, "%.1f px")) {
        interactionMgr->SetDefaultRange(range);
    }
    ImGui::Separator();

    // 2. Current Target & Prompt
    const auto* target = interactionMgr->GetCurrentTarget();
    const auto& prompt = interactionMgr->GetCurrentPrompt();

    ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Current Target & Prompt:");
    if (target) {
        ImGui::Text("  Target Entity: [%s] (ID: %llu)", target->definition.displayName.c_str(), target->id);
        ImGui::Text("  Category:      %s", interaction::CategoryToString(target->definition.category).c_str());
        ImGui::Text("  Position:      (%.1f, %.1f) | Range: %.1f px", target->position.x, target->position.y, target->definition.range);
        ImGui::Text("  Priority:      %d | Cooldown: %.2f s", target->definition.priority, target->currentCooldown);
        if (!target->definition.requiredItemId.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "  Required Item: %s", target->definition.requiredItemId.c_str());
        }
    } else {
        ImGui::TextDisabled("  No interactable target in range.");
    }

    if (prompt.visible) {
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.9f, 1.0f), "  Active Prompt: %s", prompt.GetFormattedPrompt().c_str());
    }
    if (!prompt.feedbackMessage.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "  Feedback:      %s (%.1fs remaining)", prompt.feedbackMessage.c_str(), prompt.feedbackTimer);
    }
    ImGui::Separator();

    // 3. Candidate Target Ranking Table
    const auto& candidates = interactionMgr->GetCurrentCandidates();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Nearby Candidates Ranked (%zu found):", candidates.size());

    if (!candidates.empty()) {
        if (ImGui::BeginTable("CandidatesTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Rank");
            ImGui::TableSetupColumn("Target Name");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Distance");
            ImGui::TableSetupColumn("Facing Dot");
            ImGui::TableSetupColumn("Priority");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < candidates.size(); ++i) {
                const auto& c = candidates[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (i == 0) {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "#%zu [SELECTED]", i + 1);
                } else {
                    ImGui::Text("#%zu", i + 1);
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", c.component->definition.displayName.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", interaction::CategoryToString(c.component->definition.category).c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f px", c.distance);

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.2f", c.facingAlignment);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%d", c.priority);
            }
            ImGui::EndTable();
        }
    }
    ImGui::Separator();

    // 4. World Doors Inspection & Control
    const auto& doors = interactionMgr->GetAllDoors();
    if (ImGui::CollapsingHeader("World Doors", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Total Doors: %zu", doors.size());
        for (const auto& d : doors) {
            std::string stateStr = interaction::DoorStateToString(d.GetState());
            ImVec4 col = d.IsOpen() ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) :
                         d.IsLocked() ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.9f, 0.8f, 0.5f, 1.0f);

            ImGui::Bullet();
            ImGui::Text("%s [%s]: ", d.GetName().c_str(), d.GetId().c_str());
            ImGui::SameLine();
            ImGui::TextColored(col, "%s", stateStr.c_str());
            ImGui::SameLine();
            std::string toggleBtnId = "Toggle##door_" + d.GetId();
            if (ImGui::SmallButton(toggleBtnId.c_str())) {
                const_cast<interaction::Door&>(d).Toggle();
            }
        }
    }

    // 5. World Mechanisms Inspection & Control
    const auto& mechanisms = interactionMgr->GetAllMechanisms();
    if (ImGui::CollapsingHeader("World Mechanisms", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Total Mechanisms: %zu", mechanisms.size());
        for (const auto& m : mechanisms) {
            std::string stateStr = m.IsActive() ? "ACTIVE" : "INACTIVE";
            ImVec4 col = m.IsActive() ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

            ImGui::Bullet();
            ImGui::Text("%s [%s]: ", m.GetName().c_str(), m.GetId().c_str());
            ImGui::SameLine();
            ImGui::TextColored(col, "%s", stateStr.c_str());
            ImGui::SameLine();
            std::string toggleBtnId = "Toggle##mech_" + m.GetId();
            if (ImGui::SmallButton(toggleBtnId.c_str())) {
                const_cast<interaction::Mechanism&>(m).Toggle();
            }
        }
    }

    // 6. Registered Interactables Registry
    if (ImGui::CollapsingHeader("All Registered Interactables")) {
        ImGui::Text("Registry Count: %zu", interactionMgr->GetInteractableCount());
        const auto& all = interactionMgr->GetAllInteractables();
        for (const auto& [id, comp] : all) {
            ImGui::Text("ID %llu: [%s] (%s) @ (%.0f, %.0f) - Avail: %s",
                        id, comp.definition.displayName.c_str(),
                        interaction::CategoryToString(comp.definition.category).c_str(),
                        comp.position.x, comp.position.y,
                        comp.isAvailable ? "YES" : "NO");
        }
    }
}

void DevUI::RenderLootAndInventory(entities::Player* player, world::World* world, items::ItemManager* itemMgr, audio::AudioSystem* audioSys) {
    ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "Loot & Inventory System (Phase 23)");
    ImGui::Separator();

    if (!player) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "No active player instance available.");
        return;
    }

    auto& inv = player->GetInventory();
    Vector2 playerPos = player->GetPosition();
    Vector2 playerFacing = player->GetFacingDirection();

    // 1. Inventory Summary & Controls
    if (ImGui::CollapsingHeader("Player Inventory (24 Slots)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Filled Slots: %zu / %zu (Free: %zu)",
                    inv.GetFilledSlotCount(), inv.GetCapacity(), inv.GetFreeSlotCount());
        ImGui::SameLine();
        if (ImGui::Button("Clear All Slots")) {
            inv.Clear();
            if (audioSys) audioSys->DispatchEvent("UI.ButtonClick");
        }

        if (ImGui::BeginTable("InventorySlotsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("Item ID", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 65.0f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableHeadersRow();

            for (size_t s = 0; s < inv.GetCapacity(); ++s) {
                const auto& optSlot = inv.GetSlot(s);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%zu", s);

                ImGui::TableSetColumnIndex(1);
                if (optSlot.has_value()) {
                    ImGui::Text("%s", optSlot->definitionId.c_str());
                } else {
                    ImGui::TextDisabled("[Empty]");
                }

                ImGui::TableSetColumnIndex(2);
                const items::ItemDefinition* def = nullptr;
                if (optSlot.has_value() && itemMgr) {
                    def = itemMgr->GetItem(optSlot->definitionId);
                }
                if (def) {
                    ImGui::Text("%s", def->displayName.c_str());
                } else if (optSlot.has_value()) {
                    ImGui::Text("%s", optSlot->definitionId.c_str());
                } else {
                    ImGui::TextDisabled("-");
                }

                ImGui::TableSetColumnIndex(3);
                if (def) {
                    ImGui::Text("%s", items::ItemCategoryToString(def->category));
                } else {
                    ImGui::TextDisabled("-");
                }

                ImGui::TableSetColumnIndex(4);
                if (optSlot.has_value()) {
                    int maxStack = def ? def->maxStackSize : 1;
                    ImGui::Text("%d / %d", optSlot->quantity, maxStack);
                } else {
                    ImGui::TextDisabled("0");
                }

                ImGui::TableSetColumnIndex(5);
                if (optSlot.has_value()) {
                    std::string dropOneLabel = "Drop 1##slot_" + std::to_string(s);
                    if (ImGui::SmallButton(dropOneLabel.c_str())) {
                        if (world) {
                            inv.DropItem(s, 1, *world, playerPos, playerFacing, audioSys);
                        }
                    }
                    ImGui::SameLine();
                    std::string dropAllLabel = "Drop All##slot_" + std::to_string(s);
                    if (ImGui::SmallButton(dropAllLabel.c_str())) {
                        if (world) {
                            inv.DropAll(s, *world, playerPos, playerFacing, audioSys);
                        }
                    }
                    if (def && def->category == items::ItemCategory::Consumable) {
                        ImGui::SameLine();
                        std::string useLabel = "Use##slot_" + std::to_string(s);
                        if (ImGui::SmallButton(useLabel.c_str())) {
                            player->UseConsumable(s, itemMgr, audioSys);
                        }
                    }
                }
            }
            ImGui::EndTable();
        }
    }

    // 2. Ground Loot Spawner
    if (ImGui::CollapsingHeader("Ground Loot Spawner & Debug Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (!world) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "No active world loaded for loot spawning.");
        } else {
            ImGui::Text("Spawn items directly at player position (%.1f, %.1f):", playerPos.x, playerPos.y);
            if (ImGui::Button("Spawn Small Health Potion (+50 HP)")) {
                world->SpawnWorldItem("con_health_potion_small_001", playerPos, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Spawn Large Health Potion (+100 HP)")) {
                world->SpawnWorldItem("con_health_potion_large_001", playerPos, 1);
            }

            if (ImGui::Button("Spawn Iron Arrows (x15)")) {
                world->SpawnWorldItem("arrow_iron_001", playerPos, 15);
            }
            ImGui::SameLine();
            if (ImGui::Button("Spawn Gold Coins (x50)")) {
                world->SpawnWorldItem("col_gold_coin_001", playerPos, 50);
            }
            ImGui::SameLine();
            if (ImGui::Button("Spawn Cooked Meat (x3)")) {
                world->SpawnWorldItem("con_cooked_meat_001", playerPos, 3);
            }

            if (ImGui::Button("Spawn Iron Sword")) {
                world->SpawnWorldItem("sword_iron_001", playerPos, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Spawn Hunting Bow")) {
                world->SpawnWorldItem("bow_wooden_hunting_001", playerPos, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear All Ground Loot")) {
                world->ClearWorldItems();
            }
        }
    }

    // 3. Active Ground Loot Inspection Table
    if (ImGui::CollapsingHeader("Active Ground Loot Registry")) {
        if (!world) {
            ImGui::TextDisabled("No active world.");
        } else {
            const auto& items = world->GetWorldItems();
            ImGui::Text("Active Ground Items: %zu", items.size());

            if (ImGui::BeginTable("GroundLootTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Item ID", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Radius", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Collected", ImGuiTableColumnFlags_WidthFixed, 70.0f);
                ImGui::TableHeadersRow();

                for (const auto& wi : items) {
                    if (!wi) continue;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%llu", wi->GetId());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", wi->GetItem().definitionId.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", wi->GetItem().quantity);

                    ImGui::TableSetColumnIndex(3);
                    Vector2 pos = wi->GetPosition();
                    ImGui::Text("(%.1f, %.1f)", pos.x, pos.y);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%.1f", wi->GetPickupRadius());

                    ImGui::TableSetColumnIndex(5);
                    if (wi->IsCollected()) {
                        ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "YES");
                    } else {
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "NO");
                    }
                }
                ImGui::EndTable();
            }
        }
    }
}

void DevUI::RenderUIAndHUD(ui::UIManager* uiMgr, entities::Player* player) {
    if (!uiMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "UIManager instance is not available.");
        return;
    }

    ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "UI & HUD Management Architecture");
    ImGui::Separator();

    // 1. Screen State & Switching
    UIScreen currentScreen = uiMgr->GetCurrentScreen();
    const char* screenNames[] = {
        "None (Gameplay HUD)", "MainMenu", "CharacterSelect", "Wardrobe",
        "Store", "Inventory", "Settings", "PauseMenu", "MatchResults", "GameOver"
    };
    int currentIdx = static_cast<int>(currentScreen);
    ImGui::Text("Current Screen: %s", (currentIdx >= 0 && currentIdx < 10) ? screenNames[currentIdx] : "Unknown");

    ImGui::Spacing();
    ImGui::Text("Quick Switch Screens:");
    if (ImGui::Button("HUD (None)")) uiMgr->SetScreen(UIScreen::None);
    ImGui::SameLine();
    if (ImGui::Button("Main Menu")) uiMgr->SetScreen(UIScreen::MainMenu);
    ImGui::SameLine();
    if (ImGui::Button("Pause Menu")) uiMgr->SetScreen(UIScreen::PauseMenu);
    ImGui::SameLine();
    if (ImGui::Button("Inventory")) uiMgr->SetScreen(UIScreen::Inventory);

    if (ImGui::Button("Settings")) uiMgr->SetScreen(UIScreen::Settings);
    ImGui::SameLine();
    if (ImGui::Button("Wardrobe")) uiMgr->SetScreen(UIScreen::Wardrobe);
    ImGui::SameLine();
    if (ImGui::Button("Cosmetics Store")) uiMgr->SetScreen(UIScreen::Store);
    ImGui::SameLine();
    if (ImGui::Button("Match Results")) uiMgr->SetScreen(UIScreen::MatchResults);

    ImGui::Separator();

    // 2. HUD & Vitals Diagnostics
    HUDManager& hud = uiMgr->GetHUDManager();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "HUD Vitals & Lag Bar");
    bool hudVis = hud.IsHUDVisible();
    if (ImGui::Checkbox("HUD Elements Visible", &hudVis)) {
        hud.SetHUDVisible(hudVis);
    }

    ImGui::Text("Displayed HP: %.1f", hud.GetDisplayedHealth());
    ImGui::Text("Lag Bar HP:   %.1f", hud.GetLagHealth());
    ImGui::Text("Low HP Alert: %s", hud.IsLowHealthAlertActive() ? "ACTIVE (Pulse ON)" : "Inactive");

    if (player) {
        if (ImGui::Button("Simulate Take 25 Damage")) {
            player->GetHealth().ApplyDamage(25.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Simulate Heal 25 HP")) {
            player->GetHealth().ApplyHeal(25.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Drop HP to 15 (Alert)")) {
            player->GetHealth().SetCurrentHealth(15.0f);
        }
    }

    ImGui::Separator();

    // 3. Minimap Controls
    Minimap& mm = hud.GetMinimap();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Minimap Radar");
    bool mmVis = mm.IsVisible();
    if (ImGui::Checkbox("Minimap Visible", &mmVis)) {
        mm.SetVisible(mmVis);
    }
    ImGui::SameLine();
    ImGui::Text("Current Zoom: %.1fx", mm.GetZoom());
    ImGui::SameLine();
    if (ImGui::Button("Cycle Zoom")) {
        mm.CycleZoom();
    }

    ImGui::Separator();

    // 4. Match State Controls
    MatchHUDInfo& match = hud.GetMatchInfo();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Match Status HUD");
    ImGui::SliderInt("Players Alive", &match.playersAlive, 1, 100);
    ImGui::SliderInt("Eliminations", &match.eliminations, 0, 50);
    ImGui::SliderFloat("Match Timer (sec)", &match.matchTimer, 0.0f, 1800.0f);

    ImGui::Separator();

    // 5. Toast Notifications
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Notification Toast Dispatcher");
    ImGui::InputText("Message", m_notificationTextBuffer, sizeof(m_notificationTextBuffer));
    if (ImGui::Button("Dispatch Toast")) {
        hud.GetNotifications().Push(m_notificationTextBuffer, "", UITheme::Gold, 3.5f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All Toasts")) {
        hud.GetNotifications().Clear();
    }
    ImGui::Text("Active Toasts in Queue: %zu", hud.GetNotifications().GetCount());
}

void DevUI::RenderStormDebug(storm::StormManager* stormMgr, entities::Player* player) {
    if (!stormMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "StormManager is not initialized.");
        return;
    }

    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "Battle Royale Corruption Storm Diagnostics");
    ImGui::Separator();

    // 1. Current State & Phase Metrics
    ImGui::Text("Storm State:      %s", storm::StormStateToString(stormMgr->GetState()));
    ImGui::SameLine();
    if (stormMgr->IsPaused()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[PAUSED]");
    } else {
        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.4f, 1.0f), "[ACTIVE]");
    }

    int pIdx = stormMgr->GetCurrentPhaseIndex();
    const auto* pCfg = stormMgr->GetCurrentPhaseConfig();
    ImGui::Text("Phase:            %d / %zu (%s)", pIdx + 1, stormMgr->GetTotalPhases(),
                pCfg ? pCfg->name.c_str() : "None");
    ImGui::Text("Damage Rate:      %.1f HP/s", stormMgr->GetCurrentDPS());

    // Timer & Progress bar
    float timer = stormMgr->GetPhaseTimer();
    float dur = stormMgr->GetStateDuration();
    int min = static_cast<int>(timer) / 60;
    int sec = static_cast<int>(timer) % 60;
    float frac = dur > 0.0f ? (timer / dur) : 0.0f;

    ImGui::Text("Timer:            %02d:%02d (%.1f / %.1f s)", min, sec, timer, dur);
    ImGui::ProgressBar(frac, ImVec2(-1, 0), TextFormat("%.1f s", timer));

    ImGui::Separator();

    // 2. Geometric Safe Zones
    const auto& curZone = stormMgr->GetCurrentSafeZone();
    const auto& tgtZone = stormMgr->GetTargetSafeZone();
    ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "Active Safe Zone:");
    ImGui::Text("  Center: (%.1f, %.1f) | Radius: %.1f px", curZone.center.x, curZone.center.y, curZone.radius);

    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Target Safe Zone:");
    ImGui::Text("  Center: (%.1f, %.1f) | Radius: %.1f px", tgtZone.center.x, tgtZone.center.y, tgtZone.radius);

    ImGui::Separator();

    // 3. Player Proximity & Status
    if (player) {
        Vector2 pPos = player->GetPosition();
        float distCenter = curZone.DistanceTo(pPos);
        float signedDist = curZone.SignedDistanceToBoundary(pPos);
        auto pStatus = stormMgr->GetPlayerStatus();

        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Player Status:");
        if (pStatus == storm::PlayerStormStatus::Outside) {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "  Status:        OUTSIDE SAFE ZONE!");
        } else {
            ImGui::Text("  Status:        %s", storm::PlayerStormStatusToString(pStatus));
        }
        ImGui::Text("  Player Pos:    (%.1f, %.1f)", pPos.x, pPos.y);
        ImGui::Text("  Dist to Center: %.1f px", distCenter);
        ImGui::Text("  Dist to Border: %.1f px (%s)", std::abs(signedDist), signedDist > 0 ? "Outside" : "Inside");

        const auto& dmgSys = stormMgr->GetDamageSystem();
        ImGui::Text("  Accumulator:   %.2f HP | Total Damage Dealt: %.1f HP",
                    dmgSys.GetDamageAccumulator(), dmgSys.GetTotalDamageDealt());
    } else {
        ImGui::TextDisabled("Player entity not attached.");
    }

    ImGui::Separator();

    // 4. Developer Controls
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Developer Controls:");
    if (ImGui::Button("Start Storm")) {
        stormMgr->StartStorm();
    }
    ImGui::SameLine();
    if (ImGui::Button("Advance Phase")) {
        stormMgr->AdvancePhase();
    }
    ImGui::SameLine();
    if (ImGui::Button(stormMgr->IsPaused() ? "Resume" : "Pause")) {
        stormMgr->TogglePause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Storm")) {
        stormMgr->Reset();
    }

    if (player && ImGui::Button("Force Player Outside Zone")) {
        stormMgr->ForcePlayerOutside(player);
    }

    // 5. Visual Render Toggles
    auto& renderer = stormMgr->GetBoundaryRenderer();
    bool renderBoundary = renderer.GetRenderBoundary();
    bool renderOverlay = renderer.GetRenderOverlay();
    bool renderParticles = renderer.GetRenderParticles();

    if (ImGui::Checkbox("Render Energy Boundary Ring", &renderBoundary)) {
        renderer.SetRenderBoundary(renderBoundary);
    }
    if (ImGui::Checkbox("Render Outside Corruption Overlay", &renderOverlay)) {
        renderer.SetRenderOverlay(renderOverlay);
    }
    if (ImGui::Checkbox("Render Atmospheric Particles", &renderParticles)) {
        renderer.SetRenderParticles(renderParticles);
    }
}

void DevUI::RenderMatchDebug(match::MatchManager* matchMgr, entities::Player* player) {
    if (!matchMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "MatchManager is not initialized.");
        return;
    }

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "Battle Royale Match Lifecycle System (Phase 32)");
    ImGui::Separator();

    // 1. Match State & Timers
    auto state = matchMgr->GetState();
    ImVec4 stateColor = ImVec4(0.5f, 0.9f, 0.5f, 1.0f);
    if (state == match::MatchState::Failed) stateColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    else if (state == match::MatchState::Countdown) stateColor = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
    else if (state == match::MatchState::Active) stateColor = ImVec4(0.2f, 0.9f, 1.0f, 1.0f);

    ImGui::TextColored(stateColor, "Current State:    %s", matchMgr->GetStateString());
    ImGui::Text("Match ID:         %s", matchMgr->GetConfig().matchId.c_str());
    ImGui::Text("World ID:         %s", matchMgr->GetConfig().worldId.c_str());

    float matchTime = matchMgr->GetMatchTime();
    int mins = static_cast<int>(matchTime) / 60;
    int secs = static_cast<int>(matchTime) % 60;
    ImGui::Text("Match Elapsed:    %02d:%02d (%.1f s)", mins, secs, matchTime);

    if (state == match::MatchState::Countdown) {
        const auto& cd = matchMgr->GetCountdown();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Countdown:        %.1f s remaining", cd.GetTimeRemaining());
        ImGui::ProgressBar(cd.GetProgress(), ImVec2(-1, 0));
    }

    ImGui::Separator();

    // 2. Participant Registry Summary
    const auto& reg = matchMgr->GetRegistry();
    size_t total = reg.GetTotalCount();
    size_t alive = reg.GetAliveCount();
    size_t elim = total - alive;

    ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.3f, 1.0f), "Competitors: %zu Total | %zu Alive | %zu Eliminated", total, alive, elim);

    const auto& participants = reg.GetAllParticipants();
    if (ImGui::BeginTable("ParticipantsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 160))) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Kills", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("Damage", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableHeadersRow();

        for (const auto& p : participants) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%u", p.participantId);

            ImGui::TableNextColumn();
            if (p.isWinner) {
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%s (WINNER)", p.displayName.c_str());
            } else if (p.type == match::ParticipantType::LocalHuman) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s (YOU)", p.displayName.c_str());
            } else {
                ImGui::Text("%s", p.displayName.c_str());
            }

            ImGui::TableNextColumn();
            if (p.IsAlive()) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Alive");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Eliminated");
            }

            ImGui::TableNextColumn();
            ImGui::Text("%u", p.kills);

            ImGui::TableNextColumn();
            ImGui::Text("%.0f", p.damageDealt);

            ImGui::TableNextColumn();
            if (p.placement > 0) {
                ImGui::Text("#%u", p.placement);
            } else {
                ImGui::Text("-");
            }
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    // 3. Developer Controls
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Lifecycle Controls:");
    if (ImGui::Button("Initialize Match")) {
        matchMgr->Initialize();
        if (player) {
            matchMgr->RegisterLocalPlayer(player);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Start Countdown")) {
        matchMgr->StartCountdown();
    }
    ImGui::SameLine();
    if (ImGui::Button("Force Start Match")) {
        matchMgr->ForceStartMatch();
    }
    ImGui::SameLine();
    if (ImGui::Button("Complete Match")) {
        matchMgr->CompleteMatch();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Match")) {
        matchMgr->ResetMatch(player);
    }

    // Add Simulated Participant for developer testing
    if (ImGui::Button("+ Add Test Competitor")) {
        match::MatchParticipant testP;
        testP.participantId = static_cast<uint32_t>(reg.GetTotalCount() + 1);
        testP.type = match::ParticipantType::AI;
        testP.displayName = "Bot_" + std::to_string(testP.participantId);
        testP.status = match::ParticipantStatus::Registered;
        testP.spawnPosition = Vector2{ 4200.0f + testP.participantId * 40.0f, 5200.0f };
        matchMgr->RegisterParticipant(testP);
    }

    // Manual Elimination
    static int selectedVictimId = 1;
    ImGui::InputInt("Target Victim ID", &selectedVictimId);
    if (ImGui::Button("Eliminate Participant")) {
        matchMgr->ProcessElimination(static_cast<uint32_t>(selectedVictimId), match::EliminationCause::Combat, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Storm Eliminate Participant")) {
        matchMgr->ProcessElimination(static_cast<uint32_t>(selectedVictimId), match::EliminationCause::CorruptionStorm, 0);
    }

    ImGui::Separator();

    // 4. Results & Post-Match Telemetry Diagnostics (Phase 33)
    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "Results & Statistics Diagnostics (Phase 33):");
    const auto& resMgr = matchMgr->GetResultManager();
    const auto& finalModel = resMgr.GetFinalResult();

    if (finalModel.isFinalized) {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Results Status: FINALIZED");
        ImGui::Text("  Match Outcome:  %s", results::MatchOutcomeToString(finalModel.matchOutcome));
        ImGui::Text("  Local Outcome:  %s", results::PlayerOutcomeToString(finalModel.localPlayerOutcome));
        ImGui::Text("  Winner:         [%s] (ID: %u)", finalModel.winnerDisplayName.c_str(), finalModel.winnerParticipantId);
        ImGui::Text("  Local Rank:     #%u of %u", finalModel.localPlayerPerformance.placement, finalModel.totalParticipants);
        ImGui::Text("  Local Survival: %.1f s", finalModel.localPlayerPerformance.survivalDuration);
        ImGui::Text("  Damage Recv:    %.0f (Combat: %.0f, Storm: %.0f)",
                    finalModel.localPlayerPerformance.totalDamageReceived,
                    finalModel.localPlayerPerformance.combatDamageReceived,
                    finalModel.localPlayerPerformance.stormDamageReceived);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "Results Status: PENDING / IN PROGRESS");
    }

    // Elimination Records Log
    const auto& elimLogs = resMgr.GetEliminationRecords();
    ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "Chronological Elimination Records (%zu events):", elimLogs.size());

    if (!elimLogs.empty()) {
        if (ImGui::BeginTable("ElimLogTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Evt#");
            ImGui::TableSetupColumn("Victim");
            ImGui::TableSetupColumn("Killer");
            ImGui::TableSetupColumn("Cause");
            ImGui::TableSetupColumn("Time");
            ImGui::TableHeadersRow();

            for (const auto& log : elimLogs) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("#%u", log.eventId);
                ImGui::TableNextColumn();
                ImGui::Text("%s", log.victimName.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", log.killerName.empty() ? "-" : log.killerName.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", match::EliminationCauseToString(log.cause));
                ImGui::TableNextColumn();
                ImGui::Text("%.1fs", log.matchElapsedTime);
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::Button("Force Finalize Results")) {
        matchMgr->GetResultManager().FinalizeMatchResult(
            matchMgr->GetConfig().matchId,
            matchMgr->GetMatchTime(),
            matchMgr->GetLocalParticipantId(),
            matchMgr->GetRegistry(),
            matchMgr->GetStatistics(),
            true
        );
    }

    // 5. Game Modes & Teams Diagnostics (Phase 34)
    if (auto* modeMgr = matchMgr->GetGameModeManager()) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Game Modes & Team Diagnostics (Phase 34):");
        const auto& activeMode = modeMgr->GetActiveMode();
        ImGui::Text("  Active Mode:     [%s] (ID: %s)", activeMode.displayName.c_str(), activeMode.id.c_str());
        ImGui::Text("  Team Structure:  %s (Team Size: %u)", activeMode.teamsEnabled ? "Enabled" : "Disabled (Solo)", activeMode.teamSize);
        ImGui::Text("  Friendly Fire:   %s", activeMode.friendlyFireEnabled ? "ENABLED" : "DISABLED");
        ImGui::Text("  Victory Rule:    %s", activeMode.victoryCondition.c_str());

        // Team Status Table
        const auto& teamMgr = modeMgr->GetTeamManager();
        size_t aliveTeams = teamMgr.GetAliveTeamCount(matchMgr->GetRegistry());
        ImGui::Text("  Total Teams:     %zu | Alive Teams: %zu", teamMgr.GetTotalTeamCount(), aliveTeams);

        if (teamMgr.GetTotalTeamCount() > 0) {
            if (ImGui::BeginTable("TeamStatusTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Team ID");
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Alive Members");
                ImGui::TableSetupColumn("Status");
                ImGui::TableHeadersRow();

                for (const auto& [tid, team] : teamMgr.GetAllTeams()) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", tid);
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", team.name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%zu / %zu", team.GetAliveMemberCount(matchMgr->GetRegistry()), team.GetMemberCount());
                    ImGui::TableNextColumn();
                    if (team.eliminated) {
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "ELIMINATED");
                    } else {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "ALIVE");
                    }
                }
                ImGui::EndTable();
            }
        }

        // Mode Switching in Developer Tools
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Developer Mode Controls:");
        if (ImGui::Button("Select Solo")) modeMgr->SelectMode("solo");
        ImGui::SameLine();
        if (ImGui::Button("Select Duos")) modeMgr->SelectMode("duos");
        ImGui::SameLine();
        if (ImGui::Button("Select Squads")) modeMgr->SelectMode("squads");
        ImGui::SameLine();
        if (ImGui::Button("Select Practice")) modeMgr->SelectMode("practice");
        ImGui::SameLine();
        if (ImGui::Button("Select Offline Bots")) modeMgr->SelectMode("offline_bots");
    }
}

void DevUI::RenderNetworkDiagnostics(network::NetworkManager* netMgr) {
    if (!netMgr) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[NETWORK] NetworkManager is not available.");
        return;
    }

    auto role = netMgr->GetRole();
    bool isServerListening = netMgr->IsServerRunning();
    bool isClientConnected = netMgr->IsClientConnected();
    auto clientState = netMgr->GetClientState();

    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "=== Phase 36: ENet Transport Layer Diagnostics ===");
    ImGui::Spacing();

    // 1. Role & Status Badges
    const char* roleStr = (role == network::NetworkRole::Server) ? "Server Host" :
                          (role == network::NetworkRole::Client) ? "Client Connection" : "Standalone / Inactive";
    ImGui::Text("Network Role: %s", roleStr);

    ImGui::SameLine();
    if (isServerListening) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[SERVER LISTENING]");
    } else if (isClientConnected) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[CLIENT CONNECTED]");
    } else if (clientState == network::ConnectionState::Connecting) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[CLIENT CONNECTING...]");
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[IDLE / DISCONNECTED]");
    }

    ImGui::Separator();

    // 2. Server Control & Management
    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Server Management:");
    ImGui::InputInt("Port##NetPort", &m_netPortBuffer);
    if (m_netPortBuffer < 1024) m_netPortBuffer = 1024;
    if (m_netPortBuffer > 65535) m_netPortBuffer = 65535;

    if (!isServerListening) {
        if (ImGui::Button("Start Server Host")) {
            netMgr->StartServer(static_cast<uint16_t>(m_netPortBuffer), 32);
        }
    } else {
        if (ImGui::Button("Stop Server")) {
            netMgr->StopServer();
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Listening on Port %u | Connected: %zu / 32",
                           netMgr->GetServerPort(), netMgr->GetServerConnectedCount());
    }

    // Active Connections Table
    if (isServerListening) {
        ImGui::Spacing();
        ImGui::Text("Server Active Connections (%zu):", netMgr->GetServerConnectedCount());
        const auto& conns = netMgr->GetServerConnections();
        if (conns.empty()) {
            ImGui::TextDisabled("  (No clients currently connected)");
        } else {
            if (ImGui::BeginTable("ServerConnTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Conn ID");
                ImGui::TableSetupColumn("Remote Address");
                ImGui::TableSetupColumn("State");
                ImGui::TableSetupColumn("RTT (ms)");
                ImGui::TableSetupColumn("Packets (S/R)");
                ImGui::TableSetupColumn("Bytes (S/R)");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();

                for (const auto& [cid, conn] : conns) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", cid);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s:%u", conn.GetAddress().c_str(), conn.GetPort());

                    ImGui::TableNextColumn();
                    auto stateStr = network::ToString(conn.GetState());
                    ImGui::Text("%.*s", static_cast<int>(stateStr.size()), stateStr.data());

                    ImGui::TableNextColumn();
                    const auto& m = conn.GetStats().GetMetrics();
                    ImGui::Text("%u ms (loss: %.1f%%)", m.roundTripTimeMs, m.packetLossPercent);

                    ImGui::TableNextColumn();
                    ImGui::Text("%llu / %llu", m.packetsSent, m.packetsReceived);

                    ImGui::TableNextColumn();
                    ImGui::Text("%llu / %llu", m.bytesSent, m.bytesReceived);

                    ImGui::TableNextColumn();
                    std::string kickId = "Disconnect##" + std::to_string(cid);
                    if (ImGui::Button(kickId.c_str())) {
                        netMgr->DisconnectServerClient(cid, 0);
                    }
                }
                ImGui::EndTable();
            }
        }
    }

    ImGui::Separator();

    // 3. Client Connection Controls & Diagnostics
    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Client Connection:");
    ImGui::InputText("Server Address", m_netAddressBuffer, sizeof(m_netAddressBuffer));

    auto clientStateStr = network::ToString(clientState);
    ImGui::Text("Client State: %.*s", static_cast<int>(clientStateStr.size()), clientStateStr.data());

    if (!isClientConnected && clientState != network::ConnectionState::Connecting) {
        if (ImGui::Button("Connect to Server")) {
            netMgr->ConnectClient(m_netAddressBuffer, static_cast<uint16_t>(m_netPortBuffer), 5000);
        }
    } else {
        if (ImGui::Button("Disconnect Client")) {
            netMgr->DisconnectClient();
        }
        ImGui::SameLine();
        ImGui::Text("RTT: %u ms | Target: %s:%u",
                    netMgr->GetClientRoundTripTimeMs(),
                    netMgr->GetClientServerAddress().c_str(),
                    netMgr->GetClientServerPort());
    }

    ImGui::Spacing();
    ImGui::InputText("Test Message", m_netTestMessageBuffer, sizeof(m_netTestMessageBuffer));
    if (ImGui::Button("Send Reliable Ping")) {
        netMgr->SendTestMessage(true, m_netTestMessageBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Send Unreliable Ping")) {
        netMgr->SendTestMessage(false, m_netTestMessageBuffer);
    }

    ImGui::Separator();

    // 4. Aggregate Network Statistics & Bandwidth Meter
    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Bandwidth & Packet Metrics:");
    const auto& sNetStats = netMgr->GetServerGlobalStats();
    const auto& cNetStats = netMgr->GetClientStats();
    const auto& sStats = sNetStats.GetMetrics();
    const auto& cStats = cNetStats.GetMetrics();

    ImGui::Columns(2, "NetMetricsColumns", true);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Server Aggregates:");
    ImGui::Text("Packets Sent:     %llu", sStats.packetsSent);
    ImGui::Text("Packets Recv:     %llu", sStats.packetsReceived);
    ImGui::Text("Bytes Sent:       %llu B (%.2f KB/s)", sStats.bytesSent, sNetStats.GetBytesSentPerSec() / 1024.0f);
    ImGui::Text("Bytes Recv:       %llu B (%.2f KB/s)", sStats.bytesReceived, sNetStats.GetBytesReceivedPerSec() / 1024.0f);
    ImGui::Text("Reliable Sent:    %llu", sStats.reliablePacketsSent);
    ImGui::Text("Validation Drops: %u", sStats.validationFailures);

    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Client Metrics:");
    ImGui::Text("Packets Sent:     %llu", cStats.packetsSent);
    ImGui::Text("Packets Recv:     %llu", cStats.packetsReceived);
    ImGui::Text("Bytes Sent:       %llu B (%.2f KB/s)", cStats.bytesSent, cNetStats.GetBytesSentPerSec() / 1024.0f);
    ImGui::Text("Bytes Recv:       %llu B (%.2f KB/s)", cStats.bytesReceived, cNetStats.GetBytesReceivedPerSec() / 1024.0f);
    ImGui::Text("Reliable Sent:    %llu", cStats.reliablePacketsSent);
    ImGui::Text("Validation Drops: %u", cStats.validationFailures);
    ImGui::Columns(1);

    // 5. Replication & Entity Synchronization (Phase 37)
    auto* replMgr = netMgr->GetReplicationManager();
    if (replMgr && ImGui::CollapsingHeader("Replication & Entity Synchronization (Phase 37)", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto rDiag = replMgr->GetDiagnostics();
        const auto& rCfg = replMgr->GetConfig();

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Replicated Entity Registry:");
        ImGui::Columns(4, "ReplicationEntities", true);
        ImGui::Text("Total Active: %zu", rDiag.totalReplicatedEntities);
        ImGui::NextColumn();
        ImGui::Text("Players: %zu", rDiag.playerCount);
        ImGui::NextColumn();
        ImGui::Text("Enemies: %zu", rDiag.enemyCount);
        ImGui::NextColumn();
        ImGui::Text("Loot / Env: %zu / %zu", rDiag.lootCount, rDiag.environmentalCount);
        ImGui::Columns(1);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Snapshot & Delta Metrics:");
        ImGui::Columns(3, "SnapshotMetrics", true);
        ImGui::Text("Snapshots Sent: %llu (Delta: %llu)", rDiag.snapshotsSent, rDiag.deltaSnapshotsSent);
        ImGui::NextColumn();
        ImGui::Text("Snapshots Recv: %llu (Delta: %llu)", rDiag.snapshotsReceived, rDiag.deltaSnapshotsReceived);
        ImGui::NextColumn();
        ImGui::Text("Buffer Depth: %zu / %u (Alpha: %.2f)", rDiag.currentSnapshotBufferSize, rCfg.maxSnapshotBuffer, rDiag.lastInterpolationAlpha);
        ImGui::Columns(1);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Synchronization Health & Transactions:");
        ImGui::Columns(3, "SyncHealth", true);
        ImGui::Text("Stale Snapshots Rejected: %llu", rDiag.staleSnapshotsRejected);
        ImGui::NextColumn();
        ImGui::Text("Duplicate Events Filtered: %llu", rDiag.duplicateEventsRejected);
        ImGui::NextColumn();
        ImGui::Text("Combat Events: %llu | Pickups: %llu (Succ: %llu, Rej: %llu)",
                    rDiag.combatEventsDispatched, rDiag.pickupTransactionsProcessed,
                    rDiag.pickupTransactionsSuccessful, rDiag.pickupTransactionsRejected);
        ImGui::Columns(1);
    }
}

void DevUI::RenderLuaDiagnostics(scripting::LuaManager* luaMgr, world::EnemySpawner* spawner) {
    if (!luaMgr) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Lua scripting manager is not available.");
        return;
    }

    bool isInit = luaMgr->IsInitialized();
    if (isInit) {
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "[ACTIVE] Lua 5.4 Runtime Initialized (sol2 safeties enabled)");
    } else {
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "[DISABLED] Lua Runtime Offline");
        return;
    }

    ImGui::Separator();
    ImGui::Columns(4, "LuaMetricsColumns", true);
    ImGui::Text("Memory Usage: %zu KB", luaMgr->GetMemoryUsageKb());
    ImGui::NextColumn();
    ImGui::Text("Executions: %llu", static_cast<unsigned long long>(luaMgr->GetTotalExecutionCount()));
    ImGui::NextColumn();
    ImGui::Text("Errors: %llu", static_cast<unsigned long long>(luaMgr->GetTotalErrorCount()));
    ImGui::NextColumn();
    ImGui::Text("Last Duration: %.2f us", luaMgr->GetLastExecutionTimeUs());
    ImGui::Columns(1);

    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Instruction Budget & Infinite Loop Protection", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool budgetEnabled = luaMgr->IsInstructionBudgetEnabled();
        if (ImGui::Checkbox("Enable Instruction Budget", &budgetEnabled)) {
            luaMgr->EnableInstructionBudget(budgetEnabled);
        }
        int budget = luaMgr->GetInstructionBudget();
        if (ImGui::SliderInt("Budget (Instructions)", &budget, 1000, 1000000)) {
            luaMgr->SetInstructionBudget(budget);
        }
        ImGui::TextColored(
            luaMgr->GetBudgetViolationCount() > 0 ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.5f, 0.8f, 0.5f, 1.0f),
            "Budget Violations: %llu", static_cast<unsigned long long>(luaMgr->GetBudgetViolationCount())
        );
    }

    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Enemy AI Script Telemetry", ImGuiTreeNodeFlags_DefaultOpen)) {
        size_t activeEnemies = spawner ? spawner->GetActiveEnemyCount() : 0;
        ImGui::Text("Active Enemies in World: %zu", activeEnemies);
        ImGui::Text("Authoritative Lua AI Decisions: %llu", static_cast<unsigned long long>(entities::Enemy::GetLuaDecisionCount()));
        ImGui::Text("C++ Fallback AI Executions: %llu", static_cast<unsigned long long>(entities::Enemy::GetFallbackAICount()));

        if (ImGui::Button("Reset AI Telemetry Counters")) {
            entities::Enemy::ResetDiagnostics();
        }
    }

    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Loaded Scripts & Hot Reload", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Reload All Scripts")) {
            luaMgr->ReloadAllScripts();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(Hot reload re-executes Lua files without restarting engine)");

        const auto& scripts = luaMgr->GetLoadedScripts();
        ImGui::Text("Tracked Scripts Count: %zu", scripts.size());

        if (ImGui::BeginTable("ScriptsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Script Path");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Executions");
            ImGui::TableSetupColumn("Errors");
            ImGui::TableSetupColumn("Last Time");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (const auto& [path, meta] : scripts) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", path.c_str());

                ImGui::TableSetColumnIndex(1);
                if (meta.isLoaded) {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Loaded");
                } else {
                    ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Error");
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%llu", static_cast<unsigned long long>(meta.executionCount));

                ImGui::TableSetColumnIndex(3);
                if (meta.errorCount > 0) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%llu", static_cast<unsigned long long>(meta.errorCount));
                } else {
                    ImGui::Text("0");
                }

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.1f us", meta.lastExecutionTimeUs);

                ImGui::TableSetColumnIndex(5);
                std::string btnId = "Reload##" + path;
                if (ImGui::SmallButton(btnId.c_str())) {
                    luaMgr->ReloadScript(path);
                }
            }
            ImGui::EndTable();
        }
    }
}

} // namespace wildbound::ui
