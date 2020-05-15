#ifndef BORDER_HPP
#define BORDER_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"
#include "cocos2d.h"

class Border final : public core::Entity {
public:

    Border(PhysicWorld * const world, float x, float y, float w, float h) :
        m_world { world },
        m_body { cocos2d::Vec2{ x, y }, cocos2d::Size{ w, h } }
    {
        m_world->Add<StaticBody>(&m_body, [x,y](core::Entity * ) {
            cocos2d::log("Border at [%f, %f] collide with some entity!", x, y);
        });
        m_body.EmplaceFixture(this, core::CategoryName::BORDER);
        m_body.SetMask(
            CreateMask(CategoryBits::BOUNDARY),
            CreateMask(CategoryBits::ENEMY, CategoryBits::HERO, CategoryBits::PROJECTILE, CategoryBits::PLATFORM) 
        );
    }

    ~Border() {
        m_world->Erase<StaticBody>(&m_body);
    }

private:
    PhysicWorld * const m_world { nullptr };

    StaticBody m_body;
};

#endif // BORDER_HPP