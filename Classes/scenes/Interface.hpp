#ifndef LEVEL_INTERFACE_HPP
#define LEVEL_INTERFACE_HPP

#include "cocos2d.h"

#include "PauseNode.hpp"
#include "DeathScreen.hpp"
#include "DebugScreen.hpp"

/**
 * Responsibility:
 * - manage current visible interface
 * - manage active input listeners
 */
class Interface : public cocos2d::Layer {
public:

    static Interface * create() {
        auto pRet = new (std::nothrow) Interface();
        if(pRet && pRet->init()) {
            pRet->autorelease();
        }
        else {
            delete pRet;
            pRet = nullptr;
        }
        return pRet;
    }

    [[nodiscard]] bool init() override {
        if(!cocos2d::Layer::init()) {
            return false;
        }
        this->setName("Interface");
        return true;
    }

    void onEnter() override {
        cocos2d::Node::onEnter();
        
        const auto dispatcher = this->getEventDispatcher();
        // Listen keyboard events
        const auto listener = cocos2d::EventListenerKeyboard::create();
        listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event) {
            /// TODO: Pause level `onEnter`
            if(code == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE) {
                (void) this->LaunchPauseScreen();
            }
            else if(code == cocos2d::EventKeyboard::KeyCode::KEY_TAB) {
                if(this->m_lastDisplayedScreen == Display::DEBUG) {
                    this->removeChildByName(screen::DebugScreen::NAME);
                    this->m_lastDisplayedScreen = Display::NOTHING;
                }
                else {
                    (void) this->LaunchDebugScreen();
                }
            }
            return true;
        };
        dispatcher->addEventListenerWithSceneGraphPriority(listener, this);

        // Listen custom events
        auto callback = [this](cocos2d::EventCustom* event) { 
            this->LaunchDeathScreen();            
        };
        const auto customEventListener = cocos2d::EventListenerCustom::create(DeathScreen::EVENT_NAME, callback);
        dispatcher->addEventListenerWithSceneGraphPriority(customEventListener, this);
    }

    void onExit() override {
        cocos2d::Node::onExit();
        this->getEventDispatcher()->removeAllEventListeners();
    }

private:
    void LaunchDeathScreen() {
        const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();

        const auto node = DeathScreen::create();
        node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        node->setPosition(visibleSize / 2.f);

        this->addChild(node);
        m_lastDisplayedScreen = Display::DEATH;
    }

    bool LaunchDebugScreen() {
        if(this->getChildrenCount() == 0U) {
            const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
            const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();

            const auto node = screen::DebugScreen::create();
            node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            node->setPosition(visibleSize / 2.f);

            this->addChild(node);
            m_lastDisplayedScreen = Display::DEBUG;
            return true;
        }
        return false;
    }

    bool LaunchPauseScreen() {
        if(this->getChildrenCount() == 0U) {
            const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
            const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
            
            auto node = PauseNode::create();
            node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            node->setPosition(visibleSize / 2.f);

            this->addChild(node);
            m_lastDisplayedScreen = Display::PAUSE;
            return true;
        }
        return false;
    }

private:

    enum class Display : uint8_t {
        NOTHING,
        DEATH,  // death screen is added here but removed somewhere else 
        PAUSE,  // pause is added here but removed somewhere else 
        DEBUG   // added & removed here (Interface class)
    };

    // It may not be the current displayed screen because some of them are removed without 
    // notifying interface
    Display m_lastDisplayedScreen { Display::NOTHING };

};

#endif // LEVEL_INTERFACE_HPP