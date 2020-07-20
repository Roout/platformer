#include "CurseHub.hpp"

Curses::CurseHub::CurseHub(Unit * const unit):
    m_unit { unit }
{}

void Curses::CurseHub::Update(const float dt) {
    for(auto& curse: m_curses) {
        if( curse ) {
            // update cooldown and duration
            curse->Update(dt);
            // apply curse damage
            if( curse->CanEffect() ) { 
                curse->EffectUnit(m_unit);
            }
            // remove expired
            if( curse->IsExpired() ) { 
                curse.reset();
            }
        }
    }

}