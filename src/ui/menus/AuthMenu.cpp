#include "AuthMenu.hpp"
#include <iostream>

namespace wildbound::ui {

AuthMenu::AuthMenu(network::http::BackendClient* backendClient)
    : m_client(backendClient) {
}

void AuthMenu::Open(AuthMenuMode mode) {
    m_isOpen = true;
    m_mode = mode;
    m_statusMessage.clear();
    m_activeField = 1;

    if (m_client && m_client->IsLoggedIn()) {
        m_mode = AuthMenuMode::ProfileProgression;
        RefreshProgression();
    }
}

void AuthMenu::RefreshProgression() {
    if (!m_client || !m_client->IsLoggedIn()) return;
    m_isBusy = true;
    std::weak_ptr<bool> weakAlive = m_alive;
    m_client->GetProgression([this, weakAlive](bool success, const nlohmann::json& prog, const std::string& err) {
        auto alive = weakAlive.lock();
        if (!alive || !*alive) return;
        m_isBusy = false;
        if (success) {
            m_accountLevel = prog.value("account_level", 1);
            m_totalXP = prog.value("total_xp", 0);
            m_currentLevelXP = prog.value("current_level_xp", 0);
            m_nextLevelXP = prog.value("next_level_xp", 100);
            m_progressPct = prog.value("progress_percentage", 0.0f);
            m_rewardPoints = prog.value("available_reward_points", 0);
        } else {
            m_statusMessage = err.empty() ? "Failed to load progression" : err;
            m_statusColor = UITheme::TextDanger;
        }
    });
}

bool AuthMenu::CheckHover(Rectangle r) const {
    return CheckCollisionPointRec(m_mousePos, r);
}

void AuthMenu::DrawButton(Rectangle r, const std::string& text, bool isHovered, bool isSelected) {
    Color bg = isSelected ? UITheme::SlotSelected : (isHovered ? UITheme::SlotHovered : UITheme::SlotFilled);
    Color border = isSelected ? UITheme::Gold : (isHovered ? UITheme::BorderGold : UITheme::BorderNormal);
    Color txtColor = isSelected ? UITheme::Gold : (isHovered ? UITheme::TextLight : UITheme::TextMuted);

    DrawRectangleRounded(r, 0.2f, 4, bg);
    DrawRectangleRoundedLines(r, 0.2f, 4, border);

    int textW = MeasureText(text.c_str(), 10);
    int posX = static_cast<int>(r.x + (r.width - textW) * 0.5f);
    int posY = static_cast<int>(r.y + (r.height - 10) * 0.5f);
    DrawText(text.c_str(), posX, posY, 10, txtColor);
}

void AuthMenu::HandleTextInput() {
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 126)) {
            switch (m_activeField) {
                case 1: if (m_username.length() < 32) m_username += static_cast<char>(key); break;
                case 2: if (m_password.length() < 64) m_password += static_cast<char>(key); break;
                case 3: if (m_email.length() < 64) m_email += static_cast<char>(key); break;
                case 4: if (m_displayName.length() < 32) m_displayName += static_cast<char>(key); break;
                default: break;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        switch (m_activeField) {
            case 1: if (!m_username.empty()) m_username.pop_back(); break;
            case 2: if (!m_password.empty()) m_password.pop_back(); break;
            case 3: if (!m_email.empty()) m_email.pop_back(); break;
            case 4: if (!m_displayName.empty()) m_displayName.pop_back(); break;
            default: break;
        }
    }

    if (IsKeyPressed(KEY_TAB)) {
        if (m_mode == AuthMenuMode::Login) {
            m_activeField = (m_activeField == 1) ? 2 : 1;
        } else if (m_mode == AuthMenuMode::Register) {
            m_activeField = (m_activeField % 4) + 1;
        }
    }
}

bool AuthMenu::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed) {
    (void)dt;
    if (!m_isOpen) return false;
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;

    if (m_client) {
        m_client->Update();
    }

    HandleTextInput();

    // Close button
    Rectangle closeBtn{ 450.0f, 40.0f, 24.0f, 20.0f };
    if (m_mousePressed && CheckHover(closeBtn)) {
        Close();
        return false;
    }

    if (m_mode == AuthMenuMode::Login) {
        Rectangle userBox{ 220.0f, 100.0f, 200.0f, 24.0f };
        Rectangle pwdBox{ 220.0f, 140.0f, 200.0f, 24.0f };
        Rectangle loginBtn{ 220.0f, 180.0f, 95.0f, 26.0f };
        Rectangle switchToRegBtn{ 325.0f, 180.0f, 95.0f, 26.0f };

        if (m_mousePressed) {
            if (CheckHover(userBox)) m_activeField = 1;
            else if (CheckHover(pwdBox)) m_activeField = 2;
            else if (CheckHover(loginBtn) && !m_isBusy) {
                if (m_username.empty() || m_password.empty()) {
                    m_statusMessage = "Please enter username and password.";
                    m_statusColor = UITheme::TextDanger;
                } else if (m_client) {
                    m_isBusy = true;
                    m_statusMessage = "Authenticating...";
                    m_statusColor = UITheme::TextLight;
                    std::weak_ptr<bool> weakAlive = m_alive;
                    m_client->Login(m_username, m_password, [this, weakAlive](bool success, const std::string& err) {
                        auto alive = weakAlive.lock();
                        if (!alive || !*alive) return;
                        m_isBusy = false;
                        if (success) {
                            m_statusMessage = "Login successful!";
                            m_statusColor = UITheme::TextSuccess;
                            m_mode = AuthMenuMode::ProfileProgression;
                            m_client->SaveSessionToFile();
                            RefreshProgression();
                        } else {
                            m_statusMessage = err.empty() ? "Invalid credentials" : err;
                            m_statusColor = UITheme::TextDanger;
                        }
                    });
                }
            } else if (CheckHover(switchToRegBtn)) {
                m_mode = AuthMenuMode::Register;
                m_statusMessage.clear();
                m_activeField = 1;
            }
        }
    } else if (m_mode == AuthMenuMode::Register) {
        Rectangle userBox{ 220.0f, 85.0f, 200.0f, 22.0f };
        Rectangle emailBox{ 220.0f, 115.0f, 200.0f, 22.0f };
        Rectangle pwdBox{ 220.0f, 145.0f, 200.0f, 22.0f };
        Rectangle dispBox{ 220.0f, 175.0f, 200.0f, 22.0f };
        Rectangle regBtn{ 220.0f, 210.0f, 95.0f, 26.0f };
        Rectangle backToLoginBtn{ 325.0f, 210.0f, 95.0f, 26.0f };

        if (m_mousePressed) {
            if (CheckHover(userBox)) m_activeField = 1;
            else if (CheckHover(emailBox)) m_activeField = 3;
            else if (CheckHover(pwdBox)) m_activeField = 2;
            else if (CheckHover(dispBox)) m_activeField = 4;
            else if (CheckHover(regBtn) && !m_isBusy) {
                if (m_username.empty() || m_email.empty() || m_password.empty()) {
                    m_statusMessage = "All required fields must be filled.";
                    m_statusColor = UITheme::TextDanger;
                } else if (m_client) {
                    m_isBusy = true;
                    m_statusMessage = "Creating account...";
                    m_statusColor = UITheme::TextLight;
                    std::weak_ptr<bool> weakAlive = m_alive;
                    m_client->Register(m_username, m_email, m_password, m_displayName, [this, weakAlive](bool success, const std::string& err) {
                        auto alive = weakAlive.lock();
                        if (!alive || !*alive) return;
                        m_isBusy = false;
                        if (success) {
                            m_statusMessage = "Account created & logged in!";
                            m_statusColor = UITheme::TextSuccess;
                            m_mode = AuthMenuMode::ProfileProgression;
                            m_client->SaveSessionToFile();
                            RefreshProgression();
                        } else {
                            m_statusMessage = err.empty() ? "Registration failed" : err;
                            m_statusColor = UITheme::TextDanger;
                        }
                    });
                }
            } else if (CheckHover(backToLoginBtn)) {
                m_mode = AuthMenuMode::Login;
                m_statusMessage.clear();
                m_activeField = 1;
            }
        }
    } else if (m_mode == AuthMenuMode::ProfileProgression) {
        Rectangle refreshBtn{ 220.0f, 220.0f, 65.0f, 24.0f };
        Rectangle logoutBtn{ 295.0f, 220.0f, 65.0f, 24.0f };
        Rectangle logoutAllBtn{ 370.0f, 220.0f, 65.0f, 24.0f };

        if (m_mousePressed) {
            if (CheckHover(refreshBtn)) {
                RefreshProgression();
            } else if (CheckHover(logoutBtn) && m_client) {
                std::weak_ptr<bool> weakAlive = m_alive;
                m_client->Logout([this, weakAlive](bool, const std::string&) {
                    auto alive = weakAlive.lock();
                    if (!alive || !*alive) return;
                    m_client->ClearSession();
                    m_client->SaveSessionToFile();
                    m_mode = AuthMenuMode::Login;
                    m_statusMessage = "Logged out successfully.";
                    m_statusColor = UITheme::TextLight;
                });
            } else if (CheckHover(logoutAllBtn) && m_client) {
                std::weak_ptr<bool> weakAlive = m_alive;
                m_client->LogoutAll([this, weakAlive](bool, const std::string&) {
                    auto alive = weakAlive.lock();
                    if (!alive || !*alive) return;
                    m_client->ClearSession();
                    m_client->SaveSessionToFile();
                    m_mode = AuthMenuMode::Login;
                    m_statusMessage = "Logged out from all sessions.";
                    m_statusColor = UITheme::TextLight;
                });
            }
        }
    }

    return true;
}

void AuthMenu::Render() {
    if (!m_isOpen) return;

    // Dim background overlay
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 0, 0, 0, 180 });

    // Center Panel Dialog
    Rectangle modalRec{ 160.0f, 35.0f, 320.0f, 260.0f };
    DrawRectangleRounded(modalRec, 0.08f, 6, UITheme::DarkBg);
    DrawRectangleRoundedLines(modalRec, 0.08f, 6, UITheme::BorderGold);

    // Modal Header
    DrawRectangle(160, 35, 320, 28, UITheme::PanelHeader);
    DrawLine(160, 63, 480, 63, UITheme::BorderGold);

    const char* title = (m_mode == AuthMenuMode::Login) ? "CONTENDER AUTHENTICATION — LOGIN" :
                        (m_mode == AuthMenuMode::Register ? "CREATE CONTENDER ACCOUNT" : "CONTENDER PROFILE & PROGRESSION");
    DrawText(title, 175, 43, 11, UITheme::Gold);

    // Close button "X"
    Rectangle closeBtn{ 450.0f, 40.0f, 20.0f, 18.0f };
    bool closeHover = CheckHover(closeBtn);
    DrawRectangleRounded(closeBtn, 0.2f, 4, closeHover ? UITheme::TextDanger : UITheme::SlotFilled);
    DrawText("X", 457, 44, 10, UITheme::TextLight);

    if (m_mode == AuthMenuMode::Login) {
        DrawText("Username or Email:", 220, 85, 10, UITheme::TextMuted);
        Rectangle userBox{ 220.0f, 100.0f, 200.0f, 24.0f };
        DrawRectangleRounded(userBox, 0.2f, 4, UITheme::SlotFilled);
        DrawRectangleRoundedLines(userBox, 0.2f, 4, (m_activeField == 1) ? UITheme::Gold : UITheme::BorderNormal);
        DrawText(m_username.c_str(), 226, 107, 10, UITheme::TextLight);

        DrawText("Password:", 220, 128, 10, UITheme::TextMuted);
        Rectangle pwdBox{ 220.0f, 140.0f, 200.0f, 24.0f };
        DrawRectangleRounded(pwdBox, 0.2f, 4, UITheme::SlotFilled);
        DrawRectangleRoundedLines(pwdBox, 0.2f, 4, (m_activeField == 2) ? UITheme::Gold : UITheme::BorderNormal);
        std::string maskedPwd(m_password.length(), '*');
        DrawText(maskedPwd.c_str(), 226, 147, 10, UITheme::TextLight);

        Rectangle loginBtn{ 220.0f, 180.0f, 95.0f, 26.0f };
        Rectangle switchToRegBtn{ 325.0f, 180.0f, 95.0f, 26.0f };
        DrawButton(loginBtn, "LOG IN", CheckHover(loginBtn), false);
        DrawButton(switchToRegBtn, "REGISTER", CheckHover(switchToRegBtn), false);
    } else if (m_mode == AuthMenuMode::Register) {
        DrawText("Username:", 220, 72, 9, UITheme::TextMuted);
        Rectangle userBox{ 220.0f, 85.0f, 200.0f, 22.0f };
        DrawRectangleRounded(userBox, 0.2f, 4, UITheme::SlotFilled);
        DrawRectangleRoundedLines(userBox, 0.2f, 4, (m_activeField == 1) ? UITheme::Gold : UITheme::BorderNormal);
        DrawText(m_username.c_str(), 226, 91, 10, UITheme::TextLight);

        DrawText("Email Address:", 220, 104, 9, UITheme::TextMuted);
        Rectangle emailBox{ 220.0f, 115.0f, 200.0f, 22.0f };
        DrawRectangleRounded(emailBox, 0.2f, 4, UITheme::SlotFilled);
        DrawRectangleRoundedLines(emailBox, 0.2f, 4, (m_activeField == 3) ? UITheme::Gold : UITheme::BorderNormal);
        DrawText(m_email.c_str(), 226, 121, 10, UITheme::TextLight);

        DrawText("Password:", 220, 134, 9, UITheme::TextMuted);
        Rectangle pwdBox{ 220.0f, 145.0f, 200.0f, 22.0f };
        DrawRectangleRounded(pwdBox, 0.2f, 4, UITheme::SlotFilled);
        DrawRectangleRoundedLines(pwdBox, 0.2f, 4, (m_activeField == 2) ? UITheme::Gold : UITheme::BorderNormal);
        std::string maskedPwd(m_password.length(), '*');
        DrawText(maskedPwd.c_str(), 226, 151, 10, UITheme::TextLight);

        DrawText("Display Name (optional):", 220, 164, 9, UITheme::TextMuted);
        Rectangle dispBox{ 220.0f, 175.0f, 200.0f, 22.0f };
        DrawRectangleRounded(dispBox, 0.2f, 4, UITheme::SlotFilled);
        DrawRectangleRoundedLines(dispBox, 0.2f, 4, (m_activeField == 4) ? UITheme::Gold : UITheme::BorderNormal);
        DrawText(m_displayName.c_str(), 226, 181, 10, UITheme::TextLight);

        Rectangle regBtn{ 220.0f, 210.0f, 95.0f, 26.0f };
        Rectangle backToLoginBtn{ 325.0f, 210.0f, 95.0f, 26.0f };
        DrawButton(regBtn, "CREATE", CheckHover(regBtn), false);
        DrawButton(backToLoginBtn, "HAVE ACCOUNT", CheckHover(backToLoginBtn), false);
    } else if (m_mode == AuthMenuMode::ProfileProgression) {
        std::string nameStr = "Contender: " + (m_client ? m_client->GetSession().displayName : "Offline");
        DrawText(nameStr.c_str(), 180, 75, 12, UITheme::Gold);

        std::string lvlStr = "Account Level: " + std::to_string(m_accountLevel);
        DrawText(lvlStr.c_str(), 180, 95, 11, UITheme::TextLight);

        std::string xpStr = "Total XP: " + std::to_string(m_totalXP) + " (" +
                            std::to_string(m_currentLevelXP) + " / " + std::to_string(m_nextLevelXP) + " to next level)";
        DrawText(xpStr.c_str(), 180, 115, 10, UITheme::TextMuted);

        // Progress bar
        Rectangle barBg{ 180.0f, 132.0f, 280.0f, 12.0f };
        DrawRectangleRec(barBg, UITheme::SlotFilled);
        DrawRectangleLinesEx(barBg, 1.0f, UITheme::BorderNormal);

        float fillW = (m_progressPct > 100.0f ? 100.0f : m_progressPct) / 100.0f * 278.0f;
        Rectangle barFill{ 181.0f, 133.0f, fillW, 10.0f };
        DrawRectangleRec(barFill, UITheme::Gold);

        std::string ptsStr = "Available Reward Points: " + std::to_string(m_rewardPoints);
        DrawText(ptsStr.c_str(), 180, 155, 10, UITheme::TextSuccess);

        std::string statusStr = "Identity verified via Wildbound Backend.";
        DrawText(statusStr.c_str(), 180, 180, 9, UITheme::TextMuted);

        Rectangle refreshBtn{ 200.0f, 220.0f, 75.0f, 24.0f };
        Rectangle logoutBtn{ 285.0f, 220.0f, 75.0f, 24.0f };
        Rectangle logoutAllBtn{ 370.0f, 220.0f, 75.0f, 24.0f };
        DrawButton(refreshBtn, "REFRESH", CheckHover(refreshBtn));
        DrawButton(logoutBtn, "LOGOUT", CheckHover(logoutBtn));
        DrawButton(logoutAllBtn, "LOGOUT ALL", CheckHover(logoutAllBtn));
    }

    // Status / Error message
    if (!m_statusMessage.empty()) {
        int textW = MeasureText(m_statusMessage.c_str(), 9);
        int posX = static_cast<int>(160.0f + (320.0f - textW) * 0.5f);
        DrawText(m_statusMessage.c_str(), posX, 265, 9, m_statusColor);
    }
}

} // namespace wildbound::ui
