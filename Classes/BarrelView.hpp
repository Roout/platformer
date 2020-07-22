#ifndef BARREL_VIEW_HPP
#define BARREL_VIEW_HPP

#include "Barrel.hpp"
#include "SizeDeducer.hpp"
#include "cocos2d.h"

#include "PhysicsHelper.hpp"

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

class BarrelView final : public cocos2d::DrawNode {
public:
    static BarrelView * create(Barrel* const model) {
        auto pRet = new (std::nothrow) BarrelView(model);
        if (pRet && pRet->init()) {
            pRet->autorelease();
        }
        else {
            delete pRet;
            pRet = nullptr;
        }
        return pRet;
    }

    [[nodiscard]] bool init() override {
        if( !cocos2d::DrawNode::init() ) {
            return false;
        }
        this->scheduleUpdate(); 

        const auto bodySize { m_model->GetSize() };
        auto body = cocos2d::PhysicsBody::createBox(bodySize,
            cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
            {0.f, bodySize.height / 2.f}
        );
        body->setDynamic(false);
        this->addComponent(body);


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
        // armatureDisplay->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
        
        this->addChild(armatureDisplay);

        // adjust animation
        armatureDisplay->setScale( adjustedScaleFactor );
        armatureDisplay->getAnimation()->play("idle");

        // debug
        const auto box { armatureDisplay->getBoundingBox() }; 
        this->drawRect( 
            cocos2d::Vec2 { -box.size.width / 2.f, 0.f }, 
            cocos2d::Vec2 { box.size.width / 2.f, box.size.height }, 
            cocos2d::Color4F::MAGENTA
        );

        // add state lable:
        auto state = cocos2d::Label::createWithTTF("state", "fonts/arial.ttf", 25);
        state->setName("state");
        state->setString("idle");
        //state->setAnchorPoint();
        state->setPosition(0.f, bodySize.height + 30.f);
        this->addChild(state);
        return true;
    }

    void update([[maybe_unused]] float dt) override {
        if( m_exploded ) {
            m_timeBeforeErasure -= dt;
        } 
    }

    void Explode() {
        m_model->RecieveDamage(1);

        m_exploded = true;

        // update animation
        auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
            this->getChildByName("BarrelArmature")
        );
        armatureDisplay->getAnimation()->play("strike", 1);

        // debug:
        auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
        if( !stateLabel ) throw "can't find label!";
        stateLabel->setString("exploded");
    }

    bool Finished() const noexcept {
        return m_timeBeforeErasure <= 0.f;
    }

private:

    BarrelView(Barrel* model) : 
        m_model { model }
    {}

private:

    Barrel * const m_model { nullptr };

    /**
     * Time which shows how long will the BarrelView exist 
     * since model was destroyed and @m_model pointer is dangling. 
     */
    float m_timeBeforeErasure { 0.485f };

    bool m_exploded { false };

};


#endif // BARREL_VIEW_HPP