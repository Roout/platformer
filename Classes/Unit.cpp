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
    m_body->setGravityEnable(false);
    m_body->setRotationEnable(false);
    m_body->setVelocityLimit(550);
    // m_body->setPositionOffset(cocos2d::Vec2{x, y});
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

void Unit::UpdateState(const float dt) noexcept {
    const auto direction { m_body->getVelocity() };

    /// update the direction where charactar is looking now
    if( direction.x > 0.f) m_lookSide = Side::right;
    else if( direction.x < 0.f) m_lookSide = Side::left;


    if( m_state == State::attack ) {
        m_attackTime -= dt;
        if( m_attackTime > 0.f ) {
            // exit to not change an attack state
            return; 
        }
    }

    if( direction.y != 0.f ) {
        m_state = State::jump;
    } else if( direction.x != 0.f ) {
        m_state = State::move;
    } else {
        m_state = State::idle;
    }
}