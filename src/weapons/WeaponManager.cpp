#include "WeaponManager.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using wildbound::core::Logger;

namespace wildbound::weapons {

WeaponManager::WeaponManager() = default;
WeaponManager::~WeaponManager() = default;

void WeaponManager::AssignDefaultStats(WeaponConfig& cfg) {
    // Default fallback stats
    cfg.stats.baseDamage = 20.0f;
    cfg.stats.attackSpeed = 1.0f;
    cfg.stats.attackCooldown = 0.5f;
    cfg.stats.range = 32.0f;
    cfg.stats.hitboxWidth = 32.0f;
    cfg.stats.hitboxHeight = 24.0f;
    cfg.stats.critChance = 0.05f;
    cfg.stats.critMultiplier = 1.5f;
    cfg.stats.staminaCost = 10.0f;
    cfg.stats.meleeArc = 45.0f;
    cfg.stats.drawDuration = 0.45f;
    cfg.stats.projectileSpeed = 480.0f;
    cfg.defaultRarity = WeaponRarity::Common;

    // Swords
    if (cfg.id == "sword_wooden_training_001") {
        cfg.stats.baseDamage = 12.0f;
        cfg.stats.attackCooldown = 0.35f;
        cfg.stats.range = 28.0f;
        cfg.stats.hitboxWidth = 28.0f;
        cfg.stats.hitboxHeight = 20.0f;
        cfg.stats.critChance = 0.03f;
        cfg.stats.critMultiplier = 1.25f;
        cfg.stats.staminaCost = 6.0f;
        cfg.defaultRarity = WeaponRarity::Common;
    } else if (cfg.id == "sword_rusty_short_001") {
        cfg.stats.baseDamage = 18.0f;
        cfg.stats.attackCooldown = 0.40f;
        cfg.stats.range = 30.0f;
        cfg.stats.hitboxWidth = 30.0f;
        cfg.stats.hitboxHeight = 22.0f;
        cfg.stats.critChance = 0.05f;
        cfg.stats.critMultiplier = 1.4f;
        cfg.stats.staminaCost = 8.0f;
        cfg.defaultRarity = WeaponRarity::Common;
    } else if (cfg.id == "sword_iron_001") {
        cfg.stats.baseDamage = 25.0f;
        cfg.stats.attackCooldown = 0.45f;
        cfg.stats.range = 34.0f;
        cfg.stats.hitboxWidth = 34.0f;
        cfg.stats.hitboxHeight = 24.0f;
        cfg.stats.critChance = 0.08f;
        cfg.stats.critMultiplier = 1.5f;
        cfg.stats.staminaCost = 10.0f;
        cfg.defaultRarity = WeaponRarity::Uncommon;
    } else if (cfg.id == "sword_steel_longsword_001") {
        cfg.stats.baseDamage = 34.0f;
        cfg.stats.attackCooldown = 0.55f;
        cfg.stats.range = 42.0f;
        cfg.stats.hitboxWidth = 42.0f;
        cfg.stats.hitboxHeight = 28.0f;
        cfg.stats.critChance = 0.10f;
        cfg.stats.critMultiplier = 1.6f;
        cfg.stats.staminaCost = 14.0f;
        cfg.defaultRarity = WeaponRarity::Rare;
    } else if (cfg.id == "sword_curved_001") {
        cfg.stats.baseDamage = 28.0f;
        cfg.stats.attackCooldown = 0.38f;
        cfg.stats.range = 32.0f;
        cfg.stats.hitboxWidth = 32.0f;
        cfg.stats.hitboxHeight = 24.0f;
        cfg.stats.critChance = 0.18f;
        cfg.stats.critMultiplier = 1.8f;
        cfg.stats.staminaCost = 9.0f;
        cfg.defaultRarity = WeaponRarity::Epic;
    } else if (cfg.id == "sword_ancient_rune_001") {
        cfg.stats.baseDamage = 45.0f;
        cfg.stats.attackCooldown = 0.42f;
        cfg.stats.range = 38.0f;
        cfg.stats.hitboxWidth = 38.0f;
        cfg.stats.hitboxHeight = 28.0f;
        cfg.stats.critChance = 0.20f;
        cfg.stats.critMultiplier = 2.0f;
        cfg.stats.staminaCost = 12.0f;
        cfg.defaultRarity = WeaponRarity::Legendary;
    }
    // Bows
    else if (cfg.id == "bow_wooden_hunting_001") {
        cfg.stats.baseDamage = 20.0f;
        cfg.stats.attackCooldown = 0.45f;
        cfg.stats.drawDuration = 0.35f;
        cfg.stats.projectileSpeed = 420.0f;
        cfg.stats.range = 300.0f;
        cfg.stats.critChance = 0.05f;
        cfg.stats.critMultiplier = 1.5f;
        cfg.stats.staminaCost = 8.0f;
        cfg.defaultRarity = WeaponRarity::Common;
    } else if (cfg.id == "bow_longbow_001") {
        cfg.stats.baseDamage = 36.0f;
        cfg.stats.attackCooldown = 0.70f;
        cfg.stats.drawDuration = 0.60f;
        cfg.stats.projectileSpeed = 520.0f;
        cfg.stats.range = 450.0f;
        cfg.stats.critChance = 0.10f;
        cfg.stats.critMultiplier = 1.75f;
        cfg.stats.staminaCost = 15.0f;
        cfg.defaultRarity = WeaponRarity::Rare;
    } else if (cfg.id == "bow_reinforced_001") {
        cfg.stats.baseDamage = 30.0f;
        cfg.stats.attackCooldown = 0.55f;
        cfg.stats.drawDuration = 0.50f;
        cfg.stats.projectileSpeed = 480.0f;
        cfg.stats.range = 380.0f;
        cfg.stats.critChance = 0.08f;
        cfg.stats.critMultiplier = 1.6f;
        cfg.stats.staminaCost = 12.0f;
        cfg.defaultRarity = WeaponRarity::Uncommon;
    } else if (cfg.id == "bow_recurve_001") {
        cfg.stats.baseDamage = 28.0f;
        cfg.stats.attackCooldown = 0.45f;
        cfg.stats.drawDuration = 0.40f;
        cfg.stats.projectileSpeed = 460.0f;
        cfg.stats.range = 350.0f;
        cfg.stats.critChance = 0.15f;
        cfg.stats.critMultiplier = 1.8f;
        cfg.stats.staminaCost = 10.0f;
        cfg.defaultRarity = WeaponRarity::Epic;
    } else if (cfg.id == "bow_ancient_rune_001") {
        cfg.stats.baseDamage = 48.0f;
        cfg.stats.attackCooldown = 0.50f;
        cfg.stats.drawDuration = 0.45f;
        cfg.stats.projectileSpeed = 560.0f;
        cfg.stats.range = 500.0f;
        cfg.stats.critChance = 0.22f;
        cfg.stats.critMultiplier = 2.0f;
        cfg.stats.staminaCost = 14.0f;
        cfg.defaultRarity = WeaponRarity::Mythic;
    }
    // Ammunition
    else if (cfg.id == "arrow_iron_001") {
        cfg.stats.baseDamage = 5.0f;
        cfg.stats.projectileSpeed = 480.0f;
        cfg.defaultRarity = WeaponRarity::Common;
    }
}

bool WeaponManager::Initialize(const std::string& catalogPath) {
    m_weapons.clear();
    m_weaponIndexMap.clear();

    Logger::Info("WEAPON", "Initializing WeaponManager from catalog: " + catalogPath);

    std::filesystem::path catPath(catalogPath);
    if (!std::filesystem::exists(catPath)) {
        Logger::Error("WEAPON", "Weapon catalog file does not exist: " + catalogPath);
        return false;
    }

    std::ifstream file(catPath);
    if (!file.is_open()) {
        Logger::Error("WEAPON", "Failed to open weapon catalog file: " + catalogPath);
        return false;
    }

    try {
        nlohmann::json root;
        file >> root;

        if (!root.contains("weapons") || !root["weapons"].is_array()) {
            Logger::Error("WEAPON", "Invalid weapon catalog: missing 'weapons' array");
            return false;
        }

        for (const auto& wJson : root["weapons"]) {
            WeaponConfig cfg;
            cfg.id = wJson.value("id", "");
            if (cfg.id.empty()) {
                Logger::Warn("WEAPON", "Skipping weapon with empty ID.");
                continue;
            }

            cfg.displayName = wJson.value("display_name", cfg.id);
            std::string catStr = wJson.value("category", "sword");
            cfg.category = StringToWeaponCategory(catStr);
            cfg.subcategory = wJson.value("subcategory", "");
            cfg.description = wJson.value("description", "");

            // Assign balanced default stats based on canonical table
            AssignDefaultStats(cfg);

            // Optional JSON overrides for statistics
            if (wJson.contains("stats") && wJson["stats"].is_object()) {
                const auto& s = wJson["stats"];
                cfg.stats.baseDamage = s.value("base_damage", cfg.stats.baseDamage);
                cfg.stats.attackSpeed = s.value("attack_speed", cfg.stats.attackSpeed);
                cfg.stats.attackCooldown = s.value("attack_cooldown", cfg.stats.attackCooldown);
                cfg.stats.range = s.value("range", cfg.stats.range);
                cfg.stats.hitboxWidth = s.value("hitbox_width", cfg.stats.hitboxWidth);
                cfg.stats.hitboxHeight = s.value("hitbox_height", cfg.stats.hitboxHeight);
                cfg.stats.critChance = s.value("crit_chance", cfg.stats.critChance);
                cfg.stats.critMultiplier = s.value("crit_multiplier", cfg.stats.critMultiplier);
                cfg.stats.staminaCost = s.value("stamina_cost", cfg.stats.staminaCost);
                cfg.stats.drawDuration = s.value("draw_duration", cfg.stats.drawDuration);
                cfg.stats.projectileSpeed = s.value("projectile_speed", cfg.stats.projectileSpeed);
            }
            if (wJson.contains("rarity")) {
                cfg.defaultRarity = StringToWeaponRarity(wJson["rarity"].get<std::string>());
            }

            // Visual Assets (Texture IDs follow AMS naming convention tex_<id>_icon, etc.)
            cfg.visuals.iconTextureId = "tex_" + cfg.id + "_icon";
            cfg.visuals.groundTextureId = "tex_" + cfg.id + "_ground";
            cfg.visuals.heldTextureId = "tex_" + cfg.id + "_held";
            cfg.visuals.heldSheetId = "sheet_" + cfg.id + "_held";
            cfg.visuals.animTextureId = "tex_" + cfg.id + "_anim";
            cfg.visuals.animSheetId = "sheet_" + cfg.id + "_anim";

            // Attachment points
            if (wJson.contains("grip_anchor") && wJson["grip_anchor"].is_object()) {
                cfg.attachment.gripAnchor.x = wJson["grip_anchor"].value("x", 12.0f);
                cfg.attachment.gripAnchor.y = wJson["grip_anchor"].value("y", 18.0f);
            }
            cfg.attachment.hand = wJson.value("hand", "right_hand");

            if (wJson.contains("render_order") && wJson["render_order"].is_object()) {
                const auto& ro = wJson["render_order"];
                cfg.attachment.renderOrderDown = ro.value("down", 85);
                cfg.attachment.renderOrderUp = ro.value("up", 18);
                cfg.attachment.renderOrderLeft = ro.value("left", 85);
                cfg.attachment.renderOrderRight = ro.value("right", 85);
            }

            // Directions
            if (wJson.contains("directions") && wJson["directions"].is_array()) {
                cfg.directions.clear();
                for (const auto& d : wJson["directions"]) {
                    cfg.directions.push_back(d.get<std::string>());
                }
            }

            // Animation frames
            if (wJson.contains("animation_frames") && wJson["animation_frames"].is_array()) {
                cfg.animationFrames.clear();
                for (const auto& af : wJson["animation_frames"]) {
                    cfg.animationFrames.push_back(af.get<std::string>());
                }
            }

            size_t idx = m_weapons.size();
            m_weaponIndexMap[cfg.id] = idx;
            m_weapons.push_back(std::move(cfg));
        }

        Logger::Info("WEAPON", "Successfully loaded " + std::to_string(m_weapons.size()) +
                     " weapon definitions from catalog.");
        return !m_weapons.empty();
    } catch (const std::exception& e) {
        Logger::Error("WEAPON", "Error parsing weapon catalog: " + std::string(e.what()));
        return false;
    }
}

const WeaponConfig* WeaponManager::GetWeapon(const std::string& id) const {
    auto it = m_weaponIndexMap.find(id);
    if (it != m_weaponIndexMap.end() && it->second < m_weapons.size()) {
        return &m_weapons[it->second];
    }
    return nullptr;
}

bool WeaponManager::HasWeapon(const std::string& id) const {
    return m_weaponIndexMap.find(id) != m_weaponIndexMap.end();
}

std::vector<const WeaponConfig*> WeaponManager::GetWeaponsByCategory(WeaponCategory category) const {
    std::vector<const WeaponConfig*> result;
    for (const auto& w : m_weapons) {
        if (w.category == category) {
            result.push_back(&w);
        }
    }
    return result;
}

WeaponInstance WeaponManager::CreateInstance(const std::string& id, WeaponRarity rarity) const {
    const auto* cfg = GetWeapon(id);
    if (!cfg) {
        return WeaponInstance{};
    }
    return WeaponInstance{ ++m_nextInstanceId, cfg->id, rarity, cfg->stats };
}

bool WeaponManager::ValidateAssets(const resources::AssetManager& assetMgr, std::vector<std::string>& outErrors) const {
    outErrors.clear();
    const auto& reg = assetMgr.GetRegistry();

    for (const auto& w : m_weapons) {
        // Icon texture check
        if (!reg.HasAsset(w.visuals.iconTextureId)) {
            outErrors.push_back("[" + w.id + "] Missing icon texture in AMS: " + w.visuals.iconTextureId);
        }

        // Ground texture check
        if (!reg.HasAsset(w.visuals.groundTextureId)) {
            outErrors.push_back("[" + w.id + "] Missing ground texture in AMS: " + w.visuals.groundTextureId);
        }

        // Held texture and sheet check
        if (!reg.HasAsset(w.visuals.heldTextureId)) {
            outErrors.push_back("[" + w.id + "] Missing held texture in AMS: " + w.visuals.heldTextureId);
        }
        if (!reg.HasAsset(w.visuals.heldSheetId)) {
            outErrors.push_back("[" + w.id + "] Missing held spritesheet in AMS: " + w.visuals.heldSheetId);
        }

        // Anim texture and sheet check
        if (!reg.HasAsset(w.visuals.animTextureId)) {
            outErrors.push_back("[" + w.id + "] Missing animation texture in AMS: " + w.visuals.animTextureId);
        }
        if (!reg.HasAsset(w.visuals.animSheetId)) {
            outErrors.push_back("[" + w.id + "] Missing animation spritesheet in AMS: " + w.visuals.animSheetId);
        }
    }

    if (!outErrors.empty()) {
        Logger::Warn("WEAPON", "Weapon asset validation found " + std::to_string(outErrors.size()) + " errors.");
        return false;
    }

    Logger::Info("WEAPON", "Weapon asset validation passed 100% for all " +
                 std::to_string(m_weapons.size()) + " weapons.");
    return true;
}

} // namespace wildbound::weapons
