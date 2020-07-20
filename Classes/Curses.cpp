#include "Curses.hpp"
#include "Unit.hpp"

Curses::DPS::DPS(float damagePerSecond, float duration):
    m_damage { damagePerSecond },
    m_duration { duration },
    m_cooldown { 0.f }
{}

void Curses::DPS::Update(const float dt) noexcept {
    if( m_duration > 0.f || m_duration == UNLIMITED ) {
        if( m_duration != UNLIMITED ) 
            m_duration -= dt;
        if( m_cooldown > 0.f )
            m_cooldown -= dt;
    }
}

void Curses::DPS::EffectUnit(Unit * const unit) noexcept {
    if( m_cooldown <= 0.f ) {
        m_cooldown = 1.0f;
        unit->RecieveDamage(m_damage);
    }
}