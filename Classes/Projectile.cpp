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
    const auto body = cocos2d::PhysicsBody::createBox(size);
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
    // body->setCollisionBitmask(interactWith);
    body->setContactTestBitmask(interactWith);

    this->setContentSize(size);
    this->addComponent(body);
}

void Projectile::SetContactTestBitmask(size_t mask) noexcept {
    const auto body = this->getPhysicsBody();
    body->setContactTestBitmask(mask);
}
