#include "HUDManager.hpp"
#include "../../entities/Player.hpp"
#include "../../entities/EquipmentSystem.hpp"
#include "../../combat/HealthComponent.hpp"
#include "../../items/Inventory.hpp"
#include "../../items/ItemManager.hpp"
#include "../../interaction/InteractionManager.hpp"
#include "../../interaction/InteractionPrompt.hpp"
#include "../../resources/AssetManager.hpp"
#include <cstdio>
#include <algorithm>
#include <cmath>

namespace wildbound::ui {

HUDManager::HUDManager() {
    Reset();
}

void HUDManager::Reset() {
    m_hudVisible = true;
    m_displayedHealth = 100.0f;
    m_lagHealth = 100.0f;
    m_maxHealth = 100.0f;
    m_lagTimer = 0.0f;
    m_lowHealthPulseTimer = 0.0f;
    m_isLowHealthAlert = false;
    m_notifications.Clear();
    m_matchInfo = MatchHUDInfo{};
}

void HUDManager::Update(float dt, const entities::Player* player) {
    m_notifications.Update(dt);
    m_minimap.Update(dt);

    if (player) {
        const auto& health = player->GetHealth();
        m_maxHealth = health.GetMaxHealth();
        float currentHp = health.GetCurrentHealth();

        if (currentHp < m_displayedHealth) {
            // Damage taken: drop displayed health immediately and arm lag timer
            m_displayedHealth = currentHp;
            m_lagTimer = 0.5f;
        } else if (currentHp > m_displayedHealth) {
            // Healing: increase displayed and lag health immediately
            m_displayedHealth = currentHp;
            m_lagHealth = currentHp;
        }

        // Handle lag health interpolation
        if (m_lagTimer > 0.0f) {
            m_lagTimer -= dt;
            if (m_lagTimer <= 0.0f) {
                float excessDt = -m_lagTimer;
                m_lagTimer = 0.0f;
                constexpr float LagCatchSpeed = 65.0f;
                m_lagHealth = std::max(m_displayedHealth, m_lagHealth - LagCatchSpeed * excessDt);
            }
        } else if (m_lagHealth > m_displayedHealth) {
            constexpr float LagCatchSpeed = 65.0f; // HP/sec catch-down rate
            m_lagHealth = std::max(m_displayedHealth, m_lagHealth - LagCatchSpeed * dt);
        }

        // Low health alert pulse (at or below 25%)
        float pct = (m_maxHealth > 0.0f) ? (m_displayedHealth / m_maxHealth) : 0.0f;
        m_isLowHealthAlert = (pct <= 0.25f && m_displayedHealth > 0.0f);
        if (m_isLowHealthAlert) {
            m_lowHealthPulseTimer += dt * 6.0f;
            if (m_lowHealthPulseTimer > 2.0f * 3.14159f) {
                m_lowHealthPulseTimer -= 2.0f * 3.14159f;
            }
        } else {
            m_lowHealthPulseTimer = 0.0f;
        }
    }

    // Match Timer progression
    m_matchInfo.matchTimer += dt;
    if (m_matchInfo.stormTimer > 0.0f) {
        m_matchInfo.stormTimer -= dt;
    }
}

void HUDManager::Render(resources::AssetManager& assetMgr,
                        const entities::Player* player,
                        const world::World* world,
                        const items::ItemManager* itemMgr,
                        const interaction::InteractionPrompt* prompt) {
    if (!m_hudVisible) return;

    // 1. Match Information Header
    RenderMatchInfo();

    // 2. Minimap Radar
    m_minimap.Render(world, player);

    // 3. Player Vitals (Health, Armor, Status)
    if (player) {
        RenderVitals(player);
        RenderArmor(player, itemMgr);
        RenderPlayerStatus(player);
        RenderWeaponInterface(player, itemMgr);
        RenderAmmoCounter(player, itemMgr);
    }

    // 4. Interaction Prompt
    if (prompt && prompt->visible) {
        RenderInteractionPrompt(prompt);
    }

    // 5. Toast Notifications
    m_notifications.Render(Vector2{ 12.0f, 48.0f });

    // 6. Low Health Critical Screen Vignette
    if (m_isLowHealthAlert) {
        float pulse = 0.5f + 0.5f * std::sin(m_lowHealthPulseTimer);
        unsigned char alpha = static_cast<unsigned char>(30 + pulse * 45);
        DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), 4, Color{ 220, 20, 20, alpha });
        DrawRectangle(0, static_cast<int>(UITheme::CanvasHeight - 4), static_cast<int>(UITheme::CanvasWidth), 4, Color{ 220, 20, 20, alpha });
        DrawRectangle(0, 0, 4, static_cast<int>(UITheme::CanvasHeight), Color{ 220, 20, 20, alpha });
        DrawRectangle(static_cast<int>(UITheme::CanvasWidth - 4), 0, 4, static_cast<int>(UITheme::CanvasHeight), Color{ 220, 20, 20, alpha });
    }
}

void HUDManager::RenderVitals(const entities::Player* player) {
    const float barX = 14.0f;
    const float barY = 328.0f;
    const float barW = 140.0f;
    const float barH = 12.0f;

    // Frame & Background
    DrawRectangle(static_cast<int>(barX - 2), static_cast<int>(barY - 2),
                  static_cast<int>(barW + 4), static_cast<int>(barH + 4), UITheme::DarkBgOpaque);
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                  static_cast<int>(barW), static_cast<int>(barH), UITheme::HealthBg);

    float maxHp = (m_maxHealth > 0.0f) ? m_maxHealth : 100.0f;
    float lagPct = std::clamp(m_lagHealth / maxHp, 0.0f, 1.0f);
    float dispPct = std::clamp(m_displayedHealth / maxHp, 0.0f, 1.0f);

    // 1. Lag Bar (Orange/Yellow trailing damage)
    if (lagPct > dispPct) {
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                      static_cast<int>(barW * lagPct), static_cast<int>(barH), UITheme::HealthLag);
    }

    // 2. Current Health Fill (Red or pulsing bright red if low)
    Color fillColor = UITheme::HealthFill;
    if (m_isLowHealthAlert) {
        float pulse = 0.5f + 0.5f * std::sin(m_lowHealthPulseTimer);
        fillColor = Color{ static_cast<unsigned char>(200 + pulse * 55),
                           static_cast<unsigned char>(30 + pulse * 20),
                           static_cast<unsigned char>(30 + pulse * 20), 255 };
    }
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                  static_cast<int>(barW * dispPct), static_cast<int>(barH), fillColor);

    // Top highlight line for gloss
    if (dispPct > 0.0f) {
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                      static_cast<int>(barW * dispPct), 2, Color{ 255, 140, 140, 180 });
    }

    // Outer border
    DrawRectangleLines(static_cast<int>(barX - 1), static_cast<int>(barY - 1),
                       static_cast<int>(barW + 2), static_cast<int>(barH + 2), UITheme::BorderNormal);

    // Health text label
    char hpBuf[32];
    std::snprintf(hpBuf, sizeof(hpBuf), "HP %d / %d", static_cast<int>(m_displayedHealth), static_cast<int>(maxHp));
    int tw = MeasureText(hpBuf, UITheme::FontSmall);
    DrawText(hpBuf, static_cast<int>(barX + (barW - tw) * 0.5f), static_cast<int>(barY + 1), UITheme::FontSmall, UITheme::TextLight);
}

void HUDManager::RenderArmor(const entities::Player* player, const items::ItemManager* itemMgr) {
    const float barX = 14.0f;
    const float barY = 312.0f;
    const float barW = 140.0f;
    const float barH = 10.0f;

    const auto& equip = player->GetEquipmentSystem();
    float defRating = equip.GetTotalDefense();
    float mitPct = equip.GetDamageReduction();

    // Background
    DrawRectangle(static_cast<int>(barX - 2), static_cast<int>(barY - 2),
                  static_cast<int>(barW + 4), static_cast<int>(barH + 4), UITheme::DarkBgOpaque);
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                  static_cast<int>(barW), static_cast<int>(barH), UITheme::ArmorBg);

    // Mitigation Fill (0 to 100% mapped, standard cap around 60% armor reduction)
    float fillNormalized = std::clamp(mitPct / 0.75f, 0.0f, 1.0f);
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                  static_cast<int>(barW * fillNormalized), static_cast<int>(barH), UITheme::ArmorFill);

    if (fillNormalized > 0.0f) {
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                      static_cast<int>(barW * fillNormalized), 2, UITheme::ArmorHigh);
    }

    DrawRectangleLines(static_cast<int>(barX - 1), static_cast<int>(barY - 1),
                       static_cast<int>(barW + 2), static_cast<int>(barH + 2), UITheme::BorderNormal);

    char armBuf[32];
    std::snprintf(armBuf, sizeof(armBuf), "ARMOR %d (%d%%)", static_cast<int>(defRating), static_cast<int>(mitPct * 100.0f));
    int tw = MeasureText(armBuf, UITheme::FontMicro);
    DrawText(armBuf, static_cast<int>(barX + (barW - tw) * 0.5f), static_cast<int>(barY + 1), UITheme::FontMicro, UITheme::TextLight);
}

void HUDManager::RenderWeaponInterface(const entities::Player* player, const items::ItemManager* itemMgr) {
    const auto& equip = player->GetEquipmentSystem();
    entities::EquipmentSlot activeSlot = equip.GetActiveWeaponSlot();

    const float slotSize = 34.0f;
    const float startX = 282.0f;
    const float startY = 318.0f;
    const float spacing = 42.0f;

    for (int i = 0; i < 2; ++i) {
        entities::EquipmentSlot slot = (i == 0) ? entities::EquipmentSlot::MainHand : entities::EquipmentSlot::OffHand;
        bool isActive = (slot == activeSlot);
        float posX = startX + i * spacing;

        Rectangle slotRect{ posX, startY, slotSize, slotSize };

        // Background
        DrawRectangleRec(slotRect, isActive ? UITheme::SlotFilled : UITheme::SlotEmpty);

        // Border
        Color borderColor = isActive ? UITheme::BorderGold : UITheme::BorderNormal;
        DrawRectangleLinesEx(slotRect, isActive ? 2.0f : 1.0f, borderColor);

        // Slot key indicator badge [1] or [2]
        char keyLabel[4];
        std::snprintf(keyLabel, sizeof(keyLabel), "%d", i + 1);
        DrawRectangle(static_cast<int>(posX + 2), static_cast<int>(startY + 2), 10, 10, UITheme::DarkBgOpaque);
        DrawText(keyLabel, static_cast<int>(posX + 4), static_cast<int>(startY + 2), UITheme::FontMicro, isActive ? UITheme::Gold : UITheme::TextMuted);

        // Equipped item content
        const auto& itemOpt = equip.GetEquipped(slot);
        if (itemOpt.has_value()) {
            std::string name = itemOpt->definitionId;
            // Shorten name if needed
            if (name.length() > 6) name = name.substr(0, 6);
            DrawText(name.c_str(), static_cast<int>(posX + 3), static_cast<int>(startY + 14), UITheme::FontMicro, UITheme::TextLight);
        } else {
            DrawText("Empty", static_cast<int>(posX + 4), static_cast<int>(startY + 14), UITheme::FontMicro, UITheme::TextDim);
        }

        // Active weapon cooldown / attack overlay
        if (isActive) {
            const auto& weaponCtrl = player->GetWeaponController();
            float cd = weaponCtrl.GetCooldownRemaining();
            if (cd > 0.0f) {
                float cdPct = std::clamp(cd / 0.5f, 0.0f, 1.0f);
                DrawRectangle(static_cast<int>(posX), static_cast<int>(startY + slotSize * (1.0f - cdPct)),
                              static_cast<int>(slotSize), static_cast<int>(slotSize * cdPct), Color{ 0, 0, 0, 140 });
            }

            // Bow Draw Charge Bar
            if (weaponCtrl.IsDrawing()) {
                float drawProgress = weaponCtrl.GetDrawProgress();
                Rectangle chargeBar{ posX, startY - 8.0f, slotSize, 5.0f };
                DrawRectangleRec(chargeBar, UITheme::DarkBgOpaque);
                Color chargeCol = (drawProgress >= 1.0f) ? UITheme::Gold : UITheme::TextInfo;
                DrawRectangle(static_cast<int>(chargeBar.x), static_cast<int>(chargeBar.y),
                              static_cast<int>(slotSize * drawProgress), 5, chargeCol);
                DrawRectangleLinesEx(chargeBar, 1.0f, UITheme::BorderNormal);
            }
        }
    }
}

void HUDManager::RenderAmmoCounter(const entities::Player* player, const items::ItemManager* itemMgr) {
    const auto& inv = player->GetInventory();
    int arrowCount = 0;

    // Aggregate arrows in inventory
    for (size_t i = 0; i < inv.GetCapacity(); ++i) {
        const auto& slot = inv.GetSlot(i);
        if (slot.has_value()) {
            if (slot->definitionId.find("arrow") != std::string::npos ||
                slot->definitionId.find("ammo") != std::string::npos) {
                arrowCount += slot->quantity;
            }
        }
    }

    const float ammoX = 372.0f;
    const float ammoY = 328.0f;

    DrawRectangle(static_cast<int>(ammoX), static_cast<int>(ammoY), 54, 24, UITheme::DarkBgOpaque);
    DrawRectangleLines(static_cast<int>(ammoX), static_cast<int>(ammoY), 54, 24, UITheme::BorderNormal);

    DrawText("AMMO", static_cast<int>(ammoX + 4), static_cast<int>(ammoY + 2), UITheme::FontMicro, UITheme::TextMuted);

    char countBuf[16];
    std::snprintf(countBuf, sizeof(countBuf), "%d", arrowCount);
    DrawText(countBuf, static_cast<int>(ammoX + 4), static_cast<int>(ammoY + 11), UITheme::FontSmall, arrowCount > 0 ? UITheme::Gold : UITheme::TextDanger);
}

void HUDManager::RenderPlayerStatus(const entities::Player* player) {
    const float startX = 14.0f;
    const float startY = 296.0f;

    std::string statusLabel = "NORMAL";
    Color statusCol = UITheme::TextSuccess;

    if (m_displayedHealth <= 0.0f) {
        statusLabel = "DEFEATED";
        statusCol = UITheme::TextDanger;
    } else if (m_isLowHealthAlert) {
        statusLabel = "CRITICAL";
        statusCol = UITheme::TextDanger;
    }

    Rectangle pillRect{ startX, startY, 56.0f, 13.0f };
    UITheme::DrawPill(pillRect, statusLabel, UITheme::DarkBgOpaque, statusCol, UITheme::FontMicro);
}

void HUDManager::RenderMatchInfo() {
    const float headerW = 160.0f;
    const float headerH = 22.0f;
    const float headerX = (UITheme::CanvasWidth - headerW) * 0.5f;
    const float headerY = 6.0f;

    DrawRectangle(static_cast<int>(headerX), static_cast<int>(headerY),
                  static_cast<int>(headerW), static_cast<int>(headerH), UITheme::DarkBgOpaque);
    DrawRectangleLines(static_cast<int>(headerX), static_cast<int>(headerY),
                       static_cast<int>(headerW), static_cast<int>(headerH), UITheme::BorderGoldDim);

    // Format Match Time (MM:SS)
    int minutes = static_cast<int>(m_matchInfo.matchTimer) / 60;
    int seconds = static_cast<int>(m_matchInfo.matchTimer) % 60;
    char timeBuf[16];
    std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", minutes, seconds);

    // Format Alive & Kills
    char statsBuf[48];
    std::snprintf(statsBuf, sizeof(statsBuf), "Alive: %d  Kills: %d  Time: %s",
                  m_matchInfo.playersAlive, m_matchInfo.eliminations, timeBuf);

    int tw = MeasureText(statsBuf, UITheme::FontMicro);
    DrawText(statsBuf, static_cast<int>(headerX + (headerW - tw) * 0.5f), static_cast<int>(headerY + 6), UITheme::FontMicro, UITheme::TextLight);
}

void HUDManager::RenderInteractionPrompt(const interaction::InteractionPrompt* prompt) {
    if (!prompt || !prompt->visible) return;

    std::string fullText = "[" + prompt->keyGlyph + "] " + prompt->verb;
    if (!prompt->targetName.empty()) {
        fullText += " " + prompt->targetName;
    }
    int tw = MeasureText(fullText.c_str(), UITheme::FontSmall);
    float boxW = static_cast<float>(tw + 20);
    float boxH = 22.0f;
    float posX = (UITheme::CanvasWidth - boxW) * 0.5f;
    float posY = 275.0f;

    DrawRectangle(static_cast<int>(posX), static_cast<int>(posY), static_cast<int>(boxW), static_cast<int>(boxH), UITheme::DarkBgOpaque);
    DrawRectangleLines(static_cast<int>(posX), static_cast<int>(posY), static_cast<int>(boxW), static_cast<int>(boxH), UITheme::Gold);

    DrawText(fullText.c_str(), static_cast<int>(posX + 10), static_cast<int>(posY + 5), UITheme::FontSmall, UITheme::TextLight);
}

} // namespace wildbound::ui
