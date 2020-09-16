#ifndef LEVEL_INTERFACE_HPP
#define LEVEL_INTERFACE_HPP

#include "cocos2d.h"

#include "PauseNode.hpp"

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
        const auto listener = cocos2d::EventListenerKeyboard::create();
        listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event) {
            if(code == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE && this->OnPressedButton()) {
                const auto scene = cocos2d::Director::getInstance()->getRunningScene();
                const auto level = scene->getChildByName("Level");
                if(level) {
                    level->pause();
                }
            }
            return true;
        };
        this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    }

    void onExit() override {
        cocos2d::Node::onExit();
        this->getEventDispatcher()->removeAllEventListeners();
    }

private:

    bool OnPressedButton() {
        if(this->getChildrenCount() == 0U) {
            auto node = PauseNode::create();
            const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
            const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
            node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            node->setPosition(visibleSize / 2.f);
            this->addChild(node);
            return true;
        }
        return false;
    }
};

#endif // LEVEL_INTERFACE_HPP