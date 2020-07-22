#include "Projectile.hpp"
#include "Utils.hpp"
#include "Core.hpp"

Projectile * Projectile::create(
    const cocos2d::Size& size,
    const cocos2d::Vec2& velocity,
    const int damage
) {
    auto pRet = new (std::nothrow) Projectile(size, velocity, damage);
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Projectile::init() {
    if( !cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate();

    return true;
};

void Projectile::update(float dt) {
    if (m_lifeTime > 0.f) {
        m_lifeTime -= dt;
    }
    if(m_lifeTime <= 0.f) {
        this->removeFromParentAndCleanup(true);
    }
}

Projectile::Projectile (
    const cocos2d::Size& size,
    const cocos2d::Vec2& velocity,
    const int damage
) : 
    m_lifeTime { 0.15f },
    m_damage { damage }
{    
    auto body = cocos2d::PhysicsBody::createBox(size);
    body->setVelocity(velocity);
    body->setDynamic(false);
    body->setGravityEnable(true);
    body->setRotationEnable(false);
    body->setCategoryBitmask(
        Utils::CreateMask(
            core::CategoryBits::PROJECTILE
        )
    );
    const auto interactWith { 
        Utils::CreateMask(
            core::CategoryBits::ENEMY, 
            /// TODO: add this when unit will be inherit as HERO or ENEMY and define categoty it collide with
            // core::CategoryBits::HERO,  
            core::CategoryBits::BARREL, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE 
        )
    };
    body->setCollisionBitmask(interactWith);
    body->setContactTestBitmask(interactWith);

    /**
     * Anchor point is choosen same as unit it's attached to.
     */
    this->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    this->setContentSize(size);
    this->addComponent(body);
}
