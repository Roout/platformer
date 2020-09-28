#include "UnitMovement.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"

Movement::Movement(cocos2d::PhysicsBody * const body):
    m_body { body },
    m_maxVelocity { sqrtf(2.f * m_jumpHeight * (-m_gravity)) + 50.f }
{
    m_body->retain();
    m_indicators.fill(false);
}

Movement::~Movement() {
    m_body->release();
}

void Movement::Update(const float dt) noexcept {    
    // Fix #24: avoid applying infinity force due to 0-division!
    if(dt <= 0.f) return; 

    const auto yJumpSpeed { sqrtf(2.f * m_jumpHeight * (-m_gravity)) };
    // p = mv;
    auto isOnGround { helper::IsEquel(m_body->getVelocity().y, 0.f, 0.001f) };

    for(size_t i = 0; i < m_indicators.size(); i++) {
        if(!m_indicators[i]) continue;

        const auto action { Utils::EnumCast<Action>(i) };
        switch (action) {
            case Action::JUMP : {
                const auto jumpImpulse { m_body->getMass() * yJumpSpeed };
                m_body->applyImpulse({ 0.f, jumpImpulse });
                // update state
                m_indicators[i] = --m_remainingAirSteps > 0;
            } break;
            case Action::MOVE_UP : {
                const auto force { m_body->getMass() * m_desiredVelocity / dt };
                m_body->applyForce({ 0.f, force });
            } break;
            case Action::MOVE_DOWN : {
                const auto force { -m_body->getMass() * m_desiredVelocity / dt };
                m_body->applyForce({ 0.f, force });
            } break;
            case Action::MOVE_LEFT : {
                // calculated form the expected ratio of jump height to desired shift 
                // along X-axis for this time
                auto xSpeed { m_desiredVelocity };
                if(!isOnGround) {
                    // if we're jumping/falling
                    constexpr auto ctan { 0.1f }; 
                    xSpeed = yJumpSpeed * ctan;
                }
                //  F = mv / t
                const auto force { -m_body->getMass() * xSpeed / dt };
                m_body->applyForce({ force, 0.f });
            } break;
            case Action::MOVE_RIGHT : {
                // calculated form the expected ratio of jump height to desired shift 
                // along X-axis for this time
                auto xSpeed { m_desiredVelocity };
                if(!isOnGround) {
                    // if we're jumping/falling
                    constexpr auto ctan { 0.1f }; 
                    xSpeed = yJumpSpeed * ctan;
                }
                //  F = mv / t
                const auto force { m_body->getMass() * xSpeed / dt };
                m_body->applyForce({ force, 0.f });
            } break;
            default:break;
        }
    }

    const auto currentVelocity { m_body->getVelocity() };
    auto xClamp { cocos2d::clampf(currentVelocity.x, -m_desiredVelocity, m_desiredVelocity) };
    auto yClamp { cocos2d::clampf(currentVelocity.y, -m_maxVelocity, m_maxVelocity) };
    if( m_indicators[Utils::EnumCast(Action::MOVE_UP)] || 
        m_indicators[Utils::EnumCast(Action::MOVE_DOWN)]
    ) { // clamp speed for regular movement along Y-axis
        yClamp = cocos2d::clampf(currentVelocity.y, -m_desiredVelocity, m_desiredVelocity);
    }
    m_body->setVelocity({ xClamp, yClamp });
}

void Movement::Jump() noexcept {
    m_remainingAirSteps = m_timeStepsToCompletion;
    m_indicators[Utils::EnumCast(Action::JUMP)]         = true;
    m_indicators[Utils::EnumCast(Action::MOVE_UP)]      = false;
    m_indicators[Utils::EnumCast(Action::MOVE_DOWN)]    = false;
};

void Movement::MoveUp() noexcept {
    m_indicators[Utils::EnumCast(Action::JUMP)]         = false;
    m_indicators[Utils::EnumCast(Action::MOVE_UP)]      = true;
    m_indicators[Utils::EnumCast(Action::MOVE_DOWN)]    = false;
};

void Movement::MoveDown() noexcept {
    m_indicators[Utils::EnumCast(Action::JUMP)]         = false;
    m_indicators[Utils::EnumCast(Action::MOVE_UP)]      = false;
    m_indicators[Utils::EnumCast(Action::MOVE_DOWN)]    = true;
};

void Movement::MoveRight() noexcept {
    m_indicators[Utils::EnumCast(Action::MOVE_LEFT)]    = false;
    m_indicators[Utils::EnumCast(Action::MOVE_RIGHT)]    = true;
};

void Movement::MoveLeft() noexcept {
    m_indicators[Utils::EnumCast(Action::MOVE_LEFT)]    = true;
    m_indicators[Utils::EnumCast(Action::MOVE_RIGHT)]   = false;
};

void Movement::Stop() noexcept {
    m_indicators[Utils::EnumCast(Action::MOVE_LEFT)]    = false;
    m_indicators[Utils::EnumCast(Action::MOVE_RIGHT)]   = false;
    m_indicators[Utils::EnumCast(Action::MOVE_UP)]      = false;
    m_indicators[Utils::EnumCast(Action::MOVE_DOWN)]    = false;

    const auto vel { m_body->getVelocity() };
    if(helper::IsEquel(vel.y, 0.f, 0.01f)) {
        m_body->resetForces();
    }
    m_body->setVelocity({ 0.f, vel.y });

}

void Movement::SetMaxSpeed(float speed) noexcept {
    m_desiredVelocity = speed;
}
