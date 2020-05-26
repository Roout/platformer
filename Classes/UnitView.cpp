#include "UnitView.hpp"
#include "PhysicWorld.hpp"

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"


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

    const auto body { m_model->GetBody() };
    const auto shape { body->GetShape() };
    this->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    this->setPosition(shape.origin + cocos2d::Vec2{ shape.size.width / 2.f, 0.f });
    
    // load animation data and build the armature
    const auto factory = dragonBones::CCFactory::getFactory();
    factory->loadDragonBonesData("axe-warrior/walk_ske.json");
    factory->loadTextureAtlasData("axe-warrior/walk_tex.json");
    auto armatureDisplay = factory->buildArmatureDisplay("Armature", "walk");

    // TODO: scale factor depends on device resolution so it can'be predefined constant.
    constexpr auto scaleFactor { 0.2f };
    
    armatureDisplay->setName("Armature");
    // armatureDisplay->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    
    this->addChild(armatureDisplay);

    // adjust animation
    armatureDisplay->setScale( scaleFactor );
    armatureDisplay->getAnimation()->play("walk");

    // debug
    const auto box { armatureDisplay->getBoundingBox() }; 
    this->drawRect( 
        cocos2d::Vec2 { -box.size.width / 2.f, 0.f }, 
        cocos2d::Vec2 { box.size.width / 2.f, box.size.height }, 
        cocos2d::Color4F::MAGENTA 
    );
    this->drawRect( 
        cocos2d::Vec2 { -shape.size.width / 2.f, 0.f }, 
        cocos2d::Vec2 { shape.size.width / 2.f, shape.size.height }, 
        cocos2d::Color4F::YELLOW
    );
    return true;
}

inline std::string CreateAnimationName( Unit::State state) {
    std::string animationName { "walk" };
    switch (state)
    {
        case Unit::State::idle: break;
        case Unit::State::jump: break;
        case Unit::State::move: break;
        case Unit::State::attack: break; 
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
    const auto body { m_model->GetBody() };

    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );
    
    const auto currentState { m_model->GetState() };
    const auto currentDir   { body->GetDirection() };

    if( currentState == m_lastState.state ) 
    { // state is same, checking for state details
        if( m_lastState.direction.x != currentDir.x ) 
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
        // adjust view to the new state
        if( currentState == Unit::State::move) {
            // invert the image if needed
            if( ( currentDir.x > 0.f && m_lastState.direction.x <= 0.f ) ||
                ( currentDir.x < 0.f && m_lastState.direction.x >= 0.f )
            ) {
                this->FlipX(currentDir);
            }
        }

        // update last state
        m_lastState.state       = currentState;
        m_lastState.direction   = currentDir;
        // run animation
        const std::string animationName { CreateAnimationName(currentState) };
        armatureDisplay->getAnimation()->play(animationName);
    }
}

void HeroView::update(float dt) {
    // Update animation
    this->UpdateAnimation();

    const auto body { m_model->GetBody() };
    const auto shape { body->GetShape() };
    this->setPosition(shape.origin + cocos2d::Vec2{ shape.size.width / 2.f, 0.f });
}
 
HeroView::HeroView(const Unit* model) : 
    m_model { model }
{
    const auto body { m_model->GetBody() };
    // Last update data are similar to current model's data
    // on initialization.
    m_lastState.direction = body->GetDirection();
    m_lastState.state = m_model->GetState();
}