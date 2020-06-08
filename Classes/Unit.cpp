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

    const float damage { 10.f };
    const float range { 20.f };
    const float reloadTime { 0.2f };

    m_weapon = std::make_unique<Sword>( damage, range, reloadTime );
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
    cocos2d::log(" >>> unit recieve %d damage. Current health is %d.", damage, m_health);
}

void Unit::MeleeAttack() noexcept {
    if( m_weapon->CanAttack() ) {
        const auto direction = m_body->GetDirection();
        auto position = m_body->GetShape().origin;
        if(direction.x >= 0.f) {
            position.x += m_width;
        }
        else if(direction.x < 0.f) {
            position.x -= m_width;
        }

        static int x { 0 };
        cocos2d::log(" >>> unit attack with sword: %d", ++x );

        m_weapon->Attack(m_world, position, direction);
    }
}

void Unit::UpdateWeapon(const float dt) noexcept {
    m_weapon->Update(dt);
}

void Unit::UpdateState() noexcept {
    const auto direction { m_body->GetDirection() };

    if( direction.y != 0.f ) {
        m_state = State::jump;
    } else if( direction.x != 0.f ) {
        m_state = State::move;
    } else {
        m_state = State::idle;
    }
}