#include "PauseMenu.hpp"

namespace wildbound::ui {

PauseMenu::PauseMenu()
    : m_focusedIndex(0) {
}

bool PauseMenu::CheckHover(Rectangle r) const {
    return (m_mousePos.x >= r.x && m_mousePos.x <= r.x + r.width &&
            m_mousePos.y >= r.y && m_mousePos.y <= r.y + r.height);
}

PauseMenuAction PauseMenu::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed,
                                 bool navUp, bool navDown, bool navConfirm) {
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;

    if (navUp) {
        m_focusedIndex = (m_focusedIndex - 1 + ItemCount) % ItemCount;
    }
    if (navDown) {
        m_focusedIndex = (m_focusedIndex + 1) % ItemCount;
    }

    const float btnW = 160.0f;
    const float btnH = 26.0f;
    const float startX = (UITheme::CanvasWidth - btnW) * 0.5f;
    const float startY = 100.0f;
    const float spacing = 34.0f;

    for (int i = 0; i < ItemCount; ++i) {
        Rectangle btnRect{ startX, startY + i * spacing, btnW, btnH };
        if (CheckHover(btnRect)) {
            m_focusedIndex = i;
            if (m_mousePressed) {
                switch (i) {
                    case 0: return PauseMenuAction::Resume;
                    case 1: return PauseMenuAction::Inventory;
                    case 2: return PauseMenuAction::Wardrobe;
                    case 3: return PauseMenuAction::Settings;
                    case 4: return PauseMenuAction::MainMenu;
                    case 5: return PauseMenuAction::Quit;
                }
            }
        }
    }

    if (navConfirm) {
        switch (m_focusedIndex) {
            case 0: return PauseMenuAction::Resume;
            case 1: return PauseMenuAction::Inventory;
            case 2: return PauseMenuAction::Wardrobe;
            case 3: return PauseMenuAction::Settings;
            case 4: return PauseMenuAction::MainMenu;
            case 5: return PauseMenuAction::Quit;
        }
    }

    return PauseMenuAction::None;
}

void PauseMenu::Render() {
    // Dimmed background overlay
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 10, 12, 16, 210 });

    // Center Panel
    const float panelW = 220.0f;
    const float panelH = 260.0f;
    const float panelX = (UITheme::CanvasWidth - panelW) * 0.5f;
    const float panelY = (UITheme::CanvasHeight - panelH) * 0.5f;

    Rectangle panelRect{ panelX, panelY, panelW, panelH };
    DrawRectangleRec(panelRect, UITheme::PanelBg);
    DrawRectangleLinesEx(panelRect, 1.0f, UITheme::BorderGoldDim);

    // Title
    const char* title = "GAME PAUSED";
    int tw = MeasureText(title, UITheme::FontMedium);
    DrawText(title, static_cast<int>(panelX + (panelW - tw) * 0.5f), static_cast<int>(panelY + 16.0f), UITheme::FontMedium, UITheme::Gold);

    // Divider
    DrawLine(static_cast<int>(panelX + 20.0f), static_cast<int>(panelY + 36.0f),
             static_cast<int>(panelX + panelW - 20.0f), static_cast<int>(panelY + 36.0f), UITheme::BorderNormal);

    // Menu Buttons
    static const char* labels[] = {
        "RESUME",
        "INVENTORY",
        "WARDROBE",
        "SETTINGS",
        "MAIN MENU",
        "QUIT GAME"
    };

    const float btnW = 160.0f;
    const float btnH = 24.0f;
    const float startX = (UITheme::CanvasWidth - btnW) * 0.5f;
    const float startY = panelY + 48.0f;
    const float spacing = 32.0f;

    for (int i = 0; i < ItemCount; ++i) {
        Rectangle r{ startX, startY + i * spacing, btnW, btnH };
        bool isFocused = (m_focusedIndex == i);

        DrawRectangleRec(r, isFocused ? UITheme::SlotSelected : UITheme::SlotEmpty);
        DrawRectangleLinesEx(r, 1.0f, isFocused ? UITheme::BorderGold : UITheme::BorderNormal);

        // Selection Pip
        if (isFocused) {
            DrawRectangle(static_cast<int>(r.x + 4), static_cast<int>(r.y + (r.height - 8) * 0.5f), 4, 8, UITheme::Gold);
        }

        int lw = MeasureText(labels[i], UITheme::FontSmall);
        DrawText(labels[i], static_cast<int>(r.x + (r.width - lw) * 0.5f), static_cast<int>(r.y + 6.0f),
                 UITheme::FontSmall, isFocused ? UITheme::Gold : UITheme::TextLight);
    }
}

} // namespace wildbound::ui

