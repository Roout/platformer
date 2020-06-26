#include "HealthBar.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"

HealthBar * HealthBar::create( const std::shared_ptr<Unit>& model) {
    auto pRet = new (std::nothrow) HealthBar(model);
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

    if(const auto unit { m_model.lock() }; unit != nullptr ) {
        if(const auto body { unit->GetBody() }; body != nullptr) {
            this->scheduleUpdate(); 

            const auto size = unit->GetSize();
            const auto barSize { cocos2d::Size(size.width, 20.f) };

            auto boarder = cocos2d::DrawNode::create();
            boarder->drawRect(
                cocos2d::Vec2::ZERO, 
                cocos2d::Vec2{barSize.width, barSize.height},
                cocos2d::Color4F::BLACK
            );
            boarder->setContentSize(barSize);
            boarder->setLineWidth(2.f);
            this->addChild(boarder, 1);

            auto health = cocos2d::DrawNode::create();
            health->setName("health");
            health->setContentSize(barSize);
            health->drawSolidRect(
                cocos2d::Vec2::ZERO, 
                cocos2d::Vec2{barSize.width, barSize.height},
                cocos2d::Color4F::RED
            );
            this->addChild(health, 0);

            m_maxHealth = unit->GetHealth();
            return true;
        }
    }

    return false;
}

void HealthBar::update(float dt) {
    if(const auto unit { m_model.lock() }; unit != nullptr ) {
        auto healthNode { dynamic_cast<cocos2d::DrawNode*>(this->getChildByName("health")) };
        const auto cSize { healthNode->getContentSize() };
        const auto newWidth { unit->GetHealth() * cSize.width / m_maxHealth };

        // clear previous health bar
        healthNode->clear();

        // draw mew health bar
        healthNode->drawSolidRect(
            cocos2d::Vec2::ZERO, 
            cocos2d::Vec2{newWidth, cSize.height},
            cocos2d::Color4F::RED
        );
    }
}

HealthBar::HealthBar(const std::shared_ptr<Unit>& model) :
    m_model{model}
{
}

