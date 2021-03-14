#include "Dash.hpp"
#include "Unit.hpp"
#include "Player.hpp"

#include "chipmunk/chipmunk.h"

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
            // on dash end 
            auto unit = static_cast<Unit*>(_owner);
            // TODO: add callback
            // restore speed
            unit->SetMaxSpeed(Player::MAX_SPEED);
            unit->ResetForces(true, true);
            {
                auto body = unit->getPhysicsBody();
                auto chipBody { body->getCPBody() };
                cpBodySetForce(chipBody, m_forces);
                cpBodySetVelocity(chipBody, m_velocity);
            }
        }
        else {
            auto unit = static_cast<Unit*>(_owner);
            unit->ResetForces(false, true);
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
        auto unit = static_cast<Unit*>(_owner);
        {
            auto body = unit->getPhysicsBody();
            auto chipBody { body->getCPBody() };
            m_forces = cpBodyGetForce(chipBody);
            m_velocity = cpBodyGetVelocity(chipBody);
        }
        // speed up unit
        unit->SetMaxSpeed(600.f);
        unit->MoveAlong(0.f, 0.f);
        unit->MoveAlong((unit->IsLookingLeft()? -1.f: 1.f), 0.f);
    }
}