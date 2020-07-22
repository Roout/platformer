#include "Barrel.hpp"
#include "Core.hpp"
#include "SizeDeducer.hpp"
#include "Utils.hpp"

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

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

    /// TODO: make armature creation factory function in another file!
    // load animation data and build the armature
    const auto factory = dragonBones::CCFactory::getFactory();

    if(auto bonesData = factory->getDragonBonesData("barrel"); bonesData == nullptr) {
        factory->loadDragonBonesData("barrel/barrel_ske.json");
    }
    if(auto texture = factory->getTextureAtlasData("barrel"); texture == nullptr) {
        factory->loadTextureAtlasData("barrel/barrel_tex.json");
    }
    auto armatureDisplay = factory->buildArmatureDisplay("Armature", "barrel");

    // TODO: scale factor depends on device resolution so it can'be predefined constant.
    constexpr auto designedScaleFactor { 0.2f };
    const auto adjustedScaleFactor { 
        SizeDeducer::GetInstance().GetAdjustedSize(designedScaleFactor) 
    };
    armatureDisplay->setName("BarrelArmature");
    
    this->addChild(armatureDisplay);

    // adjust animation
    armatureDisplay->setScale( adjustedScaleFactor );
    armatureDisplay->getAnimation()->play("idle");

    // add state lable:
    auto state = cocos2d::Label::createWithTTF("state", "fonts/arial.ttf", 25);
    state->setName("state");
    state->setString("idle");
    const auto height { SizeDeducer::GetInstance().GetAdjustedSize(m_height + 30.f) };
    state->setPosition(0.f, height);
    this->addChild(state);

    return true;
}

void Barrel::update(float dt) {
    if(m_isExploded) {
        m_timeBeforeErasure -= dt;
        if(m_timeBeforeErasure <= 0.f ) {
            this->removeFromParentAndCleanup(true);
        }
    }
}

void Barrel::Explode() {
    m_isExploded = true;

    // update animation
    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("BarrelArmature")
    );
    armatureDisplay->getAnimation()->play("strike", 1);

    this->removeComponent(this->getPhysicsBody());

    // debug:
    auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    if( !stateLabel ) throw "can't find label!";
    stateLabel->setString("exploded");
}

Barrel::Barrel() {
    const auto bodySize { 
        cocos2d::Size(
            SizeDeducer::GetInstance().GetAdjustedSize(m_width),
            SizeDeducer::GetInstance().GetAdjustedSize(m_height)
        ) 
    };

    auto body = cocos2d::PhysicsBody::createBox(bodySize,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
        {0.f, bodySize.height / 2.f}
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