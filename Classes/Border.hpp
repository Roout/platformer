#ifndef BORDER_HPP
#define BORDER_HPP

#include "Core.hpp"
#include "cocos2d.h"
#include "Utils.hpp"

class Border final {
public:

    Border(cocos2d::PhysicsBody * const body) 
        : m_body { body }
    {   
        m_body->setDynamic(false);
        m_body->setCategoryBitmask(
            Utils::CreateMask(
                core::CategoryBits::BOUNDARY
            )
        );
        m_body->setCollisionBitmask(
            Utils::CreateMask(
                core::CategoryBits::ENEMY, 
                core::CategoryBits::HERO, 
                core::CategoryBits::PROJECTILE, 
                core::CategoryBits::PLATFORM
            )
        );
        m_body->setContactTestBitmask(
            Utils::CreateMask(core::CategoryBits::HERO_SENSOR)
        );
    }

    ~Border() = default;
    
    const cocos2d::PhysicsBody * GetBody() const noexcept {
        return m_body;
    }

private:
    cocos2d::PhysicsBody * m_body { nullptr };
};

#endif // BORDER_HPP