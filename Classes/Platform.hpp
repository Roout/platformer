#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "Core.hpp"
#include "cocos2d.h"

class Platform final : public core::Entity {
public:
    
    Platform( cocos2d::PhysicsBody * const body) 
        : m_body { body }
    {   
        m_body->setDynamic(false);
        m_body->setCategoryBitmask(
            core::CreateMask(
                core::CategoryBits::PLATFORM
            )
        );
        m_body->setCollisionBitmask(
            core::CreateMask(
                core::CategoryBits::HERO,
                core::CategoryBits::ENEMY, 
                core::CategoryBits::PROJECTILE
            )
        );
    }

    ~Platform() = default;

    const cocos2d::PhysicsBody * GetBody() const noexcept {
        return m_body;
    }

private:
    cocos2d::PhysicsBody * m_body { nullptr };
};

#endif // PLATFORM_HPP