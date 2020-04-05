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
    return true;
}

void HeroView::update(float dt) {
    const auto shape { m_model->GetBody()->GetShape() };
    this->setPosition(shape.origin);
}
 
HeroView::HeroView(const Unit* model): 
    m_model { model } 
{}