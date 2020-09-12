#include "Projectile.hpp"
#include "Utils.hpp"
#include "Core.hpp"

Projectile * Projectile::create(
    const cocos2d::Size& size,
    const cocos2d::Vec2& velocity,
    const float damage
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

Projectile * Projectile::create(
    const char * imagePath,
    const cocos2d::Vec2& velocity,
    const float damage
) {
    auto pRet = new (std::nothrow) Projectile(imagePath, velocity, damage);
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
        this->removeComponent(this->getPhysicsBody());
        this->runAction(cocos2d::RemoveSelf::create(true));
    }
}

Projectile::Projectile (
    const cocos2d::Size& size,
    const cocos2d::Vec2& velocity,
    const float damage
) : 
    m_lifeTime { 0.15f },
    m_damage { damage }
{    
    const auto body = cocos2d::PhysicsBody::createBox(size);
    body->setVelocity(velocity);
    body->setDynamic(true);
    body->setGravityEnable(false);
    body->setRotationEnable(false);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::PROJECTILE)
    );
    this->setContentSize(size);
    this->addComponent(body);
}

Projectile::Projectile(
    const char* imagePath,
    const cocos2d::Vec2& velocity,
    const float damage
) :
    m_lifeTime { 0.15f },
    m_damage { damage }
{   
    const auto image = this->AddImage(imagePath);
    const auto size { this->getContentSize() };
    if(velocity.x > 0.f) {
        image->setFlippedX(true);
    }

    const auto body = cocos2d::PhysicsBody::createBox(size);
    body->setVelocity(velocity);
    body->setDynamic(true);
    body->setGravityEnable(false);
    body->setRotationEnable(false);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::PROJECTILE)
    );
    this->setContentSize(size);
    this->addComponent(body);
}

void Projectile::SetContactTestBitmask(size_t mask) noexcept {
    const auto body = this->getPhysicsBody();
    if(body) {
        body->setContactTestBitmask(mask);
    }
}

cocos2d::Sprite* Projectile::AddImage(const char* imagePath) {
    auto textureCache = cocos2d::Director::getInstance()->getTextureCache();
    auto texture = textureCache->getTextureForKey(imagePath);
    if (texture == nullptr) {
        texture = textureCache->addImage(imagePath);
    }
    auto sprite = cocos2d::Sprite::createWithTexture(texture);

    const auto scaleFactor { 0.4f };
    sprite->setScale(scaleFactor);
    sprite->setAnchorPoint({0.0f, 0.0f});
    sprite->setName("arrow");
    
    this->setContentSize(sprite->getContentSize() * scaleFactor);
    this->addChild(sprite, 10); /// TODO: organize Z-order!
    
    return sprite;
}
