#include "Curses.hpp"
#include "units/Unit.hpp"

#include <cassert>

namespace curses  {

DPS::DPS(size_t id, float damagePerSecond, float duration):
    Curse { id },
    m_damage { damagePerSecond },
    m_duration { duration },
    m_cooldown { 0.f }
{}

void DPS::Update(const float dt) noexcept {
    if (m_duration > 0.f || m_duration == UNLIMITED) {
        if (m_duration != UNLIMITED) 
            m_duration -= dt;
        if (m_cooldown > 0.f)
            m_cooldown -= dt;
    }
}

void DPS::EffectUnit(Unit * const unit) noexcept {
    assert(unit);
    assert(CanEffect());

    m_cooldown = 1.0f;
    unit->RecieveDamage(static_cast<int>(m_damage));
}

} // namespace curses