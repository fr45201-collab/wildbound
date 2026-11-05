#include "InventoryUI.hpp"
#include "../../entities/Player.hpp"
#include "../../entities/EquipmentSystem.hpp"
#include "../../items/ItemManager.hpp"
#include "../../items/Inventory.hpp"
#include "../../world/World.hpp"
#include "../../audio/AudioSystem.hpp"
#include <cstdio>
#include <algorithm>

namespace wildbound::ui {

InventoryUI::InventoryUI()
    : m_selectedSlot(0)
    , m_hoveredSlot(-1)
    , m_categoryTab(0)
    , m_requestClose(false) {
}

void InventoryUI::Reset() {
    m_selectedSlot = 0;
    m_hoveredSlot = -1;
    m_categoryTab = 0;
    m_requestClose = false;
}

bool InventoryUI::CheckHover(Rectangle r) const {
    return (m_mousePos.x >= r.x && m_mousePos.x <= r.x + r.width &&
            m_mousePos.y >= r.y && m_mousePos.y <= r.y + r.height);
}

void InventoryUI::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed, bool mouseLeftDown,
                         entities::Player* player,
                         const items::ItemManager* itemMgr,
                         world::World* world,
                         audio::AudioSystem* audioSys,
                         bool navUp, bool navDown, bool navLeft, bool navRight,
                         bool navConfirm, bool navDrop) {
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;
    m_mouseDown = mouseLeftDown;

    // Close button
    Rectangle closeRect{ 585.0f, 20.0f, 20.0f, 20.0f };
    if (m_mousePressed && CheckHover(closeRect)) {
        m_requestClose = true;
        if (audioSys) audioSys->DispatchEvent("ui.button.click");
        return;
    }

    // Directional Navigation
    if (navUp)    m_selectedSlot = (m_selectedSlot - GridCols + TotalSlots) % TotalSlots;
    if (navDown)  m_selectedSlot = (m_selectedSlot + GridCols) % TotalSlots;
    if (navLeft)  m_selectedSlot = (m_selectedSlot - 1 + TotalSlots) % TotalSlots;
    if (navRight) m_selectedSlot = (m_selectedSlot + 1) % TotalSlots;

    // Category Tabs
    const char* tabs[] = { "ALL", "CONSUMABLES", "EQUIPMENT", "MATERIALS", "RESOURCES" };
    for (int i = 0; i < 5; ++i) {
        Rectangle tabRect{ 36.0f + i * 68.0f, 50.0f, 64.0f, 20.0f };
        if (m_mousePressed && CheckHover(tabRect)) {
            m_categoryTab = i;
            if (audioSys) audioSys->DispatchEvent("ui.button.click");
        }
    }

    // Grid Slot Click & Hover
    const float slotSize = 44.0f;
    const float startX = 36.0f;
    const float startY = 80.0f;
    const float gap = 8.0f;

    m_hoveredSlot = -1;
    for (int row = 0; row < GridRows; ++row) {
        for (int col = 0; col < GridCols; ++col) {
            int slotIdx = row * GridCols + col;
            Rectangle sRect{ startX + col * (slotSize + gap), startY + row * (slotSize + gap), slotSize, slotSize };
            if (CheckHover(sRect)) {
                m_hoveredSlot = slotIdx;
                if (m_mousePressed) {
                    m_selectedSlot = slotIdx;
                    if (audioSys) audioSys->DispatchEvent("ui.button.click");
                }
            }
        }
    }

    // Action execution (from player inventory)
    if (player && itemMgr) {
        auto& inv = player->GetInventory();
        const auto& slotOpt = inv.GetSlot(static_cast<size_t>(m_selectedSlot));

        if (slotOpt.has_value()) {
            const auto* itemDef = itemMgr->GetItem(slotOpt->definitionId);

            // Action Button Rectangles
            Rectangle useRect{ 420.0f, 245.0f, 75.0f, 22.0f };
            Rectangle drop1Rect{ 505.0f, 245.0f, 75.0f, 22.0f };
            Rectangle dropAllRect{ 505.0f, 275.0f, 75.0f, 22.0f };

            bool triggerUse = (m_mousePressed && CheckHover(useRect)) || navConfirm;
            bool triggerDrop1 = (m_mousePressed && CheckHover(drop1Rect)) || navDrop;
            bool triggerDropAll = (m_mousePressed && CheckHover(dropAllRect));

            if (triggerUse && itemDef) {
                if (itemDef->category == items::ItemCategory::Consumable) {
                    player->UseConsumable(static_cast<size_t>(m_selectedSlot), itemMgr, audioSys);
                } else if (itemDef->category == items::ItemCategory::Weapon ||
                           itemDef->category == items::ItemCategory::Armor ||
                           itemDef->category == items::ItemCategory::Clothing) {
                    player->GetEquipmentSystem().Equip(itemDef->id, *itemMgr, player);
                    if (audioSys) audioSys->DispatchEvent("Item.Equip");
                }
            }

            if (triggerDrop1) {
                if (world) {
                    inv.DropItem(static_cast<size_t>(m_selectedSlot), 1, *world,
                                 player->GetPosition(), player->GetFacingDirection(), audioSys);
                }
            }

            if (triggerDropAll) {
                if (world) {
                    inv.DropAll(static_cast<size_t>(m_selectedSlot), *world,
                                player->GetPosition(), player->GetFacingDirection(), audioSys);
                }
            }
        }
    }
}

void InventoryUI::Render(resources::AssetManager& assetMgr,
                         entities::Player* player,
                         const items::ItemManager* itemMgr,
                         world::World* world) {
    // 1. Semi-transparent overlay & main panel
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 10, 12, 16, 215 });

    Rectangle mainPanel{ 24.0f, 16.0f, 592.0f, 328.0f };
    DrawRectangleRec(mainPanel, UITheme::PanelBg);
    DrawRectangleLinesEx(mainPanel, 1.0f, UITheme::BorderGoldDim);

    // Title
    DrawText("INVENTORY & EQUIPMENT", static_cast<int>(mainPanel.x + 18.0f), 22, UITheme::FontMedium, UITheme::Gold);

    // Close [X] Button
    Rectangle closeRect{ 585.0f, 20.0f, 20.0f, 20.0f };
    bool closeHov = CheckHover(closeRect);
    DrawRectangleRec(closeRect, closeHov ? UITheme::HealthFill : UITheme::SlotEmpty);
    DrawRectangleLinesEx(closeRect, 1.0f, UITheme::BorderNormal);
    DrawText("X", static_cast<int>(closeRect.x + 6.0f), static_cast<int>(closeRect.y + 4.0f), UITheme::FontSmall, UITheme::TextLight);

    // Capacity Text
    if (player) {
        char capBuf[32];
        std::snprintf(capBuf, sizeof(capBuf), "Capacity: %d / %d",
                      static_cast<int>(player->GetInventory().GetFilledSlotCount()),
                      static_cast<int>(player->GetInventory().GetCapacity()));
        DrawText(capBuf, static_cast<int>(mainPanel.x + 220.0f), 25, UITheme::FontSmall, UITheme::TextMuted);
    }

    // 2. Category Tabs
    const char* tabs[] = { "ALL", "CONSUMABLES", "EQUIPMENT", "MATERIALS", "RESOURCES" };
    for (int i = 0; i < 5; ++i) {
        Rectangle tabRect{ 36.0f + i * 68.0f, 50.0f, 64.0f, 20.0f };
        bool isSel = (m_categoryTab == i);
        bool isHov = CheckHover(tabRect);
        DrawRectangleRec(tabRect, isSel ? UITheme::SlotSelected : (isHov ? UITheme::SlotHovered : UITheme::SlotEmpty));
        DrawRectangleLinesEx(tabRect, 1.0f, isSel ? UITheme::BorderGold : UITheme::BorderNormal);
        int tw = MeasureText(tabs[i], UITheme::FontMicro);
        DrawText(tabs[i], static_cast<int>(tabRect.x + (tabRect.width - tw) * 0.5f), static_cast<int>(tabRect.y + 5.0f),
                 UITheme::FontMicro, isSel ? UITheme::Gold : UITheme::TextLight);
    }

    // 3. 24-Slot Grid (6 cols x 4 rows)
    const float slotSize = 44.0f;
    const float startX = 36.0f;
    const float startY = 80.0f;
    const float gap = 8.0f;

    for (int row = 0; row < GridRows; ++row) {
        for (int col = 0; col < GridCols; ++col) {
            int slotIdx = row * GridCols + col;
            Rectangle sRect{ startX + col * (slotSize + gap), startY + row * (slotSize + gap), slotSize, slotSize };

            bool isSelected = (m_selectedSlot == slotIdx);
            bool isHovered = (m_hoveredSlot == slotIdx);

            DrawRectangleRec(sRect, isSelected ? UITheme::SlotSelected : (isHovered ? UITheme::SlotHovered : UITheme::SlotEmpty));

            Color bCol = isSelected ? UITheme::BorderGold : (isHovered ? UITheme::BorderLight : UITheme::BorderNormal);
            DrawRectangleLinesEx(sRect, isSelected ? 2.0f : 1.0f, bCol);

            // Slot number badge
            char numBuf[4];
            std::snprintf(numBuf, sizeof(numBuf), "%d", slotIdx + 1);
            DrawText(numBuf, static_cast<int>(sRect.x + 3), static_cast<int>(sRect.y + 2), UITheme::FontMicro, UITheme::TextDim);

            // Item representation
            if (player) {
                const auto& itemOpt = player->GetInventory().GetSlot(static_cast<size_t>(slotIdx));
                if (itemOpt.has_value()) {
                    std::string idStr = itemOpt->definitionId;
                    if (idStr.length() > 6) idStr = idStr.substr(0, 6);
                    DrawText(idStr.c_str(), static_cast<int>(sRect.x + 4), static_cast<int>(sRect.y + 16), UITheme::FontMicro, UITheme::TextLight);

                    // Stack Count
                    if (itemOpt->quantity > 1) {
                        char qtyBuf[16];
                        std::snprintf(qtyBuf, sizeof(qtyBuf), "x%d", itemOpt->quantity);
                        int qw = MeasureText(qtyBuf, UITheme::FontMicro);
                        DrawText(qtyBuf, static_cast<int>(sRect.x + sRect.width - qw - 3),
                                 static_cast<int>(sRect.y + sRect.height - 10), UITheme::FontMicro, UITheme::Gold);
                    }
                }
            }
        }
    }

    // 4. Inspector Tooltip Panel (Right Side)
    Rectangle inspRect{ 390.0f, 50.0f, 215.0f, 275.0f };
    DrawRectangleRec(inspRect, UITheme::SubPanelBg);
    DrawRectangleLinesEx(inspRect, 1.0f, UITheme::BorderNormal);

    if (player && itemMgr) {
        const auto& itemOpt = player->GetInventory().GetSlot(static_cast<size_t>(m_selectedSlot));
        if (itemOpt.has_value()) {
            const auto* def = itemMgr->GetItem(itemOpt->definitionId);
            std::string itemName = def ? def->displayName : itemOpt->definitionId;
            std::string catName = def ? std::string(items::ItemCategoryToString(def->category)) : "Item";

            // Title & Category
            DrawText(itemName.c_str(), static_cast<int>(inspRect.x + 12), static_cast<int>(inspRect.y + 12), UITheme::FontSmall, UITheme::Gold);
            DrawText(catName.c_str(), static_cast<int>(inspRect.x + 12), static_cast<int>(inspRect.y + 28), UITheme::FontMicro, UITheme::TextMuted);

            DrawLine(static_cast<int>(inspRect.x + 10), static_cast<int>(inspRect.y + 44),
                     static_cast<int>(inspRect.x + inspRect.width - 10), static_cast<int>(inspRect.y + 44), UITheme::BorderNormal);

            // Description
            if (def && !def->description.empty()) {
                DrawText(def->description.c_str(), static_cast<int>(inspRect.x + 12), static_cast<int>(inspRect.y + 54), UITheme::FontMicro, UITheme::TextLight);
            }

            // Quantity
            char countBuf[32];
            std::snprintf(countBuf, sizeof(countBuf), "Quantity: %d", itemOpt->quantity);
            DrawText(countBuf, static_cast<int>(inspRect.x + 12), static_cast<int>(inspRect.y + 100), UITheme::FontMicro, UITheme::TextMuted);

            // Stat modifiers or consumables readout
            if (def && def->category == items::ItemCategory::Consumable) {
                char healBuf[32];
                std::snprintf(healBuf, sizeof(healBuf), "Heals: +%d HP", static_cast<int>(def->consumable.healAmount));
                DrawText(healBuf, static_cast<int>(inspRect.x + 12), static_cast<int>(inspRect.y + 120), UITheme::FontSmall, UITheme::TextSuccess);
            }

            // Action Buttons
            Rectangle useRect{ 420.0f, 245.0f, 75.0f, 22.0f };
            Rectangle drop1Rect{ 505.0f, 245.0f, 75.0f, 22.0f };
            Rectangle dropAllRect{ 505.0f, 275.0f, 75.0f, 22.0f };

            bool isConsumable = def && (def->category == items::ItemCategory::Consumable);
            bool isEquippable = def && (def->category == items::ItemCategory::Weapon ||
                                        def->category == items::ItemCategory::Armor ||
                                        def->category == items::ItemCategory::Clothing);

            const char* useLabel = isConsumable ? "USE" : (isEquippable ? "EQUIP" : "ACTION");

            // Use / Equip Button
            bool useHov = CheckHover(useRect);
            DrawRectangleRec(useRect, useHov ? UITheme::GoldHover : UITheme::Gold);
            int uw = MeasureText(useLabel, UITheme::FontSmall);
            DrawText(useLabel, static_cast<int>(useRect.x + (useRect.width - uw) * 0.5f), static_cast<int>(useRect.y + 5), UITheme::FontSmall, UITheme::DarkBgOpaque);

            // Drop 1 Button
            bool d1Hov = CheckHover(drop1Rect);
            DrawRectangleRec(drop1Rect, d1Hov ? UITheme::SlotHovered : UITheme::SlotEmpty);
            DrawRectangleLinesEx(drop1Rect, 1.0f, d1Hov ? UITheme::Gold : UITheme::BorderNormal);
            int d1w = MeasureText("DROP 1", UITheme::FontMicro);
            DrawText("DROP 1", static_cast<int>(drop1Rect.x + (drop1Rect.width - d1w) * 0.5f), static_cast<int>(drop1Rect.y + 6), UITheme::FontMicro, UITheme::TextLight);

            // Drop All Button
            bool daHov = CheckHover(dropAllRect);
            DrawRectangleRec(dropAllRect, daHov ? UITheme::HealthFill : UITheme::SlotEmpty);
            DrawRectangleLinesEx(dropAllRect, 1.0f, daHov ? UITheme::HealthFill : UITheme::BorderNormal);
            int daw = MeasureText("DROP ALL", UITheme::FontMicro);
            DrawText("DROP ALL", static_cast<int>(dropAllRect.x + (dropAllRect.width - daw) * 0.5f), static_cast<int>(dropAllRect.y + 6), UITheme::FontMicro, UITheme::TextLight);

        } else {
            DrawText("Select an item to inspect", static_cast<int>(inspRect.x + 20), static_cast<int>(inspRect.y + 110), UITheme::FontSmall, UITheme::TextDim);
        }
    }
}

} // namespace wildbound::ui
