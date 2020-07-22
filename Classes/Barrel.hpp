#ifndef BARREL_HPP
#define BARREL_HPP

#include "Core.hpp"
#include "cocos2d.h"
#include "SizeDeducer.hpp"

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

class Barrel final : public cocos2d::Node {
public:

   static Barrel * create() {
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

    bool init() override {
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

    void update(float dt) override {
        if(m_isExploded) {
            m_timeBeforeErasure -= dt;
            if(m_timeBeforeErasure <= 0.f ) {
                this->removeFromParentAndCleanup(true);
            }
        }
    }

    void Explode() {
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

private:
    
    Barrel() {
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

     /**
     * Time which shows how long will the BarrelView exist 
     * since model was destroyed and @m_model pointer is dangling. 
     */
    float m_timeBeforeErasure { 0.485f };

    bool m_isExploded { false };

private:

    static constexpr float m_width { 55.f };
    static constexpr float m_height { 120.f };
};

#endif // BARREL_HPP