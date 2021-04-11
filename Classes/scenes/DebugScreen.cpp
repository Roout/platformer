#include "DebugScreen.hpp"

#include "ui/CocosGUI.h"

#include "../units/Player.hpp"
#include "../Core.hpp"

#include <array>

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

    if (!cocos2d::Node::init()) {
        return false;
    }
    this->setName(NAME);

    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto map = scene->getChildByName("Level")->getChildByName("Map");
    auto world = scene->getPhysicsWorld();
    if (!world) return false;

    m_physicsWorldMask = world->getDebugDrawMask();
    m_isInvincible = map->getChildByName<Player*>(core::EntityNames::PLAYER)->IsInvincible();
    // m_displayState

    enum OptionKind { kPhysics, kInvicible, kState };

    std::array<cocos2d::Label*, 3U> captions {
        cocos2d::Label::createWithTTF("Physics debug ", "fonts/arial.ttf", 25)
        , cocos2d::Label::createWithTTF("Be invincible ", "fonts/arial.ttf", 25)
        , cocos2d::Label::createWithTTF("Show state    ", "fonts/arial.ttf", 25)
    };
    std::array<ui::CheckBox*, 3U> checkboxes { nullptr };

    for(auto caption: captions) {
        caption->setTextColor(cocos2d::Color4B::WHITE);
        caption->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    }
    for(auto& checkbox: checkboxes) {
        checkbox = ui::CheckBox::create(
            "cocosui/check_box_normal.png"
            ,"cocosui/check_box_normal_press.png"
            ,"cocosui/check_box_active.png"
            ,"cocosui/check_box_normal_disable.png"
            ,"cocosui/check_box_active_disable.png"
        );
        checkbox->setAnchorPoint({0.f, 0.5f});
    }

    if (m_physicsWorldMask == cocos2d::PhysicsWorld::DEBUGDRAW_ALL) {
        checkboxes[kPhysics]->setSelected(true);
    }
    if (m_isInvincible) {
        checkboxes[kInvicible]->setSelected(true);
    }
    if (m_displayState) {
        checkboxes[kState]->setSelected(true);
    }

    checkboxes[kPhysics]->addTouchEventListener([this](cocos2d::Ref* sender, ui::Widget::TouchEventType type) {
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
    checkboxes[kInvicible]->addTouchEventListener([this](cocos2d::Ref* sender, ui::Widget::TouchEventType type) {
        switch (type) {
            case ui::Widget::TouchEventType::BEGAN: break;
            case ui::Widget::TouchEventType::ENDED: {
                auto scene = cocos2d::Director::getInstance()->getRunningScene();
                // push custom event about invicibility
                /// TODO: port to some manage system or config 
                /// to be able to change the name of event at one place
                cocos2d::EventCustom event("INVINCIBLE");
                m_isInvincible = m_isInvincible? false: true;
                auto level = scene->getChildByName("Level");
                this->getEventDispatcher()->resumeEventListenersForTarget(level, true);
                level->getEventDispatcher()->dispatchEvent(&event);
                this->getEventDispatcher()->pauseEventListenersForTarget(level, true);
            } break;
            default: break;
        }
    });
    checkboxes[kState]->addTouchEventListener([this](cocos2d::Ref* sender, ui::Widget::TouchEventType type) {
        switch (type) {
            case ui::Widget::TouchEventType::BEGAN: break;
            case ui::Widget::TouchEventType::ENDED: {
                m_displayState = m_displayState? false: true;
                // auto scene = cocos2d::Director::getInstance()->getRunningScene();
            } break;
            default: break;
        }
    });
    
    const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    // const auto origin = cocos2d::Director::getInstance()->getOrigin();

    auto background = cocos2d::DrawNode::create();
    background->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    background->setPositionY(visibleSize.height / 5.f);
    const cocos2d::Vec2 leftBot { -visibleSize.width * 0.2f, -visibleSize.height * 0.2f }, 
                        rightTop { visibleSize.width * 0.2f, visibleSize.height * 0.2f };
    const cocos2d::Color4F color{0.f, 0.f, 0.f, 0.5f};
    background->drawSolidRect(leftBot, rightTop, color);
    this->addChild(background);

    auto shiftY { rightTop.y * 0.8f } ;
    captions[kPhysics]->setPosition(
        -checkboxes[kPhysics]->getContentSize().width / 2.f
        , shiftY 
    );
    auto shiftX = captions[kPhysics]->getPositionX() + captions[kPhysics]->getContentSize().width / 2.f + 10.f;
    checkboxes[kPhysics]->setPosition({shiftX, shiftY});

    captions[kInvicible]->setPosition(
        captions[kPhysics]->getPositionX()
        , shiftY - captions[kInvicible]->getContentSize().height
    );
    checkboxes[kInvicible]->setPosition({ checkboxes[kPhysics]->getPositionX(), captions[kInvicible]->getPositionY() });

    captions[kState]->setPosition(
        captions[kInvicible]->getPositionX()
        , captions[kInvicible]->getPositionY() - captions[kState]->getContentSize().height 
    );
    checkboxes[kState]->setPosition({
        checkboxes[kInvicible]->getPositionX()
        , captions[kState]->getPositionY()
    });

    for(auto caption: captions) {
        background->addChild(caption);
    }
    for(auto checkbox: checkboxes) {
        background->addChild(checkbox);
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