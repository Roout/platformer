#include "HealthBar.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"

HealthBar * HealthBar::create( const Unit* const unit) {
    auto pRet = new (std::nothrow) HealthBar(unit);
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool HealthBar::init() {
    if( !cocos2d::DrawNode::init() ) {
        return false;
    }
    this->scheduleUpdate(); 

    const auto size = m_unit->getContentSize();
    const auto barSize { cocos2d::Size(size.width, 15.f) };

    const auto boarder = cocos2d::DrawNode::create();
    boarder->drawRect(
        cocos2d::Vec2::ZERO, 
        cocos2d::Vec2{barSize.width, barSize.height},
        cocos2d::Color4F::BLACK
    );
    boarder->setContentSize(barSize);
    boarder->setLineWidth(2.f);
    this->addChild(boarder, 1);

    const auto health = cocos2d::DrawNode::create();
    health->setName("health");
    health->setContentSize(barSize);
    health->drawSolidRect(
        cocos2d::Vec2::ZERO, 
        cocos2d::Vec2{barSize.width, barSize.height},
        cocos2d::Color4F::RED
    );
    this->addChild(health, 0);
    this->setContentSize(barSize);
    m_maxHealth = m_unit->GetHealth();
    return true;
}

void HealthBar::update(float dt) {
    /// TODO: add as private property of class to avoid cast
    const auto healthNode { this->getChildByName<cocos2d::DrawNode*>("health") };
    const auto cSize { healthNode->getContentSize() };
    const auto newWidth { m_unit->GetHealth() * cSize.width / m_maxHealth };

    // clear previous health bar
    healthNode->clear();

    // draw mew health bar
    healthNode->drawSolidRect(
        cocos2d::Vec2::ZERO, 
        cocos2d::Vec2{newWidth, cSize.height},
        cocos2d::Color4F::RED
    );
}

HealthBar::HealthBar(const Unit* const unit) :
    m_unit{ unit }
{
}

