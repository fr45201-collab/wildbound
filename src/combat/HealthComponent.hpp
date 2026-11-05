#pragma once

#include <nlohmann/json.hpp>
#include <algorithm>

namespace wildbound::combat {

struct HealthConfig {
    float maxHealth = 100.0f;
    float initialHealth = 100.0f;
    bool regenerationEnabled = false;
    float regenerationRate = 0.0f; // HP per second

    static HealthConfig FromJson(const nlohmann::json& j) {
        HealthConfig cfg;
        if (j.contains("health")) {
            const auto& h = j["health"];
            cfg.maxHealth = h.value("max_health", 100.0f);
            cfg.initialHealth = h.value("initial_health", cfg.maxHealth);
            cfg.regenerationEnabled = h.value("regeneration_enabled", false);
            cfg.regenerationRate = h.value("regeneration_rate", 0.0f);
        }
        return cfg;
    }
};

class HealthComponent {
public:
    HealthComponent() = default;
    explicit HealthComponent(float maxHp, float initialHp = -1.0f, bool regen = false, float regenRate = 0.0f);

    void Initialize(const HealthConfig& config);
    void Update(float dt);

    // Damage & Healing
    bool ApplyDamage(float amount);
    bool ApplyHeal(float amount);
    bool Heal(float amount) { return ApplyHeal(amount); }
    void Reset();

    // State Queries
    float GetCurrentHealth() const { return m_currentHealth; }
    float GetMaxHealth() const { return m_maxHealth; }
    float GetHealthPercent() const { return m_maxHealth > 0.0f ? m_currentHealth / m_maxHealth : 0.0f; }

    bool IsAlive() const { return m_currentHealth > 0.0f && !m_isDead; }
    bool IsDead() const { return m_isDead; }

    void SetMaxHealth(float maxHp);
    void SetCurrentHealth(float hp);

    // Regeneration
    bool IsRegenEnabled() const { return m_regenerationEnabled; }
    void SetRegenEnabled(bool enabled) { m_regenerationEnabled = enabled; }
    float GetRegenRate() const { return m_regenerationRate; }
    void SetRegenRate(float rate) { m_regenerationRate = rate; }

    // Death marker
    void MarkDead() {
        m_currentHealth = 0.0f;
        m_isDead = true;
    }

private:
    float m_maxHealth = 100.0f;
    float m_currentHealth = 100.0f;
    bool m_regenerationEnabled = false;
    float m_regenerationRate = 0.0f;
    bool m_isDead = false;
};

} // namespace wildbound::combat
