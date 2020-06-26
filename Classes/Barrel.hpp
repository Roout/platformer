#ifndef BARREL_HPP
#define BARREL_HPP

#include "Core.hpp"
#include "cocos2d.h"

class Barrel final : public core::Entity {
public:
    Barrel( 
        cocos2d::PhysicsBody * const body, 
        const cocos2d::Size& size 
    ) : 
        m_body { body },
        m_size { size }
    {   
        m_body->setDynamic(false);
        m_body->setCategoryBitmask(
            core::CreateMask(
                core::CategoryBits::BARREL
            )
        );
        m_body->setCollisionBitmask(
            core::CreateMask(
                core::CategoryBits::BOUNDARY, 
                core::CategoryBits::PROJECTILE
            )
        );
    }

    ~Barrel() = default;
    
    void RecieveDamage( [[maybe_unused]] int damage) noexcept override {
        m_isExploded = true;
    } 

    bool IsExploded() const noexcept {
        return m_isExploded;
    }

    const cocos2d::PhysicsBody * GetBody() const noexcept {
        return m_body;
    }

    /**
     * Return size of the barrel 
     * which was already adjusted to the resolution difference
     */
    const cocos2d::Size GetSize() const noexcept {
        return m_size;
    }

private:
    cocos2d::PhysicsBody * m_body { nullptr };

    /**
     * Keep size of the barrel which was already adjusted to the resolution difference
     */
    const cocos2d::Size m_size;

    bool m_isExploded { false };
};

#endif // BARREL_HPP