#pragma once

#include <raylib.h>

namespace wildbound::graphics {

/**
 * Curated fantasy palette constants for Wildbound Phase 1.
 * Inspired by authentic 16-bit / 32-bit pixel art fantasy aesthetics.
 */
namespace Palette {
    // Deep Night / Backgrounds
    constexpr Color VoidBlack       = { 10, 12, 16, 255 };
    constexpr Color MidnightSky     = { 18, 22, 34, 255 };
    constexpr Color DarkSlate       = { 28, 36, 52, 255 };
    constexpr Color SlateGrid       = { 42, 54, 76, 255 };

    // Forest & Nature
    constexpr Color DeepMoss        = { 24, 48, 36, 255 };
    constexpr Color ForestGrass     = { 46, 88, 54, 255 };
    constexpr Color LushMeadow      = { 78, 142, 74, 255 };
    constexpr Color CanopyGreen     = { 120, 190, 95, 255 };

    // Earth & Wood
    constexpr Color SoilDark        = { 44, 30, 22, 255 };
    constexpr Color BarkBrown       = { 82, 54, 38, 255 };
    constexpr Color TimberWarm      = { 130, 86, 56, 255 };
    constexpr Color PathDirt        = { 178, 130, 88, 255 };

    // Water & Magic
    constexpr Color DeepOcean       = { 20, 40, 70, 255 };
    constexpr Color RiverBlue       = { 45, 95, 155, 255 };
    constexpr Color StreamCyan      = { 85, 170, 210, 255 };
    constexpr Color MysticGlow      = { 140, 230, 245, 255 };

    // Fantasy Metals & Accents
    constexpr Color AncientGold     = { 225, 170, 60, 255 };
    constexpr Color AmberTorch      = { 245, 120, 45, 255 };
    constexpr Color CrystalRuby     = { 210, 50, 75, 255 };
    constexpr Color PureWhite       = { 245, 245, 250, 255 };

    // UI & Debug
    constexpr Color DebugBounds     = { 255, 60, 90, 255 };
    constexpr Color DebugGrid       = { 60, 75, 105, 180 };
    constexpr Color DebugTarget     = { 255, 220, 50, 255 };
}

} // namespace wildbound::graphics

