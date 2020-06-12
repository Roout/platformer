#include "Unit.hpp"
#include "cocos2d.h"
#include "SizeDeducer.hpp"

Unit::Unit(PhysicWorld * const world, float x, float y) :
    m_world { world },
    m_health { 100 }
{
    const auto callback = [](core::Entity* ) {
        cocos2d::log("Unit collide with some entity!");
    };

    m_body = m_world->Create<KinematicBody>(
        callback,
        cocos2d::Vec2{ x, y }, cocos2d::Size { 
            SizeDeducer::GetInstance().GetAdjustedSize(m_width), 
            SizeDeducer::GetInstance().GetAdjustedSize(m_height) 
        }
    );
    m_body->EmplaceFixture(this, core::CategoryName::UNDEFINED);
    m_body->SetMask(
        CreateMask(CategoryBits::HERO),
        CreateMask(CategoryBits::ENEMY, CategoryBits::BOUNDARY, CategoryBits::PROJECTILE, CategoryBits::PLATFORM) 
    );

    const int damage { 10 };
    const int range { 20 };
    const float reloadTime { m_maxAttackTime };

    m_weapon = std::make_unique<Sword>( damage, range, reloadTime );
}

Unit::~Unit() {
    m_world->Erase(m_body);
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
    cocos2d::log(" >>> unit recieve %d damage. Current health is %d.", damage, m_health);
}

void Unit::MeleeAttack() noexcept {
    if( m_weapon->CanAttack() ) {
        // update attack direction and position for idle case
        auto position = m_body->GetShape().origin;
        auto direction = m_body->GetDirection();
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

        m_weapon->Attack(m_world, position, direction);

        m_state = State::attack;
        m_attackTime = m_maxAttackTime;
    }
}

void Unit::UpdateWeapon(const float dt) noexcept {
    m_weapon->Update(dt);
}

void Unit::UpdateState(const float dt) noexcept {
    const auto direction { m_body->GetDirection() };

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