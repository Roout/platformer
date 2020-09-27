#include "DebugScreen.hpp"
#include "ui/CocosGUI.h"

namespace screen {

DebugScreen* DebugScreen::create() {
    auto pRet = new (std::nothrow) DebugScreen();
    if( pRet && pRet->init() ) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool DebugScreen::init() {
    namespace ui = cocos2d::ui;

    if(!cocos2d::Node::init()) {
        return false;
    }
    this->setName(NAME);

    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto world = scene->getPhysicsWorld();
    if(world) {
        m_physicsWorldMask = world->getDebugDrawMask();
        m_caption = cocos2d::Label::createWithTTF("Physics debug ", "fonts/arial.ttf", 25);
        m_caption->setTextColor(cocos2d::Color4B::BLACK);

        auto checkbox = ui::CheckBox::create(
            "cocosui/check_box_normal.png",
            "cocosui/check_box_normal_press.png",
            "cocosui/check_box_active.png",
            "cocosui/check_box_normal_disable.png",
            "cocosui/check_box_active_disable.png"
        );

        if(m_physicsWorldMask == cocos2d::PhysicsWorld::DEBUGDRAW_ALL) {
            checkbox->setSelected(true);
        }
        checkbox->addTouchEventListener([this](cocos2d::Ref* sender, ui::Widget::TouchEventType type) {
            switch (type) {
                case ui::Widget::TouchEventType::BEGAN: break;
                case ui::Widget::TouchEventType::ENDED: {
                    auto scene = cocos2d::Director::getInstance()->getRunningScene();
                    auto world = scene->getPhysicsWorld();
                    if(world) {
                        this->SwitchPhysicsDebugMode();
                    }
                } break;
                default: break;
            }
        });

        m_caption->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        checkbox->setAnchorPoint({0.f, 0.5f});
        
        const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();

        auto shiftY = visibleSize.height / 3.f + origin.y;
        m_caption->setPosition({ -checkbox->getContentSize().width / 2.f, shiftY } );
        auto shiftX = m_caption->getPositionX() + m_caption->getContentSize().width / 2.f + 10.f;
        checkbox->setPosition({shiftX, shiftY});

        this->addChild(m_caption);
        this->addChild(checkbox);
    }
    return true;
};

void DebugScreen::onEnter() {
    cocos2d::Node::onEnter();
    // pause level scene on enter
    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto level = scene->getChildByName("Level");
    level->pause();
};

void DebugScreen::onExit() {
    cocos2d::Node::onExit();
    // resume level scene on exit
    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto level = scene->getChildByName("Level");
    level->resume();
};

/**
 *  static const int DEBUGDRAW_NONE;        ///< draw nothing
 *  static const int DEBUGDRAW_SHAPE;       ///< draw shapes
 *  static const int DEBUGDRAW_JOINT;       ///< draw joints
 *  static const int DEBUGDRAW_CONTACT;     ///< draw contact
 *  static const int DEBUGDRAW_ALL;         ///< draw all
 */
void DebugScreen::SwitchPhysicsDebugMode() noexcept {
    if(m_physicsWorldMask == cocos2d::PhysicsWorld::DEBUGDRAW_ALL) {
        m_physicsWorldMask = cocos2d::PhysicsWorld::DEBUGDRAW_NONE;
    }
    else {
        m_physicsWorldMask = cocos2d::PhysicsWorld::DEBUGDRAW_ALL;
    }
    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto world = scene->getPhysicsWorld();
    if(world) {
        world->setDebugDrawMask(m_physicsWorldMask);
    }
}

} // namespace screen