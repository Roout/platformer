#include "Projectile.hpp"
#include "Utils.hpp"
#include "Core.hpp"

Projectile * Projectile::create(float damage) {
    auto pRet = new (std::nothrow) Projectile(damage);
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

Projectile::Projectile(float damage) :
    m_lifeTime { 0.15f },
    m_damage { damage }
{   
}

void Projectile::SetContactTestBitmask(size_t mask) noexcept {
    const auto body = this->getPhysicsBody();
    if(body) {
        body->setContactTestBitmask(mask);
    }
}

void Projectile::SetCategoryBitmask(size_t mask) noexcept {
    const auto body = this->getPhysicsBody();
    if(body) {
        body->setCategoryBitmask(mask);
    }
}

cocos2d::Sprite* Projectile::AddImage(const char* imagePath) {
    auto textureCache = cocos2d::Director::getInstance()->getTextureCache();
    auto texture = textureCache->getTextureForKey(imagePath);
    if (texture == nullptr) {
        texture = textureCache->addImage(imagePath);
    }
    auto sprite = cocos2d::Sprite::createWithTexture(texture);
    sprite->setAnchorPoint({0.0f, 0.0f});

    this->addChild(sprite, 10); /// TODO: organize Z-order!
    return sprite;
}

cocos2d::PhysicsBody* Projectile::AddPhysicsBody(const cocos2d::Size& size) {
    const auto body = cocos2d::PhysicsBody::createBox(size);
    body->setDynamic(true);
    body->setGravityEnable(false);
    this->setContentSize(size);
    this->addComponent(body);
    return body;
}