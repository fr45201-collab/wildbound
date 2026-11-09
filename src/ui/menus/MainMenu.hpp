#pragma once

#include "../UITheme.hpp"
#include <raylib.h>
#include <string>
#include <vector>

namespace wildbound::ui {

enum class MainMenuAction {
    None = 0,
    PlayGame,
    CharacterSelect,
    Wardrobe,
    CosmeticsStore,
    Account,
    Multiplayer,
    Settings,
    Credits,
    ExitGame
};

/**
 * Main Menu Screen Subsystem.
 * Displays title banner, fantasy backdrop, versioning metadata, and handles
 * directional keyboard/gamepad focus navigation and mouse interaction.
 */
class MainMenu {
public:
    MainMenu();
    ~MainMenu() = default;

    MainMenuAction Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed,
                          bool navUp = false, bool navDown = false, bool navConfirm = false);

    void Render();

    void SetFocusedIndex(int index) { m_focusedIndex = index; }
    [[nodiscard]] int GetFocusedIndex() const { return m_focusedIndex; }

    [[nodiscard]] bool IsShowingCredits() const { return m_showCredits; }
    void SetShowingCredits(bool show) { m_showCredits = show; }

private:
    bool CheckHover(Rectangle r) const;

    int m_focusedIndex = 0;
    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed = false;
    bool m_showCredits = false;

    static constexpr int ItemCount = 9;
};

} // namespace wildbound::ui

