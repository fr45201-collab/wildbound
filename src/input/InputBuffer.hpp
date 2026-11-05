#pragma once

#include "InputAction.hpp"
#include <array>
#include <cstdint>
#include <algorithm>

namespace wildbound::input {

struct BufferedAction {
    InputAction action = InputAction::Count;
    float timeRemaining = 0.0f;
    float value = 1.0f;
    bool consumed = true;
};

/**
 * Manages action-specific buffering to prevent dropped attack, interaction, or menu presses.
 */
class InputBuffer {
public:
    InputBuffer() {
        // Initialize default buffer timeouts
        m_bufferDurations.fill(0.0f); // Default: no buffer

        // Configure default action-specific buffer windows
        SetBufferDuration(InputAction::PrimaryAttack, 0.120f);   // 120 ms
        SetBufferDuration(InputAction::SecondaryAttack, 0.120f); // 120 ms
        SetBufferDuration(InputAction::Interact, 0.150f);        // 150 ms
        SetBufferDuration(InputAction::OpenInventory, 0.150f);   // 150 ms
        SetBufferDuration(InputAction::UIConfirm, 0.100f);       // 100 ms
        SetBufferDuration(InputAction::UICancel, 0.100f);        // 100 ms

        for (auto& item : m_buffer) {
            item.consumed = true;
            item.timeRemaining = 0.0f;
        }
    }

    void SetBufferDuration(InputAction action, float durationSeconds) noexcept {
        size_t idx = static_cast<size_t>(action);
        if (idx < ACTION_COUNT) {
            m_bufferDurations[idx] = (durationSeconds > 0.0f) ? durationSeconds : 0.0f;
        }
    }

    float GetBufferDuration(InputAction action) const noexcept {
        size_t idx = static_cast<size_t>(action);
        if (idx < ACTION_COUNT) {
            return m_bufferDurations[idx];
        }
        return 0.0f;
    }

    /**
     * Enqueues an action into the buffer if it has a non-zero buffer duration.
     */
    void BufferAction(InputAction action, float value = 1.0f) noexcept {
        size_t idx = static_cast<size_t>(action);
        if (idx >= ACTION_COUNT) return;

        float dur = m_bufferDurations[idx];
        if (dur <= 0.0f) return;

        m_buffer[idx].action = action;
        m_buffer[idx].timeRemaining = dur;
        m_buffer[idx].value = value;
        m_buffer[idx].consumed = false;
    }

    /**
     * Checks if a buffered action is available (active and unconsumed).
     */
    bool HasBufferedAction(InputAction action) const noexcept {
        size_t idx = static_cast<size_t>(action);
        if (idx >= ACTION_COUNT) return false;
        return !m_buffer[idx].consumed && m_buffer[idx].timeRemaining > 0.0f;
    }

    /**
     * Consumes a buffered action. Returns true if the action was available.
     */
    bool ConsumeAction(InputAction action) noexcept {
        size_t idx = static_cast<size_t>(action);
        if (idx >= ACTION_COUNT) return false;

        if (!m_buffer[idx].consumed && m_buffer[idx].timeRemaining > 0.0f) {
            m_buffer[idx].consumed = true;
            m_buffer[idx].timeRemaining = 0.0f;
            return true;
        }
        return false;
    }

    /**
     * Updates buffer expiration timers by delta time.
     */
    void Update(float dt) noexcept {
        if (dt <= 0.0f) return;
        for (auto& item : m_buffer) {
            if (!item.consumed && item.timeRemaining > 0.0f) {
                item.timeRemaining -= dt;
                if (item.timeRemaining <= 0.0f) {
                    item.timeRemaining = 0.0f;
                    item.consumed = true;
                }
            }
        }
    }

    /**
     * Clears all buffered actions. Typically invoked on context switches or focus loss.
     */
    void Clear() noexcept {
        for (auto& item : m_buffer) {
            item.consumed = true;
            item.timeRemaining = 0.0f;
        }
    }

    /**
     * Clears only actions of a specific category.
     */
    void ClearCategory(ActionCategory category) noexcept {
        for (size_t i = 0; i < ACTION_COUNT; ++i) {
            auto act = static_cast<InputAction>(i);
            if (GetActionCategory(act) == category) {
                m_buffer[i].consumed = true;
                m_buffer[i].timeRemaining = 0.0f;
            }
        }
    }

    const BufferedAction& GetBufferedState(InputAction action) const noexcept {
        static BufferedAction dummy;
        size_t idx = static_cast<size_t>(action);
        if (idx < ACTION_COUNT) return m_buffer[idx];
        return dummy;
    }

private:
    std::array<float, ACTION_COUNT> m_bufferDurations{};
    std::array<BufferedAction, ACTION_COUNT> m_buffer{};
};

} // namespace wildbound::input

