#pragma once

#include "../UITheme.hpp"
#include "../../network/http/BackendClient.hpp"
#include <raylib.h>
#include <string>
#include <functional>

namespace wildbound::ui {

enum class AuthMenuMode {
    Login,
    Register,
    ProfileProgression
};

class AuthMenu {
public:
    explicit AuthMenu(network::http::BackendClient* backendClient = nullptr);
    ~AuthMenu() = default;

    void SetBackendClient(network::http::BackendClient* client) noexcept { m_client = client; }

    bool Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed);
    void Render();

    void Open(AuthMenuMode mode = AuthMenuMode::Login);
    void Close() noexcept { m_isOpen = false; }
    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }

    void RefreshProgression();

private:
    network::http::BackendClient* m_client{ nullptr };
    bool m_isOpen{ false };
    AuthMenuMode m_mode{ AuthMenuMode::Login };

    // Input fields
    std::string m_username;
    std::string m_email;
    std::string m_password;
    std::string m_displayName;
    int m_activeField{ 0 }; // 0=none, 1=user/email, 2=password, 3=reg_email, 4=display_name

    // Status message
    std::string m_statusMessage;
    Color m_statusColor{ UITheme::TextLight };
    bool m_isBusy{ false };

    // Progression data cache
    int m_accountLevel{ 1 };
    int m_totalXP{ 0 };
    int m_currentLevelXP{ 0 };
    int m_nextLevelXP{ 100 };
    float m_progressPct{ 0.0f };
    int m_rewardPoints{ 0 };

    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed{ false };

    bool CheckHover(Rectangle r) const;
    void DrawButton(Rectangle r, const std::string& text, bool isHovered, bool isSelected = false);
    void HandleTextInput();
};

} // namespace wildbound::ui

