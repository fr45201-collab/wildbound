#pragma once

#include <raylib.h>
#include <string>
#include <string_view>

namespace wildbound::ui {

/**
 * Global UI Design Tokens, Color Palette, and Layout Metrics for Wildbound.
 * Virtual Target Canvas: 640x360 (Scaled with point filtering to 1280x720 / 1920x1080).
 */
struct UITheme {
    // Virtual Canvas Dimensions
    static constexpr float CanvasWidth = 640.0f;
    static constexpr float CanvasHeight = 360.0f;

    // Backgrounds & Surface Colors
    static inline const Color DarkBg        = Color{ 12, 15, 20, 245 };
    static inline const Color DarkBgOpaque  = Color{ 12, 15, 20, 255 };
    static inline const Color PanelBg       = Color{ 20, 24, 32, 255 };
    static inline const Color PanelBgTrans  = Color{ 20, 24, 32, 220 };
    static inline const Color PanelHeader   = Color{ 28, 35, 48, 255 };
    static inline const Color SubPanelBg    = Color{ 16, 20, 28, 255 };

    // Borders & Bevels
    static inline const Color BorderDark    = Color{ 10, 12, 16, 255 };
    static inline const Color BorderNormal  = Color{ 45, 55, 72, 255 };
    static inline const Color BorderLight   = Color{ 70, 85, 110, 255 };
    static inline const Color BorderGold    = Color{ 210, 170, 50, 255 };
    static inline const Color BorderGoldDim = Color{ 130, 100, 30, 255 };

    // Accents & Brand Colors
    static inline const Color Gold          = Color{ 235, 185, 45, 255 };
    static inline const Color GoldHover     = Color{ 255, 215, 75, 255 };
    static inline const Color GoldPressed   = Color{ 180, 140, 30, 255 };
    static inline const Color GoldMuted     = Color{ 140, 110, 35, 255 };

    // Health Bar Palette
    static inline const Color HealthFill    = Color{ 220, 45, 45, 255 };
    static inline const Color HealthLag     = Color{ 245, 150, 45, 255 };
    static inline const Color HealthBg      = Color{ 45, 12, 12, 255 };
    static inline const Color HealthPulse   = Color{ 255, 70, 70, 255 };

    // Armor Palette
    static inline const Color ArmorFill     = Color{ 55, 135, 235, 255 };
    static inline const Color ArmorBg       = Color{ 12, 25, 48, 255 };
    static inline const Color ArmorHigh     = Color{ 110, 180, 255, 255 };

    // Stamina Palette
    static inline const Color StaminaFill   = Color{ 230, 180, 35, 255 };
    static inline const Color StaminaBg     = Color{ 40, 32, 10, 255 };

    // Slot & Button States
    static inline const Color SlotEmpty     = Color{ 24, 28, 38, 255 };
    static inline const Color SlotFilled    = Color{ 32, 38, 52, 255 };
    static inline const Color SlotHovered   = Color{ 48, 58, 80, 255 };
    static inline const Color SlotSelected  = Color{ 65, 80, 112, 255 };
    static inline const Color SlotActiveBorder = Color{ 245, 200, 60, 255 };

    // Typography & Content
    static inline const Color TextLight     = Color{ 240, 244, 250, 255 };
    static inline const Color TextMuted     = Color{ 145, 155, 172, 255 };
    static inline const Color TextDim       = Color{ 90, 100, 115, 255 };
    static inline const Color TextGold      = Color{ 245, 205, 70, 255 };
    static inline const Color TextDanger    = Color{ 255, 75, 75, 255 };
    static inline const Color TextSuccess   = Color{ 75, 225, 120, 255 };
    static inline const Color TextWarning   = Color{ 255, 185, 45, 255 };
    static inline const Color TextInfo      = Color{ 80, 170, 250, 255 };

    // Rarity Colors
    static inline const Color RarityCommon    = Color{ 175, 185, 195, 255 }; // Light Grey
    static inline const Color RarityUncommon  = Color{ 60, 200, 95, 255 };   // Green
    static inline const Color RarityRare      = Color{ 45, 140, 245, 255 };  // Blue
    static inline const Color RarityEpic      = Color{ 175, 65, 240, 255 };  // Purple
    static inline const Color RarityLegendary = Color{ 255, 150, 20, 255 };  // Orange / Gold

    // Font Size Steps (Virtual Canvas)
    static constexpr int FontMicro  = 8;
    static constexpr int FontSmall  = 10;
    static constexpr int FontRegular= 12;
    static constexpr int FontMedium = 14;
    static constexpr int FontLarge  = 16;
    static constexpr int FontTitle  = 20;
    static constexpr int FontHuge   = 24;

    // Helper: Resolve Rarity Color
    [[nodiscard]] static inline Color GetRarityColor(std::string_view rarity) {
        if (rarity == "uncommon" || rarity == "Uncommon") return RarityUncommon;
        if (rarity == "rare" || rarity == "Rare") return RarityRare;
        if (rarity == "epic" || rarity == "Epic") return RarityEpic;
        if (rarity == "legendary" || rarity == "Legendary") return RarityLegendary;
        return RarityCommon;
    }

    // Helper: Draw beveled rectangle
    static inline void DrawBeveledRect(Rectangle rect, Color fill, Color borderLight, Color borderDark, int borderWidth = 1) {
        DrawRectangleRec(rect, fill);
        // Top and Left light borders
        DrawRectangle(static_cast<int>(rect.x), static_cast<int>(rect.y), static_cast<int>(rect.width), borderWidth, borderLight);
        DrawRectangle(static_cast<int>(rect.x), static_cast<int>(rect.y), borderWidth, static_cast<int>(rect.height), borderLight);
        // Bottom and Right dark borders
        DrawRectangle(static_cast<int>(rect.x), static_cast<int>(rect.y + rect.height - borderWidth), static_cast<int>(rect.width), borderWidth, borderDark);
        DrawRectangle(static_cast<int>(rect.x + rect.width - borderWidth), static_cast<int>(rect.y), borderWidth, static_cast<int>(rect.height), borderDark);
    }

    // Helper: Draw pill tag
    static inline void DrawPill(Rectangle rect, const std::string& text, Color bg, Color textCol, int fontSize = FontSmall) {
        DrawRectangleRounded(rect, 0.4f, 4, bg);
        int tw = MeasureText(text.c_str(), fontSize);
        DrawText(text.c_str(),
                 static_cast<int>(rect.x + (rect.width - tw) * 0.5f),
                 static_cast<int>(rect.y + (rect.height - fontSize) * 0.5f),
                 fontSize, textCol);
    }
};

} // namespace wildbound::ui

