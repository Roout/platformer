#include "DebugScreen.hpp"
#include "ui/CocosGUI.h"

#include "units/Player.hpp"
#include "Core.hpp"
#include "Settings.hpp"

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
    using Debug = settings::DebugMode;

    if (!cocos2d::Node::init()) {
        return false;
    }
    this->setName(NAME);

    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto map = scene->getChildByName("Level")->getChildByName("Map");
    auto world = scene->getPhysicsWorld();
    if (!world) return false;

    using OptionKind = Debug::OptionKind;

    std::array<cocos2d::Label*, Utils::EnumSize<OptionKind>()> captions {
        cocos2d::Label::createWithTTF("Physics debug ", "fonts/arial.ttf", 25)
        , cocos2d::Label::createWithTTF("Be invincible ", "fonts/arial.ttf", 25)
        , cocos2d::Label::createWithTTF("Show state    ", "fonts/arial.ttf", 25)
    };
    std::array<ui::CheckBox*, Utils::EnumSize<OptionKind>()> checkboxes { nullptr };

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

    for (size_t i = 0; i < Utils::EnumSize<OptionKind>(); i++) {
        const auto kind = Utils::EnumCast<Debug::OptionKind>(i);
        const auto isEnabled = Debug::GetInstance().IsEnabled(kind);
        checkboxes[i]->setSelected(isEnabled);
    }

    for (size_t i = 0; i < Utils::EnumSize<OptionKind>(); i++) {
        const auto kind = Utils::EnumCast<Debug::OptionKind>(i);
        auto listener = [this, kind](cocos2d::Ref* sender, ui::Widget::TouchEventType type) {
            switch (type) {
                case ui::Widget::TouchEventType::BEGAN: break;
                case ui::Widget::TouchEventType::ENDED: {
                    Debug::GetInstance().Toggle(kind);
                    if (kind == Debug::OptionKind::kPhysics) {
                        auto scene = cocos2d::Director::getInstance()->getRunningScene();
                        if (auto world = scene->getPhysicsWorld(); world != nullptr) {
                            this->SwitchPhysicsDebugMode();
                        }
                    }
                } break;
                default: break;
            }
        };
        checkboxes[i]->addTouchEventListener(listener);
    }
    
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
    captions[Utils::EnumCast(OptionKind::kPhysics)]->setPosition(
        -checkboxes[Utils::EnumCast(OptionKind::kPhysics)]->getContentSize().width / 2.f
        , shiftY 
    );
    auto shiftX = captions[Utils::EnumCast(OptionKind::kPhysics)]->getPositionX() 
                + captions[Utils::EnumCast(OptionKind::kPhysics)]->getContentSize().width / 2.f + 10.f;
    checkboxes[Utils::EnumCast(OptionKind::kPhysics)]->setPosition({shiftX, shiftY});

    captions[Utils::EnumCast(OptionKind::kInvicible)]->setPosition(
        captions[Utils::EnumCast(OptionKind::kPhysics)]->getPositionX()
        , shiftY - captions[Utils::EnumCast(OptionKind::kInvicible)]->getContentSize().height
    );
    checkboxes[Utils::EnumCast(OptionKind::kInvicible)]->setPosition({ 
        checkboxes[Utils::EnumCast(OptionKind::kPhysics)]->getPositionX()
        , captions[Utils::EnumCast(OptionKind::kInvicible)]->getPositionY() 
    });

    captions[Utils::EnumCast(OptionKind::kState)]->setPosition(
        captions[Utils::EnumCast(OptionKind::kInvicible)]->getPositionX()
        , captions[Utils::EnumCast(OptionKind::kInvicible)]->getPositionY() 
        - captions[Utils::EnumCast(OptionKind::kState)]->getContentSize().height 
    );
    checkboxes[Utils::EnumCast(OptionKind::kState)]->setPosition({
        checkboxes[Utils::EnumCast(OptionKind::kInvicible)]->getPositionX()
        , captions[Utils::EnumCast(OptionKind::kState)]->getPositionY()
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
    using Debug = settings::DebugMode;

    auto mask = cocos2d::PhysicsWorld::DEBUGDRAW_NONE;
    if (Debug::GetInstance().IsEnabled(Debug::OptionKind::kPhysics)) {
        mask = cocos2d::PhysicsWorld::DEBUGDRAW_ALL;
    }
    auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto world = scene->getPhysicsWorld();
    if (world) {
        world->setDebugDrawMask(mask);
    }
}

} // namespace screen