#pragma once

#include "../UITheme.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <optional>

namespace wildbound::entities {
    class Player;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::world {
    class World;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::ui {

/**
 * Transactional 24-Slot Inventory and Equipment Interface.
 * Handles slot navigation, category filtering, detailed item inspection,
 * item consumption, equipment toggling, stack splitting, and ground dropping.
 */
class InventoryUI {
public:
    InventoryUI();
    ~InventoryUI() = default;

    void Reset();

    void Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed, bool mouseLeftDown,
                entities::Player* player = nullptr,
                const items::ItemManager* itemMgr = nullptr,
                world::World* world = nullptr,
                audio::AudioSystem* audioSys = nullptr,
                bool navUp = false, bool navDown = false, bool navLeft = false, bool navRight = false,
                bool navConfirm = false, bool navDrop = false);

    void Render(resources::AssetManager& assetMgr,
                entities::Player* player = nullptr,
                const items::ItemManager* itemMgr = nullptr,
                world::World* world = nullptr);

    [[nodiscard]] int GetSelectedSlot() const { return m_selectedSlot; }
    void SetSelectedSlot(int slot) { m_selectedSlot = slot; }

    [[nodiscard]] int GetCategoryTab() const { return m_categoryTab; }
    void SetCategoryTab(int tab) { m_categoryTab = tab; }

    [[nodiscard]] bool RequestClose() const { return m_requestClose; }
    void ClearRequestClose() { m_requestClose = false; }

private:
    bool CheckHover(Rectangle r) const;

    int m_selectedSlot = 0;
    int m_hoveredSlot = -1;
    int m_categoryTab = 0; // 0=All, 1=Consumable, 2=Equipment, 3=Materials, 4=Resources, 5=Special
    bool m_requestClose = false;

    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed = false;
    bool m_mouseDown = false;

    static constexpr int GridCols = 6;
    static constexpr int GridRows = 4;
    static constexpr int TotalSlots = 24;
};

} // namespace wildbound::ui
