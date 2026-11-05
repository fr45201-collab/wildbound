#include "UIManager.hpp"
#include "../input/InputManager.hpp"
#include "../interaction/InteractionPrompt.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../resources/TextureManager.hpp"
#include "../entities/Player.hpp"
#include "../weapons/WeaponData.hpp"
#include "../weapons/WeaponInstance.hpp"
#include "../items/ItemManager.hpp"
#include "../world/World.hpp"
#include "../world/WorldItem.hpp"
#include "../audio/AudioSystem.hpp"
#include "../customization/CharacterCustomizationManager.hpp"
#include "../cosmetics/CosmeticsStoreManager.hpp"
#include "../cosmetics/CosmeticsRarity.hpp"
#include "../core/Logger.hpp"
#include <algorithm>

using wildbound::core::Logger;

namespace wildbound::ui {

UIManager::UIManager() = default;

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Initialize(resources::AssetManager& assetMgr, audio::AudioSystem* audioSys) {
    (void)assetMgr;
    if (m_initialized) return true;

    Logger::Info("UI_MGR", "Initializing Player-Facing UI Management System...");
    m_audioSystem = audioSys;

    m_hudManager.Reset();
    m_inventoryUI.Reset();

    m_initialized = true;
    return true;
}

void UIManager::Shutdown() {
    if (!m_initialized) return;
    Logger::Info("UI_MGR", "Shutting down UIManager...");
    m_initialized = false;
}

void UIManager::SetScreen(UIScreen screen) {
    if (m_currentScreen != screen) {
        m_previousScreen = m_currentScreen;
        m_currentScreen = screen;

        if (m_audioSystem) {
            if (screen == UIScreen::MainMenu) {
                m_audioSystem->PlayMusic("mus_main_menu", true, 0.8f);
            } else if (screen == UIScreen::None) {
                m_audioSystem->SetBiome("forest");
            }
        }
    }
}

bool UIManager::CheckHover(Rectangle r) const {
    return ::CheckCollisionPointRec(m_mousePos, r);
}

void UIManager::Update(float dt, const input::InputManager* inputMgr) {
    if (inputMgr) {
        Update(dt,
               inputMgr->GetMouseVirtualPosition(),
               inputMgr->WasActionPressed(input::InputAction::PrimaryAttack) || inputMgr->WasActionPressed(input::InputAction::UIConfirm),
               inputMgr->IsActionHeld(input::InputAction::PrimaryAttack),
               inputMgr);
    }
}

void UIManager::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed, bool mouseLeftDown, const input::InputManager* inputMgr) {
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;
    m_mouseDown = mouseLeftDown;

    bool navUp = inputMgr ? inputMgr->WasActionPressed(input::InputAction::MoveUp) : (::IsKeyPressed(KEY_UP) || ::IsKeyPressed(KEY_W));
    bool navDown = inputMgr ? inputMgr->WasActionPressed(input::InputAction::MoveDown) : (::IsKeyPressed(KEY_DOWN) || ::IsKeyPressed(KEY_S));
    bool navLeft = inputMgr ? inputMgr->WasActionPressed(input::InputAction::MoveLeft) : (::IsKeyPressed(KEY_LEFT) || ::IsKeyPressed(KEY_A));
    bool navRight = inputMgr ? inputMgr->WasActionPressed(input::InputAction::MoveRight) : (::IsKeyPressed(KEY_RIGHT) || ::IsKeyPressed(KEY_D));
    bool navConfirm = inputMgr ? inputMgr->WasActionPressed(input::InputAction::UIConfirm) : (::IsKeyPressed(KEY_ENTER) || ::IsKeyPressed(KEY_SPACE));
    bool navCancel = inputMgr ? (inputMgr->WasActionPressed(input::InputAction::Pause) || inputMgr->WasActionPressed(input::InputAction::UICancel)) : ::IsKeyPressed(KEY_ESCAPE);
    bool navDrop = ::IsKeyPressed(KEY_Q);

    // Screen-specific updates
    if (m_currentScreen == UIScreen::MainMenu) {
        MainMenuAction act = m_mainMenu.Update(dt, mouseVirtual, mouseLeftPressed, navUp, navDown, navConfirm);
        if (act == MainMenuAction::PlayGame) {
            SetScreen(UIScreen::None);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == MainMenuAction::CharacterSelect) {
            SetScreen(UIScreen::CharacterSelect);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == MainMenuAction::Wardrobe) {
            SetScreen(UIScreen::Wardrobe);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == MainMenuAction::CosmeticsStore) {
            SetScreen(UIScreen::Store);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == MainMenuAction::Settings) {
            SetScreen(UIScreen::Settings);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == MainMenuAction::ExitGame) {
            ::CloseWindow();
        }
        return;
    }

    if (m_currentScreen == UIScreen::PauseMenu) {
        PauseMenuAction act = m_pauseMenu.Update(dt, mouseVirtual, mouseLeftPressed, navUp, navDown, navConfirm);
        if (act == PauseMenuAction::Resume) {
            SetScreen(UIScreen::None);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelClose");
        } else if (act == PauseMenuAction::Inventory) {
            SetScreen(UIScreen::Inventory);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
        } else if (act == PauseMenuAction::Wardrobe) {
            SetScreen(UIScreen::Wardrobe);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
        } else if (act == PauseMenuAction::Settings) {
            SetScreen(UIScreen::Settings);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == PauseMenuAction::MainMenu) {
            SetScreen(UIScreen::MainMenu);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (act == PauseMenuAction::Quit) {
            SetScreen(UIScreen::MainMenu);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        } else if (navCancel) {
            SetScreen(UIScreen::None);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelClose");
        }
        return;
    }

    if (m_currentScreen == UIScreen::Settings) {
        m_settingsMenu.Update(dt, mouseVirtual, mouseLeftPressed, mouseLeftDown,
                              const_cast<input::InputManager*>(inputMgr), m_audioSystem);
        if (m_settingsMenu.RequestBack() || navCancel) {
            m_settingsMenu.ClearRequestBack();
            SetScreen(m_previousScreen != UIScreen::Settings ? m_previousScreen : UIScreen::PauseMenu);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.Back");
        }
        return;
    }

    if (m_currentScreen == UIScreen::Inventory) {
        m_inventoryUI.Update(dt, mouseVirtual, mouseLeftPressed, mouseLeftDown,
                             m_lastPlayer, m_lastItemMgr, m_lastWorld, m_audioSystem,
                             navUp, navDown, navLeft, navRight, navConfirm, navDrop);
        if (m_inventoryUI.RequestClose() || navCancel) {
            m_inventoryUI.ClearRequestClose();
            SetScreen(UIScreen::None);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelClose");
        }
        return;
    }

    if (m_currentScreen == UIScreen::None) {
        m_hudManager.Update(dt, m_lastPlayer);

        if (navCancel) {
            SetScreen(UIScreen::PauseMenu);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
            return;
        }

        if ((inputMgr && inputMgr->WasActionPressed(input::InputAction::OpenInventory)) || ::IsKeyPressed(KEY_I) || ::IsKeyPressed(KEY_TAB)) {
            SetScreen(UIScreen::Inventory);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
            return;
        }

        if ((inputMgr && inputMgr->WasActionPressed(input::InputAction::OpenWardrobe)) || ::IsKeyPressed(KEY_C) || ::IsKeyPressed(KEY_K)) {
            SetScreen(UIScreen::Wardrobe);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.PanelOpen");
            return;
        }
    }
}

// ============================================================================
// UI Component Primitives
// ============================================================================

void UIManager::DrawPanel(Rectangle bounds, const std::string& style) {
    Color bgFill = (style == "dark") ? Color{ 10, 12, 18, 240 } : Color{ 18, 22, 34, 235 };
    Color border = (style == "dark") ? Color{ 40, 48, 65, 255 } : Color{ 180, 145, 65, 255 };

    // Fill
    ::DrawRectangleRec(bounds, bgFill);

    // Bevel borders
    ::DrawRectangleLinesEx(bounds, 2.0f, border);

    // Highlight top/left
    ::DrawLine(static_cast<int>(bounds.x + 2), static_cast<int>(bounds.y + 2),
               static_cast<int>(bounds.x + bounds.width - 2), static_cast<int>(bounds.y + 2),
               Color{ 240, 215, 120, 220 });
    ::DrawLine(static_cast<int>(bounds.x + 2), static_cast<int>(bounds.y + 2),
               static_cast<int>(bounds.x + 2), static_cast<int>(bounds.y + bounds.height - 2),
               Color{ 240, 215, 120, 220 });

    // Gold corner studs
    for (float cx : { bounds.x + 1, bounds.x + bounds.width - 4 }) {
        for (float cy : { bounds.y + 1, bounds.y + bounds.height - 4 }) {
            ::DrawRectangle(static_cast<int>(cx), static_cast<int>(cy), 3, 3, Color{ 255, 230, 140, 255 });
        }
    }
}

bool UIManager::DrawButton(Rectangle bounds, const std::string& text, const std::string& style, bool selected) {
    bool hovered = CheckHover(bounds);
    bool pressed = hovered && m_mouseDown;
    bool clicked = hovered && m_mousePressed;

    if (hovered && m_lastHoveredId != text) {
        m_lastHoveredId = text;
        if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonHover");
    }

    Color fillCol = Color{ 32, 40, 60, 255 };
    Color borderCol = Color{ 190, 155, 75, 255 };
    Color textCol = Color{ 245, 245, 255, 255 };

    if (style == "destructive") {
        fillCol = Color{ 60, 25, 30, 255 };
        borderCol = Color{ 200, 60, 70, 255 };
    } else if (style == "confirm") {
        fillCol = Color{ 25, 60, 35, 255 };
        borderCol = Color{ 70, 195, 105, 255 };
    } else if (style == "tab") {
        fillCol = selected ? Color{ 55, 70, 105, 255 } : Color{ 22, 28, 42, 255 };
        borderCol = selected ? Color{ 255, 225, 110, 255 } : Color{ 100, 110, 130, 255 };
    }

    if (hovered && !pressed) {
        fillCol.r = static_cast<unsigned char>(std::min(255, fillCol.r + 25));
        fillCol.g = static_cast<unsigned char>(std::min(255, fillCol.g + 25));
        fillCol.b = static_cast<unsigned char>(std::min(255, fillCol.b + 25));
        borderCol = Color{ 255, 230, 140, 255 };
    } else if (pressed) {
        fillCol.r = static_cast<unsigned char>(std::max(0, fillCol.r - 20));
        fillCol.g = static_cast<unsigned char>(std::max(0, fillCol.g - 20));
        fillCol.b = static_cast<unsigned char>(std::max(0, fillCol.b - 20));
    }

    ::DrawRectangleRec(bounds, fillCol);
    ::DrawRectangleLinesEx(bounds, selected ? 2.0f : 1.5f, borderCol);

    // Centered label
    int fontSize = (bounds.height >= 26) ? 14 : 11;
    int textW = ::MeasureText(text.c_str(), fontSize);
    int tx = static_cast<int>(bounds.x + (bounds.width - textW) * 0.5f);
    int ty = static_cast<int>(bounds.y + (bounds.height - fontSize) * 0.5f);
    if (pressed) { tx += 1; ty += 1; }

    ::DrawText(text.c_str(), tx, ty, fontSize, textCol);

    if (clicked && m_audioSystem) {
        m_audioSystem->DispatchEvent("UI.ButtonClick");
    }

    return clicked;
}

void UIManager::DrawProgressBar(Rectangle bounds, float progress, Color fillColor, Color bgColor) {
    float clamped = std::clamp(progress, 0.0f, 1.0f);
    ::DrawRectangleRec(bounds, bgColor);
    ::DrawRectangleLinesEx(bounds, 1.0f, Color{ 140, 120, 70, 255 });

    Rectangle fillRect = { bounds.x + 1, bounds.y + 1, (bounds.width - 2) * clamped, bounds.height - 2 };
    ::DrawRectangleRec(fillRect, fillColor);
}

void UIManager::DrawIcon(const std::string& iconId, Vector2 pos, float size, Color tint) {
    (void)tint;
    // Stylized procedural vector icon rendering for crisp pixel fidelity
    if (iconId == "health") {
        ::DrawRectangle(static_cast<int>(pos.x), static_cast<int>(pos.y + 3), static_cast<int>(size), static_cast<int>(size - 6), Color{ 230, 45, 55, 255 });
        ::DrawRectangle(static_cast<int>(pos.x + 3), static_cast<int>(pos.y), static_cast<int>(size - 6), static_cast<int>(size), Color{ 230, 45, 55, 255 });
    } else if (iconId == "stamina") {
        ::DrawTriangle(Vector2{ pos.x + size * 0.6f, pos.y },
                       Vector2{ pos.x + size * 0.2f, pos.y + size * 0.5f },
                       Vector2{ pos.x + size * 0.5f, pos.y + size * 0.5f }, Color{ 245, 205, 60, 255 });
        ::DrawTriangle(Vector2{ pos.x + size * 0.5f, pos.y + size * 0.45f },
                       Vector2{ pos.x + size * 0.4f, pos.y + size },
                       Vector2{ pos.x + size * 0.8f, pos.y + size * 0.45f }, Color{ 245, 205, 60, 255 });
    } else {
        ::DrawRectangleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(size), static_cast<int>(size), Color{ 220, 185, 90, 255 });
    }
}

// ============================================================================
// Screen Rendering
// ============================================================================

void UIManager::Render(graphics::Renderer& renderer,
                      resources::AssetManager& assetMgr,
                      entities::Player* player,
                      const items::ItemManager* itemMgr,
                      const interaction::InteractionPrompt* prompt,
                      world::World* world,
                      customization::CharacterCustomizationManager* customMgr,
                      cosmetics::CosmeticsStoreManager* storeMgr)
{
    m_lastPlayer = player;
    m_lastItemMgr = itemMgr;
    m_lastWorld = world;

    const auto* activePrompt = prompt ? prompt : m_interactionPrompt;

    switch (m_currentScreen) {
        case UIScreen::None:
            RenderGameplayHUD(renderer, assetMgr, player, activePrompt);
            break;
        case UIScreen::MainMenu:
            RenderMainMenu(renderer, assetMgr);
            break;
        case UIScreen::CharacterSelect:
            RenderCharacterSelect(renderer, assetMgr, player, customMgr);
            break;
        case UIScreen::Wardrobe:
            RenderWardrobe(renderer, assetMgr, player, customMgr, storeMgr);
            break;
        case UIScreen::Store:
            RenderStore(renderer, assetMgr, player, customMgr, storeMgr);
            break;
        case UIScreen::Inventory:
            RenderInventory(renderer, assetMgr, itemMgr, player, world);
            break;
        case UIScreen::Settings:
            RenderSettings(renderer, assetMgr);
            break;
        case UIScreen::PauseMenu:
            RenderGameplayHUD(renderer, assetMgr, player, activePrompt);
            RenderPauseMenu(renderer, assetMgr);
            break;
        case UIScreen::MatchResults:
            RenderMatchResults(renderer, assetMgr);
            break;
        case UIScreen::GameOver:
            RenderGameOver(renderer, assetMgr);
            break;
    }
}

void UIManager::RenderMainMenu(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    (void)assetMgr;
    m_mainMenu.Render();
}

void UIManager::RenderCharacterSelect(graphics::Renderer& renderer, resources::AssetManager& assetMgr, entities::Player* player, customization::CharacterCustomizationManager* customMgr) {
    ::DrawRectangleGradientV(0, 0, 640, 360, Color{ 24, 28, 42, 255 }, Color{ 14, 16, 24, 255 });

    DrawPanel(Rectangle{ 40, 30, 560, 300 }, "main");
    ::DrawText("CHARACTER SELECTION", 220, 45, 18, Color{ 245, 215, 95, 255 });

    // Pedestal preview panel
    DrawPanel(Rectangle{ 220, 85, 200, 180 }, "dark");
    ::DrawText("Adventurer Pedestal", 260, 100, 13, Color{ 180, 190, 210, 255 });

    // Render character preview
    if (player) {
        player->GetModularAppearance().Render(
            renderer, assetMgr, Vector2{ 320, 220 },
            graphics::animation::Direction::Down,
            player->GetAnimController()
        );
    }

    if (DrawButton(Rectangle{ 170, 160, 40, 30 }, "<", "primary")) {
        if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
    }
    if (DrawButton(Rectangle{ 430, 160, 40, 30 }, ">", "primary")) {
        if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
    }

    if (DrawButton(Rectangle{ 240, 280, 160, 30 }, "CONFIRM SELECT", "confirm")) {
        SetScreen(UIScreen::None);
    }
    if (DrawButton(Rectangle{ 50, 42, 60, 24 }, "BACK", "primary")) {
        SetScreen(UIScreen::MainMenu);
    }
}

void UIManager::RenderWardrobe(graphics::Renderer& renderer, resources::AssetManager& assetMgr, entities::Player* player, customization::CharacterCustomizationManager* customMgr, cosmetics::CosmeticsStoreManager* storeMgr) {
    ::DrawRectangleGradientV(0, 0, 640, 360, Color{ 35, 28, 24, 255 }, Color{ 18, 14, 12, 255 });

    DrawPanel(Rectangle{ 20, 15, 600, 330 }, "main");
    ::DrawText("CHARACTER CUSTOMIZATION & WARDROBE", 155, 25, 16, Color{ 245, 215, 95, 255 });

    // Initialize preview session if not active
    if (customMgr && !customMgr->GetPreview().IsActive()) {
        customMgr->BeginPreview();
    }

    // Category Tabs: Hair, Head, Helm, Face, Chest, Legs, Feet, Back, Skin, Loadouts
    const std::vector<std::string> tabs = { "Hair", "Head", "Helm", "Face", "Chest", "Legs", "Feet", "Back", "Skin", "Loadouts" };
    for (size_t i = 0; i < tabs.size(); ++i) {
        Rectangle tabRect{ 28.0f + i * 58.0f, 52.0f, 54.0f, 22.0f };
        if (DrawButton(tabRect, tabs[i], "tab", m_wardrobeCategoryTab == static_cast<int>(i))) {
            m_wardrobeCategoryTab = static_cast<int>(i);
            m_wardrobeScrollOffset = 0;
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.TabSwitch");
        }
    }

    // Left List Container
    DrawPanel(Rectangle{ 28, 80, 350, 252 }, "dark");

    if (customMgr && m_wardrobeCategoryTab == 9) {
        // Loadouts Tab
        ::DrawText("SAVED APPEARANCE LOADOUTS", 40, 90, 13, Color{ 220, 200, 120, 255 });

        const auto& loadouts = customMgr->GetLoadouts();
        float y = 112.0f;
        for (size_t i = 0; i < loadouts.size(); ++i) {
            std::string nameLabel = std::to_string(i + 1) + ". " + loadouts[i].name;
            ::DrawText(nameLabel.c_str(), 42, static_cast<int>(y + 4), 11, Color{ 240, 240, 240, 255 });

            if (DrawButton(Rectangle{ 190, y, 42, 20 }, "LOAD", "confirm")) {
                customMgr->LoadLoadout(i);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
            }
            if (DrawButton(Rectangle{ 236, y, 42, 20 }, "SAVE", "primary")) {
                customMgr->SaveCurrentToLoadout(i);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
            }
            if (DrawButton(Rectangle{ 282, y, 42, 20 }, "COPY", "primary")) {
                customMgr->DuplicateLoadout(i);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
            }
            if (DrawButton(Rectangle{ 328, y, 42, 20 }, "DEL", "destructive")) {
                customMgr->DeleteLoadout(i);
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
            }

            y += 26.0f;
        }

        if (loadouts.size() < customMgr->GetMaxLoadouts()) {
            if (DrawButton(Rectangle{ 42, y + 8, 180, 22 }, "+ CREATE NEW LOADOUT", "primary")) {
                customMgr->CreateLoadout("Loadout " + std::to_string(loadouts.size() + 1));
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
            }
        }
    } else if (customMgr) {
        // Cosmetics Category Tab
        customization::CustomizationSlot currentSlot = customization::CustomizationSlot::Hair;
        switch (m_wardrobeCategoryTab) {
            case 0: currentSlot = customization::CustomizationSlot::Hair; break;
            case 1: currentSlot = customization::CustomizationSlot::Headwear; break;
            case 2: currentSlot = customization::CustomizationSlot::Helmet; break;
            case 3: currentSlot = customization::CustomizationSlot::Face; break;
            case 4: currentSlot = customization::CustomizationSlot::Chest; break;
            case 5: currentSlot = customization::CustomizationSlot::Legs; break;
            case 6: currentSlot = customization::CustomizationSlot::Feet; break;
            case 7: currentSlot = customization::CustomizationSlot::Back; break;
            case 8: currentSlot = customization::CustomizationSlot::Skin; break;
            default: break;
        }

        auto cosmetics = customMgr->GetAvailableCosmetics(currentSlot);

        std::string slotTitle = std::string(customization::CustomizationSlotToString(currentSlot)) + " (" + std::to_string(cosmetics.size()) + ")";
        ::DrawText(slotTitle.c_str(), 40, 88, 12, Color{ 220, 200, 120, 255 });

        if (DrawButton(Rectangle{ 260, 86, 105, 18 }, "UNEQUIP SLOT", "destructive")) {
            customMgr->UnequipPreviewSlot(currentSlot);
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        }

        int maxVisible = 7;
        if (cosmetics.size() > static_cast<size_t>(maxVisible)) {
            if (m_wardrobeScrollOffset > 0) {
                if (DrawButton(Rectangle{ 345, 110, 25, 20 }, "^", "primary")) {
                    m_wardrobeScrollOffset = std::max(0, m_wardrobeScrollOffset - 1);
                }
            }
            if (m_wardrobeScrollOffset + maxVisible < static_cast<int>(cosmetics.size())) {
                if (DrawButton(Rectangle{ 345, 290, 25, 20 }, "v", "primary")) {
                    m_wardrobeScrollOffset = std::min(static_cast<int>(cosmetics.size()) - maxVisible, m_wardrobeScrollOffset + 1);
                }
            }
        }

        float itemY = 110.0f;
        std::string equippedId = customMgr->GetPreview().GetPreviewAppearance().GetCosmetic(currentSlot);

        for (int i = m_wardrobeScrollOffset; i < static_cast<int>(cosmetics.size()) && i < m_wardrobeScrollOffset + maxVisible; ++i) {
            const auto* item = cosmetics[i];
            bool isEquipped = (equippedId == item->id);

            Rectangle btnRect{ 40, itemY, 300, 24 };
            std::string label = item->displayName;
            bool isOwned = storeMgr ? storeMgr->GetOwnershipManager().IsOwned(item->id) : true;
            if (isEquipped) {
                label += " [EQUIPPED]";
            } else if (!isOwned) {
                label += " [LOCKED]";
            } else {
                label += " [OWNED]";
            }

            if (DrawButton(btnRect, label, isEquipped ? "confirm" : "primary", isEquipped)) {
                if (storeMgr) {
                    storeMgr->SelectCosmetic(item->id);
                } else {
                    customMgr->ApplyPreviewCosmetic(item->id);
                }
                if (m_audioSystem) m_audioSystem->DispatchEvent("UI.CosmeticApply");
            }

            itemY += 27.0f;
        }
    }

    // Right Character Display Box
    DrawPanel(Rectangle{ 388, 80, 222, 252 }, "dark");
    ::DrawText("CHARACTER PREVIEW", 440, 90, 12, Color{ 220, 200, 120, 255 });

    if (customMgr && customMgr->GetPreview().IsActive()) {
        auto& preview = customMgr->GetPreview();

        if (DrawButton(Rectangle{ 400, 108, 28, 22 }, "<", "primary")) {
            preview.RotateCounterClockwise();
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        }

        std::string dirStr = std::string(graphics::animation::DirectionToString(preview.GetDirection()));
        ::DrawText(dirStr.c_str(), 480 - (::MeasureText(dirStr.c_str(), 11) / 2), 112, 11, Color{ 240, 240, 240, 255 });

        if (DrawButton(Rectangle{ 562, 108, 28, 22 }, ">", "primary")) {
            preview.RotateClockwise();
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        }

        std::string animBtnText = preview.IsWalkingAnimation() ? "ANIM: WALK" : "ANIM: IDLE";
        if (DrawButton(Rectangle{ 455, 134, 88, 20 }, animBtnText, "tab", preview.IsWalkingAnimation())) {
            preview.ToggleAnimationMode();
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        }

        preview.Render(renderer, assetMgr, Vector2{ 500, 200 });

        if (storeMgr) {
            const auto* selDef = storeMgr->GetSelectedCosmetic();
            if (selDef) {
                bool isOwned = storeMgr->IsSelectedOwned();
                bool isEquipped = storeMgr->IsSelectedEquipped();
                if (!isOwned) {
                    if (DrawButton(Rectangle{ 400, 264, 95, 22 }, "PREVIEW", "primary")) {
                        storeMgr->PreviewSelected();
                    }
                    if (DrawButton(Rectangle{ 502, 264, 100, 22 }, "UNLOCK (PROT)", "confirm")) {
                        storeMgr->UnlockPrototypeSelected();
                    }
                } else if (!isEquipped) {
                    if (DrawButton(Rectangle{ 400, 264, 202, 22 }, "EQUIP", "confirm")) {
                        storeMgr->EquipSelected();
                    }
                } else {
                    if (DrawButton(Rectangle{ 400, 264, 202, 22 }, "UNEQUIP", "destructive")) {
                        storeMgr->UnequipSlot(selDef->slot);
                    }
                }
            }
        }

        if (DrawButton(Rectangle{ 396, 296, 68, 24 }, "CONFIRM", "confirm")) {
            customMgr->ConfirmPreview();
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonConfirm");
            SetScreen(m_previousScreen);
        }

        if (DrawButton(Rectangle{ 468, 296, 62, 24 }, "RESET", "primary")) {
            customMgr->ResetPreview();
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.ButtonClick");
        }

        if (DrawButton(Rectangle{ 534, 296, 68, 24 }, "CANCEL", "destructive")) {
            customMgr->CancelPreview();
            if (m_audioSystem) m_audioSystem->DispatchEvent("UI.Back");
            SetScreen(m_previousScreen);
        }
    } else if (player) {
        player->GetModularAppearance().Render(
            renderer, assetMgr, Vector2{ 500, 218 },
            graphics::animation::Direction::Down,
            player->GetAnimController()
        );
    }

    if (DrawButton(Rectangle{ 28, 25, 54, 22 }, "BACK", "primary")) {
        if (customMgr && customMgr->GetPreview().IsActive()) {
            customMgr->CancelPreview();
        }
        SetScreen(UIScreen::MainMenu);
    }
}


void UIManager::RenderStore(graphics::Renderer& renderer, resources::AssetManager& assetMgr, entities::Player* player, customization::CharacterCustomizationManager* customMgr, cosmetics::CosmeticsStoreManager* storeMgr) {
    if (storeMgr) {
        storeMgr->SetActiveTab(cosmetics::StoreTab::Store);
    }
    RenderWardrobe(renderer, assetMgr, player, customMgr, storeMgr);
}

void UIManager::RenderInventory(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const items::ItemManager* itemMgr, entities::Player* player, world::World* world) {
    (void)renderer;
    m_inventoryUI.Render(assetMgr, player, itemMgr, world);
}

void UIManager::RenderSettings(graphics::Renderer& renderer, resources::AssetManager& assetMgr, input::InputManager* inputMgr) {
    (void)renderer;
    (void)assetMgr;
    m_settingsMenu.Render(m_audioSystem, inputMgr);
}

void UIManager::RenderPauseMenu(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    (void)assetMgr;
    m_pauseMenu.Render();
}

void UIManager::RenderGameplayHUD(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const entities::Player* player, const interaction::InteractionPrompt* prompt) {
    (void)renderer;
    m_hudManager.Render(assetMgr, player, m_lastWorld, m_lastItemMgr, prompt);
}

void UIManager::RenderMatchResults(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    (void)assetMgr;
    ::DrawRectangleGradientV(0, 0, 640, 360, Color{ 28, 24, 38, 255 }, Color{ 14, 12, 18, 255 });
    DrawPanel(Rectangle{ 120, 40, 400, 280 }, "main");
    ::DrawText("MATCH RESULTS", 245, 60, 20, Color{ 255, 215, 80, 255 });
    ::DrawText("VICTORY ACHIEVED", 235, 95, 16, Color{ 80, 235, 120, 255 });

    DrawPanel(Rectangle{ 150, 130, 340, 110 }, "dark");
    ::DrawText("Eliminations: 5", 170, 145, 12, WHITE);
    ::DrawText("Damage Dealt: 1,450", 170, 165, 12, WHITE);
    ::DrawText("Surviving Time: 12:45", 170, 185, 12, WHITE);
    ::DrawText("Rank: #1 Champion", 170, 205, 12, Color{ 255, 215, 80, 255 });

    if (DrawButton(Rectangle{ 240, 265, 160, 30 }, "RETURN TO MENU", "confirm")) {
        SetScreen(UIScreen::MainMenu);
    }
}

void UIManager::RenderGameOver(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    (void)assetMgr;
    ::DrawRectangleGradientV(0, 0, 640, 360, Color{ 35, 12, 16, 255 }, Color{ 15, 6, 8, 255 });
    DrawPanel(Rectangle{ 140, 60, 360, 240 }, "main");
    ::DrawText("DEFEAT", 270, 80, 24, Color{ 240, 50, 60, 255 });
    ::DrawText("You were eliminated in the wild.", 215, 120, 13, Color{ 200, 180, 180, 255 });

    if (DrawButton(Rectangle{ 240, 180, 160, 28 }, "TRY AGAIN", "confirm")) {
        SetScreen(UIScreen::None);
    }
    if (DrawButton(Rectangle{ 240, 220, 160, 28 }, "MAIN MENU", "primary")) {
        SetScreen(UIScreen::MainMenu);
    }
}

} // namespace wildbound::ui
