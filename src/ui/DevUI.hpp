#pragma once

#include "../core/EngineConfig.hpp"
#include "../graphics/Camera2D.hpp"
#include "../graphics/Viewport.hpp"
#include <string>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::world {
    class World;
    class WorldRenderer;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::combat {
    class CombatSystem;
}

namespace wildbound::world {
    class EnemySpawner;
    class BuildingManager;
    class BiomeManager;
    class WorldManager;
}

namespace wildbound::world::environment {
    class EnvironmentalAnimationSystem;
}

namespace wildbound::weapons {
    class WeaponManager;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::physics {
    class CollisionManager;
}

namespace wildbound::input {
    class InputManager;
}

namespace wildbound::interaction {
    class InteractionManager;
}

namespace wildbound::customization {
    class CharacterCustomizationManager;
}

namespace wildbound::cosmetics {
    class CosmeticsStoreManager;
}

namespace wildbound::ai {
    class AIManager;
}

namespace wildbound::storm {
    class StormManager;
}

namespace wildbound::ui {
    class UIManager;
}

namespace wildbound::ai {
    class AIManager;
}

namespace wildbound::storm {
    class StormManager;
}

namespace wildbound::ui {

class DevUI {
public:
    DevUI();
    ~DevUI();

    DevUI(const DevUI&) = delete;
    DevUI& operator=(const DevUI&) = delete;

    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderDebugOverlay(
        const core::EngineConfig& config,
        float fps,
        float dt,
        const graphics::Camera2DSystem& camera,
        const graphics::Viewport& viewport,
        Vector2 mouseScreen,
        Vector2 mouseVirtual,
        Vector2 mouseWorld,
        resources::AssetManager* assetMgr = nullptr,
        world::World* world = nullptr,
        world::WorldRenderer* worldRenderer = nullptr,
        entities::Player* player = nullptr,
        combat::CombatSystem* combatSys = nullptr,
        world::EnemySpawner* spawner = nullptr,
        world::environment::EnvironmentalAnimationSystem* envAnimSys = nullptr,
        world::BuildingManager* buildingMgr = nullptr,
        world::BiomeManager* biomeMgr = nullptr,
        weapons::WeaponManager* weaponMgr = nullptr,
        items::ItemManager* itemMgr = nullptr,
        audio::AudioSystem* audioSys = nullptr,
        physics::CollisionManager* colMgr = nullptr,
        input::InputManager* inputMgr = nullptr,
        interaction::InteractionManager* interactionMgr = nullptr,
        customization::CharacterCustomizationManager* customMgr = nullptr,
        cosmetics::CosmeticsStoreManager* storeMgr = nullptr,
        ui::UIManager* uiMgr = nullptr,
        ai::AIManager* aiMgr = nullptr,
        world::WorldManager* worldMgr = nullptr,
        storm::StormManager* stormMgr = nullptr
    );

    void ToggleVisible() { m_visible = !m_visible; }
    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }

    void ToggleAnimDebug() { m_showAnimDebug = !m_showAnimDebug; }
    bool IsAnimDebug() const { return m_showAnimDebug; }

    void ToggleInteractionDebug() { m_showInteractionDebug = !m_showInteractionDebug; }
    bool IsInteractionDebug() const { return m_showInteractionDebug; }

private:
    void RenderWorldAndMap(world::World* world, world::WorldRenderer* worldRenderer, Vector2 mouseWorld, entities::Player* player = nullptr, world::BuildingManager* buildingMgr = nullptr, const graphics::Camera2DSystem* camera = nullptr, resources::AssetManager* assetMgr = nullptr, world::BiomeManager* biomeMgr = nullptr, world::environment::EnvironmentalAnimationSystem* envAnim = nullptr, world::WorldManager* worldMgr = nullptr);
    void RenderPlayerDebug(entities::Player* player);
    void RenderCombatAndAI(combat::CombatSystem* combatSys, world::EnemySpawner* spawner, entities::Player* player, ai::AIManager* aiMgr = nullptr);
    void RenderAssetBrowser(resources::AssetManager* assetMgr);
    void RenderAssetValidation(resources::AssetManager* assetMgr);
    void RenderAnimationHUD(entities::Player* player);
    void RenderWardrobe(entities::Player* player, resources::AssetManager* assetMgr, customization::CharacterCustomizationManager* customMgr = nullptr);
    void RenderCosmeticsStore(cosmetics::CosmeticsStoreManager* storeMgr, customization::CharacterCustomizationManager* customMgr = nullptr);
    void RenderCreaturePreview(resources::AssetManager* assetMgr);
    void RenderEnvironmentPreview(resources::AssetManager* assetMgr);
    void RenderLivingWorld(world::environment::EnvironmentalAnimationSystem* envAnimSys, resources::AssetManager* assetMgr, Vector2 playerPos);
    void RenderWeaponPreview(weapons::WeaponManager* weaponMgr, resources::AssetManager* assetMgr, entities::Player* player = nullptr);
    void RenderItemPreview(items::ItemManager* itemMgr, resources::AssetManager* assetMgr);
    void RenderAudioDebug(audio::AudioSystem* audioSys);
    void RenderPhysicsAndCollision(physics::CollisionManager* colMgr);
    void RenderInputDebug(input::InputManager* inputMgr);
    void RenderInteractionDebug(interaction::InteractionManager* interactionMgr, entities::Player* player = nullptr);
    void RenderLootAndInventory(entities::Player* player, world::World* world, items::ItemManager* itemMgr, audio::AudioSystem* audioSys);
    void RenderUIAndHUD(ui::UIManager* uiMgr, entities::Player* player);
    void RenderStormDebug(storm::StormManager* stormMgr, entities::Player* player);

    bool m_initialized = false;
    bool m_visible = false;
    bool m_showAnimDebug = false;
    bool m_showInteractionDebug = false;

    // UI & HUD Debug state
    char m_notificationTextBuffer[128]{ "Test notification alert!" };

    // Asset Browser state
    char m_searchBuffer[64]{ 0 };
    std::string m_selectedCategory = "All";
    std::string m_selectedAssetId;

    // Item Preview state (Phase 16)
    std::string m_selectedItemId = "col_gold_coin_001";
    int m_selectedItemCategoryFilter = 0; // 0=All, 1=Collectibles, 2=Consumables, 3=Materials, 4=Resources, 5=Special
    char m_itemSearchBuffer[64]{ 0 };

    // Weapon Preview state (Phase 15)
    std::string m_selectedWeaponId = "sword_iron_001";
    int m_weaponFacing = 0;
    int m_weaponAnimFrame = 0;
    bool m_showGripCrosshair = true;

    // Creature Preview state (Phase 9)
    int m_selectedCreatureIdx = 0;
    int m_creatureFacing = 0;
    int m_creatureAnimIdx = 0;
    int m_previewFrameIndex = 0;

    // Environment Preview state (Phase 10)
    int m_selectedEnvCategory = 0; // 0=All, 1=Nature, 2=Buildings, 3=Props
    int m_selectedEnvAssetIdx = 0;

    // Living World state (Phase 11)
    int m_selectedAmbientSpeciesIdx = 0;

    // Audio System state (Phase 18)
    int m_selectedMusicIndex = 0;
    int m_selectedSfxIndex = 0;
    float m_previewSfxVolume = 1.0f;
    float m_previewSfxPitch = 1.0f;
};

} // namespace wildbound::ui
