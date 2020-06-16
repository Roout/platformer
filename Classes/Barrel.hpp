#ifndef BARREL_HPP
#define BARREL_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"
#include "cocos2d.h"

class Barrel final : public core::Entity {
public:
    
    Barrel(PhysicWorld * const world, float x, float y, float w, float h) :
        m_world { world }
    {   
        const auto callback = [x,y](core::Entity * ) {
            cocos2d::log("Barrel at [%f, %f] collide with some entity!", x, y);
        };
        m_body = m_world->Create<StaticBody>(
            callback,
            cocos2d::Vec2{ x, y }, cocos2d::Size{ w, h } 
        );
        m_body->EmplaceFixture(this, core::CategoryName::BARREL);
        m_body->SetMask(
            CreateMask(CategoryBits::BOUNDARY),
            CreateMask(CategoryBits::PROJECTILE) 
        );
    }

    ~Barrel() {
        m_world->Erase(m_body);
    }
    
    void RecieveDamage( [[maybe_unused]] int damage) noexcept override {
        m_isExploded = true;
    } 

    bool IsExploded() const noexcept {
        return m_isExploded;
    }

    const StaticBody * GetBody() const noexcept {
        return m_body;
    } 

private:
    
    PhysicWorld * const m_world { nullptr };

    StaticBody * m_body { nullptr };

    bool m_isExploded { false };
};

#endif // BARREL_HPP