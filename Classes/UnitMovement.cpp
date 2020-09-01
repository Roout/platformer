#include "UnitMovement.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"

Movement::Movement(cocos2d::PhysicsBody * const body):
    m_body { body } 
{
    m_body->retain();
}

Movement::~Movement() {
    m_body->release();
}

void Movement::Update(const float dt) noexcept {
    if( m_remainingJumpSteps ) {
        // F = mv / t
        const auto force { 
            ( m_body->getMass() * m_desiredVelocity * 1.5f ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        const auto multiplier { (4.f * m_remainingJumpSteps + 1.f) / 6.f };
        m_body->applyForce({ 0.f, force * multiplier });
        m_remainingJumpSteps--;
    }

    const auto jumpSideMoveMultiplier { m_remainingJumpSteps? 0.6f : 0.8f };

    if( m_isMovingLeft || m_isMovingRight ) {
        // F = mv / t
        const auto force { m_body->getMass() * m_desiredVelocity / dt };
        m_body->applyForce({ m_isMovingLeft? -force: force, 0.f });
    }

    const auto currentVelocity { m_body->getVelocity() };
    m_body->setVelocity({
        cocos2d::clampf(
            currentVelocity.x, 
            -m_desiredVelocity * jumpSideMoveMultiplier, 
            m_desiredVelocity * jumpSideMoveMultiplier
        ),
        cocos2d::clampf(
            currentVelocity.y, 
            -m_desiredVelocity, 
            m_desiredVelocity
        )
    });
}

void Movement::Jump() noexcept {
    m_remainingJumpSteps = m_timeStepsToCompletion;
};
void Movement::MoveRight() noexcept {
    m_isMovingRight = true;
    m_isMovingLeft = false;
};
void Movement::MoveLeft() noexcept {
    m_isMovingRight = false;
    m_isMovingLeft = true;
};

void Movement::Stop() noexcept {
    const auto vel { m_body->getVelocity() };
    if(helper::IsEquel(vel.y, 0.f, 0.01f)) {
        m_body->resetForces();
    }
    m_body->setVelocity({ 0.f, vel.y });

    m_isMovingLeft = m_isMovingRight = false;
}

void Movement::SetMaxSpeed(float speed) noexcept {
    m_desiredVelocity = speed;
}
