#pragma once

#include "../UITheme.hpp"
#include <raylib.h>
#include <vector>
#include <string>

namespace wildbound::ui {

enum class PauseMenuAction {
    None = 0,
    Resume,
    Inventory,
    Wardrobe,
    Settings,
    MainMenu,
    Quit
};

/**
 * In-game Modal Pause Menu.
 * Pauses game simulation, preserves input contexts, and routes player to
 * Resume, Inventory, Wardrobe, Settings, Main Menu, or Quit.
 * Supports mouse hover/click and keyboard/gamepad navigation.
 */
class PauseMenu {
public:
    PauseMenu();
    ~PauseMenu() = default;

    PauseMenuAction Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed,
                           bool navUp = false, bool navDown = false, bool navConfirm = false);

    void Render();

    void SetFocusedIndex(int index) { m_focusedIndex = index; }
    [[nodiscard]] int GetFocusedIndex() const { return m_focusedIndex; }

private:
    bool CheckHover(Rectangle r) const;

    int m_focusedIndex = 0;
    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed = false;

    static constexpr int ItemCount = 6;
};

} // namespace wildbound::ui

