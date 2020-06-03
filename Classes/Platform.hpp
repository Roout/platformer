#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"
#include "cocos2d.h"

class Platform final : public core::Entity {
public:
    
    Platform(PhysicWorld * const world, float x, float y, float w, float h) :
        m_world { world },
        m_body { 
            m_world->Create<StaticBody>(
                [x,y](core::Entity * ) {
                    cocos2d::log("Platform at [%f, %f] collide with some entity!", x / 80.f, y / 80.f );
                }, 
                cocos2d::Vec2{ x, y }, cocos2d::Size{ w, h } 
            ) 
        }
    {
        m_body->EmplaceFixture(this, core::CategoryName::PLATFORM);
        m_body->SetMask(
            CreateMask(CategoryBits::PLATFORM),
            CreateMask(CategoryBits::ENEMY, CategoryBits::HERO, CategoryBits::PROJECTILE, CategoryBits::BOUNDARY) 
        );
    }

private:
    PhysicWorld * const m_world { nullptr };
    
    StaticBody * const m_body { nullptr };
};

#endif // PLATFORM_HPP