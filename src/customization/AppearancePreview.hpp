#pragma once

#include "CharacterAppearance.hpp"
#include "CosmeticDefinition.hpp"
#include "../entities/ModularCharacter.hpp"
#include "../graphics/animation/AnimationController.hpp"
#include "../graphics/animation/Direction.hpp"

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::customization {

class CosmeticRegistry;

/**
 * Isolated Live Character Preview Subsystem.
 * Manages temporary appearance customization changes without modifying authoritative gameplay state.
 */
class AppearancePreview {
public:
    AppearancePreview();
    ~AppearancePreview() = default;

    // Preview Lifecycle
    void BeginPreview(const CharacterAppearance& currentAppearance, const CosmeticRegistry& registry);
    bool ApplyPreviewCosmetic(const CosmeticDefinition& cosmetic, const CosmeticRegistry& registry);
    bool UnequipPreviewSlot(CustomizationSlot slot, const CosmeticRegistry& registry);
    void ResetPreview(const CosmeticRegistry& registry);
    bool ConfirmPreview(CharacterAppearance& outAuthoritativeAppearance);
    void CancelPreview();

    [[nodiscard]] bool IsActive() const noexcept { return m_isActive; }
    [[nodiscard]] const CharacterAppearance& GetPreviewAppearance() const noexcept { return m_previewAppearance; }
    [[nodiscard]] const CharacterAppearance& GetOriginalAppearance() const noexcept { return m_originalAppearance; }
    [[nodiscard]] bool HasUnsavedChanges() const noexcept { return !(m_previewAppearance == m_originalAppearance); }

    // Directional & Animation Controls
    void RotateClockwise();
    void RotateCounterClockwise();
    void SetDirection(graphics::animation::Direction dir) { m_previewDirection = dir; }
    [[nodiscard]] graphics::animation::Direction GetDirection() const noexcept { return m_previewDirection; }

    void ToggleAnimationMode();
    void SetWalkingAnimation(bool walking);
    [[nodiscard]] bool IsWalkingAnimation() const noexcept { return m_isWalking; }

    // Frame Update & Rendering
    void Update(float dt);
    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr, Vector2 position);

    // Modular preview character accessor
    [[nodiscard]] const entities::ModularCharacter& GetModularCharacter() const noexcept { return m_previewModularChar; }

private:
    void RefreshPreviewCharacter(const CosmeticRegistry& registry);

    bool m_isActive = false;
    bool m_isWalking = false;
    graphics::animation::Direction m_previewDirection = graphics::animation::Direction::Down;

    CharacterAppearance m_originalAppearance;
    CharacterAppearance m_previewAppearance;

    entities::ModularCharacter m_previewModularChar;
    graphics::animation::AnimationController m_animController;
};

} // namespace wildbound::customization

