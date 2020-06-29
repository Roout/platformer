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
    body->setMass(1.f);
    this->addComponent(body);

    // Last update data are similar to current model's data
    // on initialization.
    /// TODO: initialize this to default values as the unit was just created!
    m_lastState.direction = body->getVelocity();
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

void HeroView::FlipX(const cocos2d::Vec2& currentDir) {
    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );

    if(const auto isTurnedRight = armatureDisplay->getArmature()->getFlipX(); 
        isTurnedRight && currentDir.x < 0.f
    ) {
        armatureDisplay->getArmature()->setFlipX(false);
    } 
    else if( !isTurnedRight && currentDir.x > 0.f) { 
        armatureDisplay->getArmature()->setFlipX(true);
    }
}

void HeroView::UpdateAnimation() {
    const auto body { this->getPhysicsBody() };

    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );
    
    const auto currentState { m_model->GetState() };
    const auto currentDir   { body->getVelocity() };

    if( currentState == m_lastState.state ) 
    { // state is same, checking for state details
        if( !helper::HaveSameSigns(m_lastState.direction.x, currentDir.x ) ) 
        { // direction has been changed, adjust animation:
            if( currentDir.x != 0.f ) {
                // unit moving in another direction
                this->FlipX(currentDir);
            } else {
                // the model doesn't move along x-Axis but stayed still!
                // TODO: add another animation
            }
            // update last state:
            m_lastState.direction = currentDir;
        }
        // otherwise directions are equel and nothing really changed!
    } else { // state has been changed.
        /// TODO: flipX can be seperated cuz now it's same for equel and different states! 
        // adjust view to the new state
        if( currentState == Unit::State::move ) {
            // invert the image if needed
            if( !helper::HaveSameSigns(m_lastState.direction.x, currentDir.x ) ) {
                this->FlipX(currentDir);
            }
        }
        else if ( currentState == Unit::State::jump ) {
            // do nothing ..
        }

        // update last state
        m_lastState.state       = currentState;
        m_lastState.direction   = currentDir;
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