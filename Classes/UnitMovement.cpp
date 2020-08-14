#include "UnitMovement.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"

void Movement::Update(const float dt) noexcept {
    if( m_counter.remainingJumpSteps ) {
        // F = mv / t
        const auto force { 
            ( m_unit->getPhysicsBody()->getMass() * m_desiredVelocity * 1.5f ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        const auto multiplier { (4.f * m_counter.remainingJumpSteps + 1.f) / 6.f };
        m_unit->getPhysicsBody()->applyForce({ 0.f, force * multiplier });
        m_counter.remainingJumpSteps--;
    }

    auto jumpSideMoveMultiplier { 0.8f };
    if( m_unit->m_currentState.m_act == Unit::Act::jump) {
        jumpSideMoveMultiplier = 0.6f;
    }

    if( m_counter.remainingMoveLeft ) {
        m_counter.remainingMoveLeft--;
        // F = mv / t
        const auto force { 
            ( m_unit->getPhysicsBody()->getMass() * m_desiredVelocity ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        
        m_unit->getPhysicsBody()->applyForce({ -force, 0.f });
    }
    else if( m_counter.remainingMoveRight ) {
        m_counter.remainingMoveRight--;
        // F = mv / t
        const auto force { 
            ( m_unit->getPhysicsBody()->getMass() * m_desiredVelocity ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        m_unit->getPhysicsBody()->applyForce({ force, 0.f });
    }

    const auto currentVelocity { m_unit->getPhysicsBody()->getVelocity() };
    m_unit->getPhysicsBody()->setVelocity({
        cocos2d::clampf(
            currentVelocity.x, 
            -m_desiredVelocity * jumpSideMoveMultiplier, 
            m_desiredVelocity * jumpSideMoveMultiplier
        ),
        cocos2d::clampf(currentVelocity.y, -m_desiredVelocity, m_desiredVelocity )
    });
}

void Movement::Jump() noexcept {
    m_counter.remainingJumpSteps = m_timeStepsToCompletion;
};
void Movement::MoveRight() noexcept {
    m_counter.remainingMoveRight = m_timeStepsToCompletion;
    m_counter.remainingMoveLeft  = 0;
};
void Movement::MoveLeft() noexcept {
    m_counter.remainingMoveRight = 0;
    m_counter.remainingMoveLeft  = m_timeStepsToCompletion;
};

void Movement::Stop() noexcept {
    m_unit->getPhysicsBody()->setVelocity({ 0.f, 0.f });
    m_unit->getPhysicsBody()->resetForces();

    m_counter.Clear();
}

void Movement::StopXAxisMove() noexcept {
    const auto vel { m_unit->getPhysicsBody()->getVelocity() };
    if(helper::IsEquel(vel.y, 0.f, 0.01f)) {
        m_unit->getPhysicsBody()->resetForces();
    }
    m_unit->getPhysicsBody()->setVelocity({ 0.f, vel.y });

    m_counter.remainingMoveLeft = m_counter.remainingMoveRight = 0;
}

void Movement::SetMaxSpeed(float speed) noexcept {
    m_desiredVelocity = speed;
}
