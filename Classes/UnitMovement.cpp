#include "UnitMovement.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"

Movement::Movement(cocos2d::PhysicsBody * const body):
    m_body { body } 
{
    m_body->retain();
    m_indicators.fill(false);
}

Movement::~Movement() {
    m_body->release();
}

void Movement::Update(const float dt) noexcept {    
    const auto airSideMoveMultiplier { m_remainingAirSteps? 0.5f : 0.7f };
    const auto force { //  F = mv / t
        ( m_body->getMass() * m_desiredVelocity * 1.5f ) / 
        ( dt * m_timeStepsToCompletion ) 
    };
    const auto multiplier { (4.f * m_remainingAirSteps + 1.f) / 6.f };

    for(size_t i = 0; i < m_indicators.size(); i++) {
        if(!m_indicators[i]) continue;

        const auto direction { Utils::EnumCast<Direction>(i) };
        switch (direction) {
            case Direction::UP : {
                m_body->applyForce({ 0.f, force * multiplier });
                // update state
                m_indicators[i] = --m_remainingAirSteps > 0;
            } break;
            case Direction::DOWN : {
                m_body->applyForce({ 0.f, -force * multiplier });
                // update state
                m_indicators[i] = --m_remainingAirSteps > 0;
            } break;
            case Direction::LEFT : {
                const auto leftForce { -m_body->getMass() * m_desiredVelocity / dt };
                m_body->applyForce({ leftForce, 0.f });
            } break;
            case Direction::RIGHT : {
                const auto rightForce { m_body->getMass() * m_desiredVelocity / dt };
                m_body->applyForce({ rightForce, 0.f });
            } break;
            default:break;
        }
    }

    const auto currentVelocity { m_body->getVelocity() };
    m_body->setVelocity({
        cocos2d::clampf(
            currentVelocity.x, 
            -m_desiredVelocity * airSideMoveMultiplier, 
            m_desiredVelocity * airSideMoveMultiplier
        ),
        cocos2d::clampf(
            currentVelocity.y, 
            -m_desiredVelocity * 1.2f, 
            m_desiredVelocity * 1.2f
        )
    });
}

void Movement::MoveUp() noexcept {
    m_remainingAirSteps = m_timeStepsToCompletion;
    m_indicators[Utils::EnumCast(Direction::UP)]    = true;
    m_indicators[Utils::EnumCast(Direction::DOWN)]  = false;
};

void Movement::MoveDown() noexcept {
    m_remainingAirSteps = m_timeStepsToCompletion;
    m_indicators[Utils::EnumCast(Direction::UP)]    = false;
    m_indicators[Utils::EnumCast(Direction::DOWN)]  = true;
};

void Movement::MoveRight() noexcept {
    m_indicators[Utils::EnumCast(Direction::LEFT)]  = false;
    m_indicators[Utils::EnumCast(Direction::RIGHT)] = true;
};

void Movement::MoveLeft() noexcept {
    m_indicators[Utils::EnumCast(Direction::LEFT)]  = true;
    m_indicators[Utils::EnumCast(Direction::RIGHT)] = false;
};

void Movement::Stop() noexcept {
    m_indicators[Utils::EnumCast(Direction::LEFT)]  = false;
    m_indicators[Utils::EnumCast(Direction::RIGHT)] = false;

    const auto vel { m_body->getVelocity() };
    if(helper::IsEquel(vel.y, 0.f, 0.01f)) {
        m_body->resetForces();
    }
    m_body->setVelocity({ 0.f, vel.y });

}

void Movement::SetMaxSpeed(float speed) noexcept {
    m_desiredVelocity = speed;
}
