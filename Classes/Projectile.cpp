#include "Projectile.hpp"

#include "Utils.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"

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
    if (!cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate();

    return true;
};

void Projectile::update(float dt) {
    cocos2d::Node::update(dt);
    this->UpdateLifetime(dt);
    this->UpdateState(dt);
    if (m_animator) {
        this->UpdateAnimation(); 
    }
    this->UpdatePhysicsBody();
}

void Projectile::pause() {
    cocos2d::Node::pause();
    if (m_animator) {
        m_animator->pause();
    }
}
    
void Projectile::resume() {
    cocos2d::Node::resume();
    if (m_animator) {
        m_animator->resume();
    }
}

void Projectile::UpdatePhysicsBody() noexcept {
    if (m_currentState != m_previousState && m_currentState == State::EXPLODED) {
        this->removeComponent(this->getPhysicsBody());
    }
}

void Projectile::UpdateLifetime(const float dt) noexcept {
    if (m_lifeTime > 0.f) {
        m_lifeTime -= dt;
    }
}

void Projectile::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_lifeTime <= 0.f) {
        m_currentState = State::EXPLODED;
        if (!m_animator) {
            this->runAction(cocos2d::RemoveSelf::create(true));
        }
    }
    else {
        m_currentState = State::IDLE;
    }
}

void Projectile::FlipX() noexcept {
    if (m_animator) {
        m_animator->FlipX();
    }
    else if (m_image) {
        m_image->setFlippedX(!m_image->isFlippedX());
    }
}

void Projectile::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        const auto isOneTimeAttack { m_currentState == State::EXPLODED };
        const auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(!this->IsAlive()) {
            m_animator->EndWith([this](){ 
                this->runAction(cocos2d::RemoveSelf::create(true));
            });
        }
    }
}

void Projectile::AddAnimator(std::string chachedArmatureName) {
    m_animator = dragonBones::Animator::create(std::move(chachedArmatureName));
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::IDLE), "walk"),       // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(State::EXPLODED), "attack")  // sorry the illustrator is a little bit of an idiot
    });
    m_animator->setPosition({0.f, 0.f});
    m_animator->setAnchorPoint({0.f, 0.f});
    this->addChild(m_animator);
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
    m_image = cocos2d::Sprite::createWithTexture(texture);
    m_image->setAnchorPoint({0.0f, 0.0f});

    this->addChild(m_image, 10); /// TODO: organize Z-order!
    return m_image;
}

cocos2d::PhysicsBody* Projectile::AddPhysicsBody(const cocos2d::Size& size) {
    const auto body = cocos2d::PhysicsBody::createBox(size);
    body->setDynamic(true);
    body->setGravityEnable(false);
    this->setContentSize(size);
    this->addComponent(body);
    return body;
}