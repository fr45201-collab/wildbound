#include "HealthComponent.hpp"
#include <algorithm>

namespace wildbound::combat {

HealthComponent::HealthComponent(float maxHp, float initialHp, bool regen, float regenRate)
    : m_maxHealth(std::max(1.0f, maxHp))
    , m_currentHealth(initialHp >= 0.0f ? std::clamp(initialHp, 0.0f, m_maxHealth) : m_maxHealth)
    , m_regenerationEnabled(regen)
    , m_regenerationRate(std::max(0.0f, regenRate))
    , m_isDead(m_currentHealth <= 0.0f)
{
}

void HealthComponent::Initialize(const HealthConfig& config) {
    m_maxHealth = std::max(1.0f, config.maxHealth);
    m_currentHealth = std::clamp(config.initialHealth, 0.0f, m_maxHealth);
    m_regenerationEnabled = config.regenerationEnabled;
    m_regenerationRate = std::max(0.0f, config.regenerationRate);
    m_isDead = (m_currentHealth <= 0.0f);
}

void HealthComponent::Update(float dt) {
    if (m_isDead || !m_regenerationEnabled || m_regenerationRate <= 0.0f || dt <= 0.0f) {
        return;
    }

    if (m_currentHealth < m_maxHealth) {
        m_currentHealth = std::min(m_maxHealth, m_currentHealth + m_regenerationRate * dt);
    }
}

bool HealthComponent::ApplyDamage(float amount) {
    if (m_isDead || amount <= 0.0f) {
        return false;
    }

    m_currentHealth = std::max(0.0f, m_currentHealth - amount);

    if (m_currentHealth <= 0.0f) {
        m_currentHealth = 0.0f;
        m_isDead = true;
    }

    return true;
}

bool HealthComponent::ApplyHeal(float amount) {
    if (m_isDead || amount <= 0.0f) {
        return false;
    }

    if (m_currentHealth >= m_maxHealth) {
        return false;
    }

    m_currentHealth = std::min(m_maxHealth, m_currentHealth + amount);
    return true;
}

void HealthComponent::Reset() {
    m_currentHealth = m_maxHealth;
    m_isDead = false;
}

void HealthComponent::SetMaxHealth(float maxHp) {
    m_maxHealth = std::max(1.0f, maxHp);
    m_currentHealth = std::min(m_currentHealth, m_maxHealth);
}

void HealthComponent::SetCurrentHealth(float hp) {
    m_currentHealth = std::clamp(hp, 0.0f, m_maxHealth);
    m_isDead = (m_currentHealth <= 0.0f);
}

} // namespace wildbound::combat
