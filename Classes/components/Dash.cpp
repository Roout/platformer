#include "Dash.hpp"

#include "units/Unit.hpp"

#include "chipmunk/chipmunk.h"

#include <cassert>

Dash* Dash::create(float cooldown, float initSpeed, float dashSpeed) {
    auto pRet = new(std::nothrow) Dash(cooldown, initSpeed, dashSpeed);
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
    auto unit = static_cast<Unit*>(_owner);
    if (_owner && !unit->IsDead() && !m_dashTimer.IsFinished()) {
        m_dashTimer.Update(dt);
        if (m_dashTimer.IsFinished()) {
            // on dash end 
            // TODO: add callback
            // restore speed
            unit->SetMaxSpeed(m_initialSpeed);
            unit->Stop(Movement::Axis::XY);
            {
                auto body = unit->getPhysicsBody();
                auto chipBody { body->getCPBody() };
                cpBodySetForce(chipBody, m_forces);
                cpBodySetVelocity(chipBody, m_velocity);
            }
        }
        else {
            unit->Stop(Movement::Axis::Y);
        }
    }

    m_cooldownTimer.Update(dt);
}

void Dash::Initiate(float dashDuration) noexcept {
    auto unit = static_cast<Unit*>(_owner);
    if (_owner 
        && !unit->IsDead() 
        && m_dashTimer.IsFinished() 
        && m_cooldownTimer.IsFinished()
    ) {
        m_dashTimer.Start(dashDuration);
        m_cooldownTimer.Start(m_cooldown);
        {
            auto body { unit->getPhysicsBody() };
            auto chipBody { body->getCPBody() };
            m_forces = cpBodyGetForce(chipBody);
            m_velocity = cpBodyGetVelocity(chipBody);
        }
        // speed up unit
        unit->SetMaxSpeed(m_dashSpeed);
        unit->Stop(Movement::Axis::XY);
        unit->MoveAlong(unit->IsLookingLeft()? Movement::Direction::LEFT
            : Movement::Direction::RIGHT);
    }
}