#include "MainMenu.hpp"

namespace wildbound::ui {

MainMenu::MainMenu()
    : m_focusedIndex(0)
    , m_showCredits(false) {
}

bool MainMenu::CheckHover(Rectangle r) const {
    return (m_mousePos.x >= r.x && m_mousePos.x <= r.x + r.width &&
            m_mousePos.y >= r.y && m_mousePos.y <= r.y + r.height);
}

MainMenuAction MainMenu::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed,
                               bool navUp, bool navDown, bool navConfirm) {
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;

    if (m_showCredits) {
        if (m_mousePressed || navConfirm) {
            m_showCredits = false;
        }
        return MainMenuAction::None;
    }

    if (navUp) {
        m_focusedIndex = (m_focusedIndex - 1 + ItemCount) % ItemCount;
    }
    if (navDown) {
        m_focusedIndex = (m_focusedIndex + 1) % ItemCount;
    }

    const float btnW = 160.0f;
    const float btnH = 22.0f;
    const float startX = 60.0f;
    const float startY = 130.0f;
    const float spacing = 28.0f;

    for (int i = 0; i < ItemCount; ++i) {
        Rectangle btnRect{ startX, startY + i * spacing, btnW, btnH };
        if (CheckHover(btnRect)) {
            m_focusedIndex = i;
            if (m_mousePressed) {
                switch (i) {
                    case 0: return MainMenuAction::PlayGame;
                    case 1: return MainMenuAction::CharacterSelect;
                    case 2: return MainMenuAction::Wardrobe;
                    case 3: return MainMenuAction::CosmeticsStore;
                    case 4: return MainMenuAction::Settings;
                    case 5: m_showCredits = true; return MainMenuAction::None;
                    case 6: return MainMenuAction::ExitGame;
                }
            }
        }
    }

    if (navConfirm) {
        switch (m_focusedIndex) {
            case 0: return MainMenuAction::PlayGame;
            case 1: return MainMenuAction::CharacterSelect;
            case 2: return MainMenuAction::Wardrobe;
            case 3: return MainMenuAction::CosmeticsStore;
            case 4: return MainMenuAction::Settings;
            case 5: m_showCredits = true; return MainMenuAction::None;
            case 6: return MainMenuAction::ExitGame;
        }
    }

    return MainMenuAction::None;
}

void MainMenu::Render() {
    // Backdrop gradient / dark wash
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 14, 18, 26, 255 });

    // Decorative background pattern
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), 6, UITheme::BorderGoldDim);
    DrawRectangle(0, static_cast<int>(UITheme::CanvasHeight - 6), static_cast<int>(UITheme::CanvasWidth), 6, UITheme::BorderGoldDim);

    // Game Logo & Subtitle
    DrawText("WILDBOUND", 60, 48, UITheme::FontHuge, UITheme::Gold);
    DrawText("2D FANTASY BATTLE ROYALE", 62, 78, UITheme::FontSmall, UITheme::TextMuted);

    // Decorative line under title
    DrawLine(60, 96, 260, 96, UITheme::BorderGold);

    // Menu options
    static const char* labels[] = {
        "PLAY MATCH",
        "CHARACTER SELECT",
        "WARDROBE",
        "COSMETICS STORE",
        "SETTINGS",
        "CREDITS",
        "EXIT GAME"
    };

    const float btnW = 160.0f;
    const float btnH = 22.0f;
    const float startX = 60.0f;
    const float startY = 130.0f;
    const float spacing = 28.0f;

    for (int i = 0; i < ItemCount; ++i) {
        Rectangle r{ startX, startY + i * spacing, btnW, btnH };
        bool isFocused = (m_focusedIndex == i);

        DrawRectangleRec(r, isFocused ? UITheme::SlotSelected : UITheme::SlotEmpty);
        DrawRectangleLinesEx(r, 1.0f, isFocused ? UITheme::BorderGold : UITheme::BorderNormal);

        if (isFocused) {
            DrawRectangle(static_cast<int>(r.x + 3), static_cast<int>(r.y + 4), 3, static_cast<int>(r.height - 8), UITheme::Gold);
        }

        DrawText(labels[i], static_cast<int>(r.x + 16), static_cast<int>(r.y + 5),
                 UITheme::FontSmall, isFocused ? UITheme::Gold : UITheme::TextLight);
    }

    // Version & build badge
    DrawText("WILDBOUND v0.27.0-DEV | RAYLIB C++20", static_cast<int>(UITheme::CanvasWidth - 210),
             static_cast<int>(UITheme::CanvasHeight - 20), UITheme::FontMicro, UITheme::TextDim);

    // Credits modal dialog if opened
    if (m_showCredits) {
        DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 5, 8, 12, 230 });
        Rectangle credRect{ 160.0f, 60.0f, 320.0f, 240.0f };
        DrawRectangleRec(credRect, UITheme::PanelBg);
        DrawRectangleLinesEx(credRect, 1.0f, UITheme::BorderGold);

        DrawText("CREDITS", static_cast<int>(credRect.x + 120), static_cast<int>(credRect.y + 16), UITheme::FontMedium, UITheme::Gold);
        DrawLine(static_cast<int>(credRect.x + 20), static_cast<int>(credRect.y + 38), static_cast<int>(credRect.x + credRect.width - 20), static_cast<int>(credRect.y + 38), UITheme::BorderNormal);

        DrawText("Wildbound Systems Architecture:", static_cast<int>(credRect.x + 24), static_cast<int>(credRect.y + 50), UITheme::FontSmall, UITheme::Gold);
        DrawText("Core Gameplay Engineering Team", static_cast<int>(credRect.x + 24), static_cast<int>(credRect.y + 68), UITheme::FontSmall, UITheme::TextLight);

        DrawText("Built using:", static_cast<int>(credRect.x + 24), static_cast<int>(credRect.y + 100), UITheme::FontSmall, UITheme::Gold);
        DrawText("- C++20 & MSVC", static_cast<int>(credRect.x + 32), static_cast<int>(credRect.y + 118), UITheme::FontSmall, UITheme::TextLight);
        DrawText("- raylib 5.5 Graphics & Audio", static_cast<int>(credRect.x + 32), static_cast<int>(credRect.y + 136), UITheme::FontSmall, UITheme::TextLight);
        DrawText("- Lua 5.4 & sol2 Scripting", static_cast<int>(credRect.x + 32), static_cast<int>(credRect.y + 154), UITheme::FontSmall, UITheme::TextLight);
        DrawText("- Dear ImGui Developer Tooling", static_cast<int>(credRect.x + 32), static_cast<int>(credRect.y + 172), UITheme::FontSmall, UITheme::TextLight);

        DrawText("Click anywhere or press Enter to return", static_cast<int>(credRect.x + 40), static_cast<int>(credRect.y + 210), UITheme::FontMicro, UITheme::TextMuted);
    }
}

} // namespace wildbound::ui

