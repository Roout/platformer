#ifndef BORDER_HPP
#define BORDER_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"
#include "cocos2d.h"

class Border final : public core::Entity {
public:

    Border(PhysicWorld * const world, float x, float y, float w, float h) :
        m_world { world },
        m_body { 
            m_world->Create<StaticBody>(
                [x,y](core::Entity * ) {
                    cocos2d::log("Border at [%f, %f] collide with some entity!", x, y);
                },
                cocos2d::Vec2{ x, y }, cocos2d::Size{ w, h } 
            )
        }
    {
        m_body->EmplaceFixture(this, core::CategoryName::BORDER);
        m_body->SetMask(
            CreateMask(CategoryBits::BOUNDARY),
            CreateMask(CategoryBits::ENEMY, CategoryBits::HERO, CategoryBits::PROJECTILE, CategoryBits::PLATFORM) 
        );
    }

private:
    PhysicWorld * const m_world { nullptr };

    StaticBody * const m_body { nullptr };
};

#endif // BORDER_HPP