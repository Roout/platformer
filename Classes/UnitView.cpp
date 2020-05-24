#include "UnitView.hpp"
#include "Unit.hpp"
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
    this->setPosition(shape.origin);
    
    auto factory = dragonBones::CCFactory::getFactory();
    factory->loadDragonBonesData("axe-warrior-3/walk_ske.json");
    factory->loadTextureAtlasData("axe-warrior-3/walk_tex.json");
    auto armatureDisplay = factory->buildArmatureDisplay("Armature", "walk");
    armatureDisplay->setScale( 0.2f );
    // factory->loadDragonBonesData("mecha_1002_101d_show/mecha_1002_101d_show_ske.dbbin");
    // factory->loadTextureAtlasData("mecha_1002_101d_show/mecha_1002_101d_show_tex.json");
    // const auto armatureDisplay = factory->buildArmatureDisplay("mecha_1002_101d", "mecha_1002_101d_show");
    armatureDisplay->getAnimation()->play("walk");
    armatureDisplay->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
    // this->setScaleX(-1.f);
    // armatureDisplay->setScaleX(-1.f);


    this->addChild(armatureDisplay);
    const auto box { armatureDisplay->getBoundingBox() }; 
    // this->drawSolidRect(cocos2d::Vec2::ZERO, shape.size, cocos2d::Color4F::MAGENTA );
    this->drawRect(cocos2d::Vec2::ZERO , box.size, cocos2d::Color4F::MAGENTA );

    // armatureDisplay->getAnimation()->play("walk");
    return true;
}

void HeroView::update(float dt) {
    const auto body { m_model->GetBody() };
    const auto shape { body->GetShape() };
    this->setPosition(shape.origin);
    
}
 
HeroView::HeroView(const Unit* model) : 
    m_model { model } 
{}