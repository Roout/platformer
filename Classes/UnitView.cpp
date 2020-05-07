#include "UnitView.hpp"
#include "Unit.hpp"
#include "PhysicWorld.hpp"

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
    this->drawSolidRect(cocos2d::Vec2::ZERO, shape.size, cocos2d::Color4F::MAGENTA );

    auto label = cocos2d::Label::createWithSystemFont(
        (body->IsOnGround()? "ground": "no ground"), 
        "Arial", 
        18
    );
    this->addChild(label);
    label->setPosition(cocos2d::Vec2(shape.size.width / 2.f, shape.size.height + 5.f));
    label->setName("info");
    
    return true;
}

void HeroView::update(float dt) {
    const auto body { m_model->GetBody() };
    const auto shape { body->GetShape() };
    this->setPosition(shape.origin);
    auto label = (cocos2d::Label*)this->getChildByName("info");
    label->setString(body->IsOnGround()? "ground": "no ground");
}
 
HeroView::HeroView(const Unit* model): 
    m_model { model } 
{}