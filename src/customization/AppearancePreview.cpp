#include "AppearancePreview.hpp"
#include "CosmeticRegistry.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"

namespace wildbound::customization {

AppearancePreview::AppearancePreview() {
    m_previewAppearance.ResetToDefault();
    m_originalAppearance.ResetToDefault();
}

void AppearancePreview::BeginPreview(const CharacterAppearance& currentAppearance, const CosmeticRegistry& registry) {
    m_originalAppearance = currentAppearance;
    m_previewAppearance = currentAppearance;
    m_isActive = true;
    m_previewDirection = graphics::animation::Direction::Down;
    m_isWalking = false;

    RefreshPreviewCharacter(registry);
}

bool AppearancePreview::ApplyPreviewCosmetic(const CosmeticDefinition& cosmetic, const CosmeticRegistry& registry) {
    if (!m_isActive) return false;

    m_previewAppearance.SetCosmetic(cosmetic.slot, cosmetic.id);
    RefreshPreviewCharacter(registry);
    return true;
}

bool AppearancePreview::UnequipPreviewSlot(CustomizationSlot slot, const CosmeticRegistry& registry) {
    if (!m_isActive) return false;

    m_previewAppearance.ClearSlot(slot);
    RefreshPreviewCharacter(registry);
    return true;
}

void AppearancePreview::ResetPreview(const CosmeticRegistry& registry) {
    if (!m_isActive) return;

    m_previewAppearance = m_originalAppearance;
    RefreshPreviewCharacter(registry);
}

bool AppearancePreview::ConfirmPreview(CharacterAppearance& outAuthoritativeAppearance) {
    if (!m_isActive) return false;

    outAuthoritativeAppearance = m_previewAppearance;
    m_originalAppearance = m_previewAppearance;
    m_isActive = false;
    return true;
}

void AppearancePreview::CancelPreview() {
    if (!m_isActive) return;

    m_previewAppearance = m_originalAppearance;
    m_isActive = false;
}

void AppearancePreview::RotateClockwise() {
    switch (m_previewDirection) {
        case graphics::animation::Direction::Down:  m_previewDirection = graphics::animation::Direction::Left; break;
        case graphics::animation::Direction::Left:  m_previewDirection = graphics::animation::Direction::Up; break;
        case graphics::animation::Direction::Up:    m_previewDirection = graphics::animation::Direction::Right; break;
        case graphics::animation::Direction::Right: m_previewDirection = graphics::animation::Direction::Down; break;
    }
}

void AppearancePreview::RotateCounterClockwise() {
    switch (m_previewDirection) {
        case graphics::animation::Direction::Down:  m_previewDirection = graphics::animation::Direction::Right; break;
        case graphics::animation::Direction::Right: m_previewDirection = graphics::animation::Direction::Up; break;
        case graphics::animation::Direction::Up:    m_previewDirection = graphics::animation::Direction::Left; break;
        case graphics::animation::Direction::Left:  m_previewDirection = graphics::animation::Direction::Down; break;
    }
}

void AppearancePreview::ToggleAnimationMode() {
    SetWalkingAnimation(!m_isWalking);
}

void AppearancePreview::SetWalkingAnimation(bool walking) {
    m_isWalking = walking;
}

void AppearancePreview::Update(float dt) {
    (void)dt;
    if (!m_isActive) return;

    std::string prefix = m_isWalking ? "player_walk_" : "player_idle_";
    std::string clipName = prefix + std::string(graphics::animation::DirectionToString(m_previewDirection));

    if (m_animController.GetCurrentClipName() != clipName) {
        m_animController.Play(clipName, false, 1.0f);
    }
}

void AppearancePreview::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr, Vector2 position) {
    m_animController.Update(GetFrameTime(), assetMgr);
    m_previewModularChar.Render(renderer, assetMgr, position, m_previewDirection, m_animController);
}

void AppearancePreview::RefreshPreviewCharacter(const CosmeticRegistry& registry) {
    m_previewAppearance.ApplyToModularCharacter(m_previewModularChar, registry);
}

} // namespace wildbound::customization

