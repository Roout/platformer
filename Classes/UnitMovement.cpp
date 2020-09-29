#include "UnitMovement.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"
#include <cmath>

Movement::Movement(cocos2d::PhysicsBody * const body):
    m_body { body },
    m_maxVelocity { floorf(sqrtf(2.f * m_jumpHeight * (-m_gravity)) + 1.f ) },
    m_upJumpSpeed { sqrtf(2.f * m_jumpHeight * (-m_gravity)) },
    m_downJumpSpeed { -m_gravity * sqrtf(2.f * m_jumpHeight / (-m_gravity)) }
{
    m_body->retain();
}

Movement::~Movement() {
    m_body->release();
}

/** 
 * TODO:
 * - [x] fix clamping for jump state
 * - [x] fix horizontal speed update on jump (now if u jumo with non-zero force applied along x-axis)
 * cotangence won't work!
 */
void Movement::Update(float dt) noexcept {
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


void Movement::Push(float x, float y) noexcept {
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
        this->Stop();
    }
    else {
        m_force = { force * x, force * y };
    }
}

void Movement::Stop() noexcept {
    m_force.x = m_force.y = m_impulse.x = 0.f;
    const auto velocity { m_body->getVelocity() };
    if(helper::IsEquel(velocity.y, 0.f, 0.01f)) {
        m_body->resetForces();
    }
    m_body->setVelocity({ 0.f, velocity.y });
}

void Movement::SetMaxSpeed(float speed) noexcept {
    m_desiredVelocity = speed;
}
