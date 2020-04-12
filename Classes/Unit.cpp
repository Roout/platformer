#include "Unit.hpp"

Unit::Unit(PhysicWorld * const world, float x, float y) :
    m_world { world },
    m_body { factory::body_creator<KinematicBody>(world) ( 
        cocos2d::Vec2{ x, y }, 
        cocos2d::Size{ m_width, m_height }, this) 
    },
    m_health { 100 }
{
    m_body->SetMask(
        CreateMask(CategoryBits::HERO),
        CreateMask(CategoryBits::ENEMY, CategoryBits::BOUNDARY) 
    );
    m_weapon = std::make_unique<Sword>(10, 20, 0.2f);
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
}


void Unit::MeleeAttack() noexcept {
    if( m_weapon->CanAttack() ) {
        const auto direction = m_body->GetDirection();
        auto position = m_body->GetShape().origin;
        position.x += m_state == State::idle_left? -m_width: m_width;
        
        m_weapon->Attack(m_world, position, direction);
    }
}