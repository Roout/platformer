#include "Movement.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"
#include "chipmunk/chipmunk.h"
#include <cmath>

Movement::Movement(cocos2d::PhysicsBody * const body):
    m_body { body },
    m_upJumpSpeed { sqrtf(2.f * JUMP_HEIGHT * (-GRAVITY)) },
    m_maxVelocity { floorf(m_upJumpSpeed + 1.f) },
    m_downJumpSpeed { -GRAVITY * sqrtf(2.f * JUMP_HEIGHT / (-GRAVITY)) }
{
    m_body->retain();
}

Movement::~Movement() {
    m_body->release();
}

void Movement::Update(float [[maybe_unused]] dt) noexcept {
    if(m_impulse.x != 0.f || m_impulse.y != 0.f ) {
        m_body->applyImpulse(m_impulse);
        m_impulse = { 0.f, 0.f };
    }
    
    if(m_force.x != 0.f || m_force.y != 0.f ) {
        m_body->applyForce(m_force);
    }

    const auto currentVelocity { m_body->getVelocity() };
    auto xClamp { 
        cocos2d::clampf(currentVelocity.x, -m_desiredVelocity, m_desiredVelocity) 
    };
    auto yClamp { // default clamp for speed limit when jumping
        cocos2d::clampf(currentVelocity.y, -m_maxVelocity, m_maxVelocity) 
    };
    if(m_force.y != 0.f) { // clamp speed for regular movement along Y-axis
        yClamp = cocos2d::clampf(currentVelocity.y, -m_desiredVelocity, m_desiredVelocity);
    }
    m_body->setVelocity({ xClamp, yClamp });
}


void Movement::Push(float [[maybe_unused]] x, float y) noexcept {
    float yJumpSpeed = 0.f;
    if(y > 0.f) { // jump
        yJumpSpeed = m_upJumpSpeed;
    }
    else if(y < 0.f) { // fall (usefull for spiders on death)
        yJumpSpeed = m_downJumpSpeed;
    }
    const auto jumpImpulse { m_body->getMass() * yJumpSpeed };
    m_impulse.y = jumpImpulse * y;
}

void Movement::Move(float x, float y) noexcept {
    const auto force { m_body->getMass() * m_desiredVelocity * 25.f };

    if(x == 0.f && y == 0.f) {
        this->ResetForce();
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
