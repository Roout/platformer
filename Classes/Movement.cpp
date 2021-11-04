#include "Movement.hpp"
#include "PhysicsHelper.hpp"
#include "units/Unit.hpp"

#include "cocos2d.h"
#include "chipmunk/chipmunk.h"

#include <cmath>
#include <cassert>

Movement::Movement(cocos2d::PhysicsBody * const body
    , float gravity
    , float jumpHeight
) 
    : m_body { body }
{
    assert(body);
    
    SetJumpHeight(jumpHeight, gravity);
    m_body->retain();
}

Movement::~Movement() {
    assert(m_body);
    m_body->release();
}

void Movement::Update() noexcept {
    if (m_impulse.x != 0.f || m_impulse.y != 0.f) {
        m_body->applyImpulse(m_impulse);
        m_impulse = { 0.f, 0.f };
    }
    
    if (m_force.x != 0.f || m_force.y != 0.f) {
        m_body->applyForce(m_force);
    }

    const auto currentVelocity { m_body->getVelocity() };
    auto xClamp { 
        cocos2d::clampf(currentVelocity.x, -m_desiredVelocity, m_desiredVelocity) 
    };
    auto yClamp { // default clamp for speed limit when jumping
        cocos2d::clampf(currentVelocity.y, -m_maxVelocity, m_maxVelocity) 
    };
    if (m_force.y != 0.f) { // clamp speed for regular movement along Y-axis
        yClamp = cocos2d::clampf(currentVelocity.y, -m_desiredVelocity, m_desiredVelocity);
    }
    m_body->setVelocity({ xClamp, yClamp });
}


void Movement::Push(float x, float y) noexcept {
    float yJumpSpeed = 0.f;

    if (y > 0.f) { // jump
        yJumpSpeed = m_upJumpSpeed;
    }
    else if (y < 0.f) { // fall (usefull for spiders on death)
        yJumpSpeed = m_downJumpSpeed;
    }
    const auto jumpImpulse { m_body->getMass() * yJumpSpeed };
    // TODO: introduce the ability to customize horizontal velocity
    m_impulse.x = m_body->getMass() * m_desiredVelocity * x;
    m_impulse.y = jumpImpulse * y;
}

void Movement::Move(float x, float y) noexcept {
    const auto force { m_body->getMass() * m_desiredVelocity * 20.f };

    if (x == 0.f && y == 0.f) {
        ResetForce();
    }
    else {
        m_force = { force * x, force * y };
    }
}

void Movement::ResetForce() noexcept {
    auto chipBody { m_body->getCPBody() };
    m_force.x = m_force.y = m_impulse.x = m_impulse.y = 0.f;
    cpBodySetForce(chipBody, {0.f, 0.f});
    cpBodySetVelocity(chipBody, {0.f, 0.f});
}

void Movement::ResetForceX() noexcept {
    auto chipBody { m_body->getCPBody() };
    auto force { cpBodyGetForce(chipBody) };
    auto velocity { cpBodyGetVelocity(chipBody) };
    force.x = velocity.x = 0.f;
    cpBodySetForce(chipBody, force);
    cpBodySetVelocity(chipBody, velocity);
    // reset internal settings
    m_force.x = m_impulse.x = 0.f;
}

void Movement::ResetForceY() noexcept {
    auto chipBody { m_body->getCPBody() };
    auto force { cpBodyGetForce(chipBody) };
    auto velocity { cpBodyGetVelocity(chipBody) };
    force.y = velocity.y = 0.f;
    cpBodySetForce(chipBody, force);
    cpBodySetVelocity(chipBody, velocity);
    // reset internal settings
    m_force.y = m_impulse.y = 0.f;
}

void Movement::SetMaxSpeed(float speed) noexcept {
    m_desiredVelocity = speed;
}

void Movement::SetJumpHeight(float height, float gravity) noexcept {
    assert(height * (-gravity) >= 0.f);
    assert(gravity != 0.f);
    
    m_upJumpSpeed = sqrtf(2.f * height * (-gravity));
    m_maxVelocity = floorf(m_upJumpSpeed + 1.f);
    m_downJumpSpeed = -gravity * sqrtf(2.f * height / (-gravity));
}