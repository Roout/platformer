#include "UnitView.hpp"
#include "PhysicsHelper.hpp"

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

#include "SizeDeducer.hpp"

HeroView * HeroView::create(const Unit* const model) {
    auto pRet = new (std::nothrow) HeroView(model);
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool HeroView::init() {
    if( !cocos2d::DrawNode::init() ) {
        return false;
    }
    this->scheduleUpdate();

    const auto unitBodySize = m_model->GetSize();
    auto body = cocos2d::PhysicsBody::createBox(
        unitBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
        {0.f, unitBodySize.height / 2.f}
    );
    body->setMass(25.f);
    this->addComponent(body);

    // Last update data are similar to current model's data
    // on initialization.
    /// TODO: initialize this to default values as the unit was just created!
    m_lastState.direction = m_model->GetSide();
    m_lastState.state = m_model->GetState();
    
    // load animation data and build the armature
    /// TODO: move this to function
    const auto factory = dragonBones::CCFactory::getFactory();
    if(auto bonesData = factory->getDragonBonesData("mc"); bonesData == nullptr) {
        factory->loadDragonBonesData("mc/mc_ske.json");
    }
    if(auto texture = factory->getTextureAtlasData("mc"); texture == nullptr) {
        factory->loadTextureAtlasData("mc/mc_tex.json");
    }
    auto armatureDisplay = factory->buildArmatureDisplay("Armature", "mc");

    // TODO: scale factor depends on device resolution so it can'be predefined constant.
    constexpr auto designedScaleFactor { 0.2f };
    const auto adjustedScaleFactor { 
        SizeDeducer::GetInstance().GetAdjustedSize(designedScaleFactor) 
    };
    armatureDisplay->setName("Armature");
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
    state->setPosition(0.f, unitBodySize.height + 60.f);
    this->addChild(state);
    return true;
}

inline std::string CreateAnimationName( Unit::State state) {
    std::string animationName { "walk" };
    switch (state)
    {
        case Unit::State::idle: animationName = "idle"; break;
        case Unit::State::jump: animationName = "jump"; break;
        case Unit::State::move: animationName = "walk"; break;
        case Unit::State::attack: animationName = "attack"; break; 
        default: break;
    }
    return animationName;
}

void HeroView::FlipX(const Unit::Side currentSide) {
    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );

    if(const auto isTurnedRight = armatureDisplay->getArmature()->getFlipX(); 
        isTurnedRight && currentSide == Unit::Side::left
    ) {
        armatureDisplay->getArmature()->setFlipX(false);
    } 
    else if( !isTurnedRight && currentSide == Unit::Side::right) { 
        armatureDisplay->getArmature()->setFlipX(true);
    }
}

void HeroView::UpdateAnimation() {
    const auto body { this->getPhysicsBody() };

    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );
    
    const auto currentState { m_model->GetState() };
    const auto currentSide  { m_model->GetSide() };

    if( currentState == m_lastState.state ) 
    { // state is same, checking for state details
        if( m_lastState.direction != currentSide ) 
        { // direction has been changed, adjust animation:
            this->FlipX(currentSide);
            // update last state:
            m_lastState.direction = currentSide;
        }
        // otherwise directions are equel and nothing really changed!
    } else { // state has been changed.
        /// TODO: flipX can be seperated cuz now it's same for equel and different states! 
        // adjust view to the new state
        if( currentState == Unit::State::move || currentState == Unit::State::jump ) {
            // invert the image if needed
            if( m_lastState.direction != currentSide ) {
                this->FlipX(currentSide);
            }
        }
        // else if ( currentState == Unit::State::jump ) {
        //     // do nothing ..
        // }

        // update last state
        m_lastState.state       = currentState;
        m_lastState.direction   = currentSide;
        // run animation
        const std::string animationName { CreateAnimationName(currentState) };
        if ( currentState == Unit::State::jump || 
             currentState == Unit::State::attack 
        ) {
            armatureDisplay->getAnimation()->play(animationName, 1);
        } else {
            armatureDisplay->getAnimation()->play(animationName);
        }
    }
}

void HeroView::update(float dt) {
    // Update animation
    this->UpdateAnimation();
    // Debug >> Update state:
    auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    
    if( !stateLabel ) throw "can't find label!";
    stateLabel->setString(CreateAnimationName(m_model->GetState()));
}
 
HeroView::HeroView(const Unit* model) : 
    m_model { model }
{}