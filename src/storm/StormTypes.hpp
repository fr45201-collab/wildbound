#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <cmath>
#include <algorithm>

namespace wildbound::storm {

/**
 * Geometric Safe Zone Definition in 2D World Coordinates.
 */
struct SafeZone {
    Vector2 center{ 5120.0f, 5120.0f };
    float radius = 4000.0f;

    [[nodiscard]] bool Contains(Vector2 point) const noexcept {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }

    [[nodiscard]] float DistanceTo(Vector2 point) const noexcept {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    [[nodiscard]] float SignedDistanceToBoundary(Vector2 point) const noexcept {
        return DistanceTo(point) - radius;
    }

    [[nodiscard]] Vector2 GetClosestSafePoint(Vector2 point) const noexcept {
        float d = DistanceTo(point);
        if (d <= radius || d < 0.001f) {
            return point;
        }
        float scale = radius / d;
        return Vector2{
            center.x + (point.x - center.x) * scale,
            center.y + (point.y - center.y) * scale
        };
    }

    static SafeZone Lerp(const SafeZone& a, const SafeZone& b, float t) noexcept {
        t = std::clamp(t, 0.0f, 1.0f);
        return SafeZone{
            Vector2{
                a.center.x + (b.center.x - a.center.x) * t,
                a.center.y + (b.center.y - a.center.y) * t
            },
            a.radius + (b.radius - a.radius) * t
        };
    }
};

/**
 * High-Level State of the Corruption Storm Cycle.
 */
enum class StormState {
    Idle,           // Not started
    Warning,        // Safe zone static; countdown to next shrink
    Shrinking,      // Safe zone actively moving/shrinking
    Paused,         // Timers paused (e.g., game paused or debug)
    Completed       // Final phase collapsed to terminal radius
};

inline const char* StormStateToString(StormState state) noexcept {
    switch (state) {
        case StormState::Idle:      return "Idle";
        case StormState::Warning:   return "Warning";
        case StormState::Shrinking: return "Shrinking";
        case StormState::Paused:    return "Paused";
        case StormState::Completed: return "Completed";
        default:                    return "Unknown";
    }
}

/**
 * Status of a Player Relative to the Safe Zone.
 */
enum class PlayerStormStatus {
    Safe,           // Inside safe zone, no immediate danger
    Approaching,    // Inside safe zone, but storm warning active
    Shrinking,      // Inside shrinking zone
    Outside,        // Outside safe zone! Taking corruption damage
    FinalZone       // In final collapsed ring
};

inline const char* PlayerStormStatusToString(PlayerStormStatus status) noexcept {
    switch (status) {
        case PlayerStormStatus::Safe:        return "Safe";
        case PlayerStormStatus::Approaching: return "Storm Approaching";
        case PlayerStormStatus::Shrinking:   return "Zone Shrinking";
        case PlayerStormStatus::Outside:     return "Outside Safe Zone";
        case PlayerStormStatus::FinalZone:   return "Final Safe Zone";
        default:                             return "Unknown";
    }
}

/**
 * Configuration Parameters for a Single Storm Phase.
 */
struct StormPhaseConfig {
    int phaseIndex = 1;
    std::string name = "Phase 1";
    float warningDuration = 60.0f; // Seconds
    float shrinkDuration = 90.0f;  // Seconds
    float targetRadiusRatio = 0.75f; // Fraction of initial radius
    float damagePerSecond = 1.0f;
    std::string warningMessage = "CORRUPTION STORM APPROACHING";
    float visualIntensity = 0.40f;
    std::string audioEvent = "sfx_storm_warning";

    static StormPhaseConfig FromJson(const nlohmann::json& j) {
        StormPhaseConfig p;
        p.phaseIndex = j.value("phaseIndex", 1);
        p.name = j.value("name", "Phase " + std::to_string(p.phaseIndex));
        p.warningDuration = j.value("warningDuration", 60.0f);
        p.shrinkDuration = j.value("shrinkDuration", 90.0f);
        p.targetRadiusRatio = j.value("targetRadiusRatio", 0.75f);
        p.damagePerSecond = j.value("damagePerSecond", 1.0f);
        p.warningMessage = j.value("warningMessage", "CORRUPTION STORM APPROACHING");
        p.visualIntensity = j.value("visualIntensity", 0.4f);
        p.audioEvent = j.value("audioEvent", "sfx_storm_warning");
        return p;
    }
};

/**
 * Root Storm Configuration Definition.
 */
struct StormConfig {
    std::string name = "Wildbound Battle Royale Standard Storm";
    std::string version = "1.0.0";
    float initialRadius = 4000.0f;
    Vector2 initialCenter{ 5120.0f, 5120.0f };
    float centerDriftMaxRatio = 0.40f;
    std::vector<StormPhaseConfig> phases;

    static StormConfig CreateDefault(Rectangle playableBounds) {
        StormConfig cfg;
        cfg.initialCenter = Vector2{
            playableBounds.x + playableBounds.width * 0.5f,
            playableBounds.y + playableBounds.height * 0.5f
        };
        cfg.initialRadius = std::min(playableBounds.width, playableBounds.height) * 0.48f;
        
        cfg.phases = {
            { 1, "Phase 1 - Gathering Storm", 60.0f, 90.0f, 0.75f, 1.0f, "CORRUPTION STORM APPROACHING", 0.40f, "sfx_storm_warning" },
            { 2, "Phase 2 - Violet Encroachment", 45.0f, 75.0f, 0.55f, 2.0f, "SAFE ZONE IS SHRINKING", 0.55f, "sfx_storm_warning" },
            { 3, "Phase 3 - Miasma Surge", 40.0f, 65.0f, 0.35f, 4.0f, "SAFE ZONE IS SHRINKING", 0.70f, "sfx_storm_warning" },
            { 4, "Phase 4 - Deep Corruption", 35.0f, 55.0f, 0.20f, 6.0f, "SAFE ZONE IS SHRINKING", 0.85f, "sfx_storm_warning" },
            { 5, "Phase 5 - Cataclysmic Collapse", 30.0f, 45.0f, 0.10f, 8.0f, "SAFE ZONE IS SHRINKING", 0.95f, "sfx_storm_warning" },
            { 6, "Final Phase - The Eye of Corruption", 20.0f, 30.0f, 0.03f, 12.0f, "FINAL SAFE ZONE COLLAPSING", 1.0f, "sfx_storm_warning" }
        };
        return cfg;
    }

    static StormConfig LoadFromJson(const std::string& path, Rectangle playableBounds) {
        StormConfig cfg = CreateDefault(playableBounds);
        try {
            std::ifstream file(path);
            if (!file.is_open()) return cfg;
            nlohmann::json j;
            file >> j;
            cfg.name = j.value("name", cfg.name);
            cfg.version = j.value("version", cfg.version);
            cfg.initialRadius = j.value("initialRadius", cfg.initialRadius);
            if (j.contains("initialCenter")) {
                cfg.initialCenter.x = j["initialCenter"].value("x", cfg.initialCenter.x);
                cfg.initialCenter.y = j["initialCenter"].value("y", cfg.initialCenter.y);
            }
            cfg.centerDriftMaxRatio = j.value("centerDriftMaxRatio", cfg.centerDriftMaxRatio);
            if (j.contains("phases") && j["phases"].is_array() && !j["phases"].empty()) {
                cfg.phases.clear();
                for (const auto& item : j["phases"]) {
                    cfg.phases.push_back(StormPhaseConfig::FromJson(item));
                }
            }
        } catch (...) {
            return CreateDefault(playableBounds);
        }
        return cfg;
    }
};

} // namespace wildbound::storm

