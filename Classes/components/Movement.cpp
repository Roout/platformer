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

void Movement::Push(Direction dir, float scale) noexcept {
    assert(scale > 0.f);
    const float mass = m_body->getMass();
    switch (dir) {
        case Direction::UP: {
            m_impulse.y = mass * m_upJumpSpeed * scale; 
        } break;
        case Direction::DOWN: {
            m_impulse.y = -mass * m_downJumpSpeed * scale; 
        } break;
        case Direction::LEFT: {
            m_impulse.x = -mass * m_desiredVelocity * scale;
        } break;
        case Direction::RIGHT: {
            m_impulse.x = mass * m_desiredVelocity * scale;
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void Movement::Move(Direction dir, float scale) noexcept {
    assert(scale > 0.f);

    const auto force { m_body->getMass() * m_desiredVelocity * 20.f};
    
    switch (dir) {
        case Direction::UP: {
            m_force.y = force * scale;
        } break;
        case Direction::DOWN: {
            m_force.y = -force * scale;
        } break;
        case Direction::LEFT: {
            m_force.x = -force * scale;
        } break;
        case Direction::RIGHT: {
            m_force.x = force * scale;
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void Movement::Stop(Axis axis) noexcept {
    auto chipBody { m_body->getCPBody() };
    auto force { cpBodyGetForce(chipBody) };
    auto velocity { cpBodyGetVelocity(chipBody) };

    switch (axis) {
        case Axis::X: {
            force.x = velocity.x = 0.f;
            m_force.x = m_impulse.x = 0.f;
        } break;
        case Axis::Y: {
            force.y = velocity.y = 0.f;
            m_force.y = m_impulse.y = 0.f;
        } break;
        case Axis::XY: {
            force = velocity = { 0.f, 0.f };
            m_force = m_impulse = { 0.f, 0.f };
        } break;
    }

    cpBodySetForce(chipBody, force);
    cpBodySetVelocity(chipBody, velocity);
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