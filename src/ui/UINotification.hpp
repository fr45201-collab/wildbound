#pragma once

#include "UITheme.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>

namespace wildbound::ui {

struct NotificationToast {
    std::string message;
    std::string iconId;
    Color accentColor = UITheme::Gold;
    float duration = 3.5f;
    float elapsed = 0.0f;
    float alpha = 1.0f;
    float slideOffset = -40.0f; // Animates from -40 to 0 on entry
};

/**
 * Non-blocking HUD Toast Notification Queue.
 * Displays temporary gameplay announcements (item pickups, level alerts, match events)
 * with entry slide, persistence timer, and smooth alpha fadeout.
 */
class NotificationQueue {
public:
    NotificationQueue() = default;

    void Push(const std::string& message, const std::string& iconId = "", Color accent = UITheme::Gold, float duration = 3.5f) {
        if (m_toasts.size() >= MaxVisibleToasts) {
            m_toasts.pop_front();
        }
        NotificationToast toast;
        toast.message = message;
        toast.iconId = iconId;
        toast.accentColor = accent;
        toast.duration = duration > 0.5f ? duration : 3.5f;
        toast.elapsed = 0.0f;
        toast.alpha = 0.0f;
        toast.slideOffset = -25.0f;
        m_toasts.push_back(std::move(toast));
    }

    void Update(float dt) {
        for (auto it = m_toasts.begin(); it != m_toasts.end(); ) {
            it->elapsed += dt;

            // Animate entry slide and alpha
            constexpr float EntryDuration = 0.25f;
            if (it->elapsed < EntryDuration) {
                float t = it->elapsed / EntryDuration;
                it->slideOffset = -25.0f * (1.0f - t);
                it->alpha = t;
            } else {
                it->slideOffset = 0.0f;
                // Fade out near end of duration
                float remaining = it->duration - it->elapsed;
                constexpr float ExitDuration = 0.5f;
                if (remaining <= ExitDuration && remaining >= 0.0f) {
                    it->alpha = remaining / ExitDuration;
                } else if (remaining < 0.0f) {
                    it->alpha = 0.0f;
                } else {
                    it->alpha = 1.0f;
                }
            }

            if (it->elapsed >= it->duration) {
                it = m_toasts.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Render(Vector2 startPos = { 12.0f, 48.0f }) const {
        float currentY = startPos.y;
        for (const auto& toast : m_toasts) {
            if (toast.alpha <= 0.01f) continue;

            const float width = 160.0f;
            const float height = 22.0f;
            const float posX = startPos.x + toast.slideOffset;

            // Fade colors
            Color bg = UITheme::DarkBg;
            bg.a = static_cast<unsigned char>(bg.a * toast.alpha);

            Color border = toast.accentColor;
            border.a = static_cast<unsigned char>(border.a * toast.alpha);

            Color textCol = UITheme::TextLight;
            textCol.a = static_cast<unsigned char>(255 * toast.alpha);

            // Draw toast background
            DrawRectangle(static_cast<int>(posX), static_cast<int>(currentY), static_cast<int>(width), static_cast<int>(height), bg);
            // Left accent bar
            DrawRectangle(static_cast<int>(posX), static_cast<int>(currentY), 3, static_cast<int>(height), border);
            // Border outline
            DrawRectangleLines(static_cast<int>(posX), static_cast<int>(currentY), static_cast<int>(width), static_cast<int>(height), Color{ border.r, border.g, border.b, static_cast<unsigned char>(80 * toast.alpha) });

            // Text
            DrawText(toast.message.c_str(), static_cast<int>(posX + 8.0f), static_cast<int>(currentY + 5.0f), UITheme::FontSmall, textCol);

            currentY += height + 4.0f;
        }
    }

    void Clear() {
        m_toasts.clear();
    }

    [[nodiscard]] size_t GetCount() const { return m_toasts.size(); }
    [[nodiscard]] const std::deque<NotificationToast>& GetToasts() const { return m_toasts; }

private:
    static constexpr size_t MaxVisibleToasts = 5;
    std::deque<NotificationToast> m_toasts;
};

} // namespace wildbound::ui

