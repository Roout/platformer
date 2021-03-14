#include "Dash.hpp"
#include "Unit.hpp"
#include "Player.hpp"

#include <cassert>

Dash* Dash::create(float cooldown) {
    auto pRet = new(std::nothrow) Dash(cooldown);
    if(pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

void Dash::update(float dt) {
    if (!m_dashTimer.IsFinished()) {
        m_dashTimer.Update(dt);
        if (m_dashTimer.IsFinished()) {
            // TODO: add callback
            // on dash end 
            auto unit = static_cast<Unit*>(_owner);
            unit->MoveAlong(0, 0);
            // restore speed
            unit->SetMaxSpeed(Player::MAX_SPEED);
        }
    }

    m_cooldownTimer.Update(dt);
}

void Dash::Initiate(float dashDuration) noexcept {
    if (m_dashTimer.IsFinished() 
        && m_cooldownTimer.IsFinished()
    ) {
        m_dashTimer.Start(dashDuration);
        m_cooldownTimer.Start(m_cooldown);
        // speed up unit
        auto unit = static_cast<Unit*>(_owner);
        unit->SetMaxSpeed(600.f);
        unit->MoveAlong((unit->IsLookingLeft()? -1.f: 1.f), 0.f);
    }
}