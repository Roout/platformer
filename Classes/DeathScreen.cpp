#include "DeathScreen.hpp"
#include "LevelScene.hpp"
#include "ui/CocosGUI.h"


DeathScreen* DeathScreen::create() {
    auto pRet = new (std::nothrow) DeathScreen();
    if( pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool DeathScreen::init() {
    if( !cocos2d::Node::init() ) {
        return false;
    }

    this->setName(NAME);

    const auto restartButton = cocos2d::ui::Button::create(
        "normal_restart.png", 
        "selected_restart.png", 
        "disabled_restart.png"
    );
    restartButton->addTouchEventListener([](cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
            switch (type) {
                case cocos2d::ui::Widget::TouchEventType::BEGAN: break;
                case cocos2d::ui::Widget::TouchEventType::ENDED: {
                    auto scene = cocos2d::Director::getInstance()->getRunningScene();
                    auto level = scene->getChildByName("Level");
                    if(level != nullptr) {
                        dynamic_cast<LevelScene*>(level)->Restart();
                    }
                    scene->getChildByName("Interface")->removeAllChildren();
                } break;
                default: break;
            }
    });
    this->addChild(restartButton);

    const auto size = this->getContentSize();

    restartButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    restartButton->setPosition( size / 2.f );

    return true;
}
