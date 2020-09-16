#include "PauseNode.hpp"
#include "LevelScene.hpp"
#include "ui/CocosGUI.h"

PauseNode* PauseNode::create() {
    auto pRet = new (std::nothrow) PauseNode();
    if( pRet && pRet->init() ) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool PauseNode::init() {
    if(!cocos2d::Node::init()) {
        return false;
    }
    
    this->setName("Pause");

    auto restartButton = cocos2d::ui::Button::create("normal_restart.png", "selected_restart.png", "disabled_restart.png");
    restartButton->addTouchEventListener([&](cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
            switch (type) {
                case cocos2d::ui::Widget::TouchEventType::BEGAN: break;
                case cocos2d::ui::Widget::TouchEventType::ENDED: {
                    const auto scene = cocos2d::Director::getInstance()->getRunningScene();
                    const auto level = scene->getChildByName("Level");
                    level->resume();
                    dynamic_cast<LevelScene*>(level)->Restart();
                    scene->getChildByName("Interface")->removeAllChildren();
                } break;
                default: break;
            }
    });
    this->addChild(restartButton);

    const auto resumeButton = cocos2d::ui::Button::create("normal_resume.png", "selected_resume.png", "disabled_resume.png");
    resumeButton->addTouchEventListener([&](cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
            switch (type) {
                case cocos2d::ui::Widget::TouchEventType::BEGAN: break;
                case cocos2d::ui::Widget::TouchEventType::ENDED: {
                    const auto scene = cocos2d::Director::getInstance()->getRunningScene();
                    const auto level = scene->getChildByName("Level");
                    level->resume();
                    scene->getChildByName("Interface")->removeAllChildren();
                } break;
                default: break;
            }
    });
    this->addChild(resumeButton);

    const auto size = this->getContentSize();

    restartButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    const cocos2d::Vec2 restartShift{ 0.f, restartButton->getContentSize().height / 3.f};
    restartButton->setPosition( restartShift + size / 2.f );

    resumeButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    const cocos2d::Vec2 resumeShift{ 0.f, -restartButton->getContentSize().height / 3.f};
    resumeButton->setPosition( resumeShift + size / 2.f );

    return true;
}

