#include "Barrel.hpp"
#include "Core.hpp"
#include "SizeDeducer.hpp"
#include "Utils.hpp"
#include <algorithm>

#include "DragonBonesAnimator.hpp"

Barrel * Barrel::create() {
    auto pRet = new (std::nothrow) Barrel();
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Barrel::init() {
    if( !cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate(); 

    // setup animator
    m_animator = dragonBones::Animator::create("barrel"); 
    m_animator->InitializeAnimations({
        std::make_pair<std::size_t, std::string>(Utils::EnumCast(State::idle), "idle"),
        std::make_pair<std::size_t, std::string>(Utils::EnumCast(State::exploded), "strike")
    });
    (void) m_animator->Play(Utils::EnumCast(State::idle), dragonBones::Animator::INFINITY_LOOP);
    this->addChild(m_animator);
    
    m_animator->setScale(0.2f); // TODO: introduce multi-resolution scaling
    this->AddPhysicsBody(m_designedSize);
    this->setContentSize(m_designedSize);
    
    // debug state lable:
    auto state = cocos2d::Label::createWithTTF("state", "fonts/arial.ttf", 25);
    state->setName("state");
    state->setString("idle");
    const auto height { SizeDeducer::GetInstance().GetAdjustedSize(m_designedSize.height + 30.f) };
    state->setPosition(0.f, height);
    this->addChild(state);

    return true;
}

void Barrel::Explode() {
    m_animator->Play(Utils::EnumCast(State::exploded), 1).EndWith([this]() {
        // note: can't use `this->removeFromParent()` cuz it well be execute from this 
        // function and invalidate Animator instance.
        // Approach with action makes sure it will be executed from the parent node. 
        this->runAction(cocos2d::RemoveSelf::create(true));
    });

    this->removeComponent(this->getPhysicsBody());

    // debug:
    auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    if( !stateLabel ) throw "can't find label!";
    stateLabel->setString("exploded");
}

void Barrel::AddPhysicsBody(const cocos2d::Size& size) {
    auto body = cocos2d::PhysicsBody::createBox(size,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
        {0.f, size.height / 2.f}
    );
    body->setDynamic(false);
    body->setCategoryBitmask(
        Utils::CreateMask(
            core::CategoryBits::BARREL
        )
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PROJECTILE
        ) 
    );
    this->addComponent(body);
}

void Barrel::pause() {
    cocos2d::Node::pause();
    m_animator->pause();
}
    
void Barrel::resume() {
    cocos2d::Node::resume();
    m_animator->resume();
}