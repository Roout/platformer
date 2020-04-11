#include "Unit.hpp"

Unit::Unit(PhysicWorld * const world, float x, float y) : 
    m_body { factory::body_creator<KinematicBody>(world) ( 
        cocos2d::Vec2{ x, y }, 
        cocos2d::Size{ m_width, m_height }, this) 
    }
{
    m_body->SetMask(
        CreateMask(CategoryBits::HERO),
        CreateMask(CategoryBits::ENEMY, CategoryBits::BOUNDARY) 
    );
}


void Unit::MeleeAttack() noexcept {
    // create rectangle/circle shape (!not physic body) using weapon attack range
    // detect collision and identify attack targets:
    //     go through all enemies around
    //         check for intersection with their shape
    //         if intersection occure
    //             add enemy to affected targets
    // foreach affected target
    //     unit->apply_weapon_affect(target) // deal damage
}