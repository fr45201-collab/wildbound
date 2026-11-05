#pragma once

#include "InputAction.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace wildbound::input {

enum class InputContextType : uint8_t {
    Gameplay,
    Menu,
    Inventory,
    Wardrobe,
    Settings,
    Paused,
    Count
};

inline std::string ContextTypeToString(InputContextType type) {
    switch (type) {
        case InputContextType::Gameplay:  return "Gameplay";
        case InputContextType::Menu:      return "Menu";
        case InputContextType::Inventory: return "Inventory";
        case InputContextType::Wardrobe:  return "Wardrobe";
        case InputContextType::Settings:  return "Settings";
        case InputContextType::Paused:    return "Paused";
        default:                          return "Unknown";
    }
}

/**
 * Definition of an input context with permitted action categories and modal blocking rules.
 */
struct InputContext {
    InputContextType type = InputContextType::Gameplay;
    std::string name = "Gameplay";
    int priority = 0;
    bool isModal = false; // When true, completely blocks input to contexts below it on the stack
    uint32_t allowedCategoryMask = 0;

    bool AllowsCategory(ActionCategory category) const noexcept {
        return (allowedCategoryMask & (1u << static_cast<uint32_t>(category))) != 0;
    }

    void AllowCategory(ActionCategory category, bool allow = true) noexcept {
        if (allow) {
            allowedCategoryMask |= (1u << static_cast<uint32_t>(category));
        } else {
            allowedCategoryMask &= ~(1u << static_cast<uint32_t>(category));
        }
    }
};

/**
 * Creates predefined configurations for the 6 core Wildbound contexts.
 */
inline InputContext CreateContextPreset(InputContextType type) {
    InputContext ctx;
    ctx.type = type;
    ctx.name = ContextTypeToString(type);

    switch (type) {
        case InputContextType::Gameplay:
            ctx.priority = 0;
            ctx.isModal = false;
            ctx.AllowCategory(ActionCategory::Movement);
            ctx.AllowCategory(ActionCategory::Combat);
            ctx.AllowCategory(ActionCategory::Inventory);
            ctx.AllowCategory(ActionCategory::Character);
            ctx.AllowCategory(ActionCategory::General);
            ctx.AllowCategory(ActionCategory::Debug);
            break;

        case InputContextType::Menu:
            ctx.priority = 10;
            ctx.isModal = true;
            ctx.AllowCategory(ActionCategory::UI);
            ctx.AllowCategory(ActionCategory::General);
            ctx.AllowCategory(ActionCategory::Debug);
            break;

        case InputContextType::Inventory:
            ctx.priority = 20;
            ctx.isModal = true;
            ctx.AllowCategory(ActionCategory::UI);
            ctx.AllowCategory(ActionCategory::Inventory);
            ctx.AllowCategory(ActionCategory::General);
            ctx.AllowCategory(ActionCategory::Debug);
            break;

        case InputContextType::Wardrobe:
            ctx.priority = 20;
            ctx.isModal = true;
            ctx.AllowCategory(ActionCategory::UI);
            ctx.AllowCategory(ActionCategory::Character);
            ctx.AllowCategory(ActionCategory::General);
            ctx.AllowCategory(ActionCategory::Debug);
            break;

        case InputContextType::Settings:
            ctx.priority = 30;
            ctx.isModal = true;
            ctx.AllowCategory(ActionCategory::UI);
            ctx.AllowCategory(ActionCategory::General);
            ctx.AllowCategory(ActionCategory::Debug);
            break;

        case InputContextType::Paused:
            ctx.priority = 40;
            ctx.isModal = true;
            ctx.AllowCategory(ActionCategory::UI);
            ctx.AllowCategory(ActionCategory::General);
            ctx.AllowCategory(ActionCategory::Debug);
            break;

        default:
            break;
    }
    return ctx;
}

/**
 * Manages the priority stack of input contexts.
 */
class InputContextManager {
public:
    InputContextManager() {
        // Initialize default gameplay context
        PushContext(InputContextType::Gameplay);
    }

    void PushContext(InputContextType type) {
        m_stack.push_back(CreateContextPreset(type));
    }

    void PopContext() {
        if (m_stack.size() > 1) { // Retain at least the root context
            m_stack.pop_back();
        }
    }

    void SetContext(InputContextType type) {
        m_stack.clear();
        m_stack.push_back(CreateContextPreset(type));
    }

    InputContextType GetActiveContextType() const noexcept {
        if (m_stack.empty()) return InputContextType::Gameplay;
        return m_stack.back().type;
    }

    const InputContext& GetActiveContext() const noexcept {
        static InputContext fallback = CreateContextPreset(InputContextType::Gameplay);
        if (m_stack.empty()) return fallback;
        return m_stack.back();
    }

    const std::vector<InputContext>& GetStack() const noexcept {
        return m_stack;
    }

    bool IsActionAllowed(InputAction action) const noexcept {
        if (m_stack.empty()) return true;

        ActionCategory cat = GetActionCategory(action);
        // If top context is modal, only top context decides
        const auto& top = m_stack.back();
        if (top.isModal) {
            return top.AllowsCategory(cat);
        }

        // Otherwise check from top to bottom
        for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it) {
            if (it->AllowsCategory(cat)) return true;
            if (it->isModal) break;
        }
        return false;
    }

    size_t GetStackDepth() const noexcept {
        return m_stack.size();
    }

private:
    std::vector<InputContext> m_stack;
};

} // namespace wildbound::input

