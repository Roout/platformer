#include "Unit.hpp"
#include "cocos2d.h"
#include "SizeDeducer.hpp"
#include "PhysicsHelper.hpp" 

Unit::Unit() :
    m_health { 100 }
{   
    const int damage { 10 };
    const int range { SizeDeducer::GetInstance().GetAdjustedSize(20) };
    const float reloadTime { m_maxAttackTime };

    m_weapon = std::make_unique<Sword>( damage, range, reloadTime );
}

Unit::~Unit() {}

void Unit::AddBody(cocos2d::PhysicsBody * const body) noexcept {
    m_body = body;
    m_body->setDynamic(true);
    m_body->setGravityEnable(true);
    m_body->setRotationEnable(false);
    m_body->setCategoryBitmask(
        core::CreateMask(
            core::CategoryBits::HERO
        )
    );
    m_body->setCollisionBitmask(
        core::CreateMask(
            core::CategoryBits::ENEMY, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE, 
            core::CategoryBits::PLATFORM, 
            core::CategoryBits::TRAP
        )
    );

    const auto sensorShapeSize = cocos2d::Size{ 
        GetSize().width / 2.f,
        SizeDeducer::GetInstance().GetAdjustedSize(10.f)
    };
    auto sensorShape = cocos2d::PhysicsShapeBox::create(
        sensorShapeSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT
    );
    sensorShape->setSensor(true);
    sensorShape->setCategoryBitmask(
        core::CreateMask(
            core::CategoryBits::HERO_SENSOR
        )
    );
    sensorShape->setCollisionBitmask(0);
    sensorShape->setContactTestBitmask(
        core::CreateMask(
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );
    m_body->addShape(sensorShape, false);
}

cocos2d::Size Unit::GetSize() const noexcept {
    return {
        SizeDeducer::GetInstance().GetAdjustedSize(m_width), 
        SizeDeducer::GetInstance().GetAdjustedSize(m_height)
    };
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
    cocos2d::log(" >>> unit recieve %d damage. Current health is %d.", damage, m_health);
}

void Unit::MeleeAttack() noexcept {
    if( m_weapon->CanAttack() ) {
        // update attack direction and position for idle case
        auto position = m_body->getPosition();
        auto direction = m_body->getVelocity();
        if(m_lookSide == Side::right) {
            direction.x = 1.f;
            position.x += SizeDeducer::GetInstance().GetAdjustedSize(m_width);
        }
        else if(m_lookSide == Side::left) {
            direction.x = -1.f;
            // position will be updated later base on projectile width!
        }

        static int x { 0 };
        cocos2d::log(" >>> unit attack with sword: %d", ++x );

        m_weapon->Attack(position, direction);

        m_state = State::attack;
        m_attackTime = m_maxAttackTime;
    }
}

void Unit::UpdateWeapon(const float dt) noexcept {
    m_weapon->Update(dt);
}

bool Unit::IsOnGround() const noexcept {
    const auto direction { m_body->getVelocity() };
    constexpr float EPS { 0.00001f };
    
    auto isOnGround { helper::IsEquel(direction.y, 0.f, EPS) };
    return isOnGround && m_hasContactWithGround;
}

void Unit::UpdateState(const float dt) noexcept {
    const auto direction { m_body->getVelocity() };
    constexpr float EPS { 0.00001f };
    // update character direction
    if( helper::IsPositive(direction.x, EPS) ) {
        m_lookSide = Side::right;
    } else if( helper::IsNegative(direction.x, EPS) ) {
        m_lookSide = Side::left;
    }

    // update character state
    if( m_state == State::attack ) {
        m_attackTime -= dt;
        if(  helper::IsPositive(m_attackTime, EPS) ) {
            // exit to not change an attack state
            return; 
        }
    }

    if( ! this->IsOnGround() ) {
        m_state = State::jump;
    } else if( !helper::IsEquel(direction.x, 0.f, EPS) ) {
        m_state = State::move;
    } else {
        m_state = State::idle;
    }
}

void Movement::Update(const float dt) noexcept {
    if( m_counter.remainingJumpSteps ) {
        // F = mv / t
        const auto force { 
            ( m_unit->m_body->getMass() * m_desiredVelocity ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        const auto multiplier { (2.f * m_counter.remainingJumpSteps + 1.f) / 6.f };
        m_unit->m_body->applyForce({ 0.f, force * multiplier });
        m_counter.remainingJumpSteps--;
    }

    auto jumpSideMoveMultiplier { 1.f };
    if( m_unit->GetState() == Unit::State::jump) {
        jumpSideMoveMultiplier = 0.6f;
    }

    if( m_counter.remainingMoveLeft ) {
        m_counter.remainingMoveLeft--;
        // F = mv / t
        const auto force { 
            ( m_unit->m_body->getMass() * m_desiredVelocity ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        
        m_unit->m_body->applyForce({ -force, 0.f });
    }
    else if( m_counter.remainingMoveRight ) {
        m_counter.remainingMoveRight--;
        // F = mv / t
        const auto force { 
            ( m_unit->m_body->getMass() * m_desiredVelocity ) / 
            ( dt * m_timeStepsToCompletion ) 
        };
        m_unit->m_body->applyForce({ force, 0.f });
    }

    const auto currentVelocity { m_unit->m_body->getVelocity() };
    m_unit->m_body->setVelocity({
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
    m_unit->m_body->setVelocity({ 0.f, 0.f });
    m_unit->m_body->resetForces();

    m_counter.Clear();
}

void Movement::StopXAxisMove() noexcept {
    const auto vel { m_unit->m_body->getVelocity() };
    m_unit->m_body->setVelocity({ 0.f, vel.y });

    m_counter.remainingMoveLeft = m_counter.remainingMoveRight = 0;
}