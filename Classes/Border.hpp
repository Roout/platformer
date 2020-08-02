#ifndef BORDER_HPP
#define BORDER_HPP

#include "Core.hpp"
#include "cocos2d.h"
#include "Utils.hpp"

class Border final : public cocos2d::Node {
public:
    static Border* create(const cocos2d::Size & size) {
        auto pRet = new (std::nothrow) Border(size);
        if (pRet && pRet->init()) {
            pRet->autorelease();
        }
        else {
            delete pRet;
            pRet = nullptr;
        }
        return pRet;
    }

private:

    Border(const cocos2d::Size& size) {   
        cocos2d::PhysicsBody * const body = cocos2d::PhysicsBody::createBox(size);
        body->setDynamic(false);
        body->setCategoryBitmask(
            Utils::CreateMask( core::CategoryBits::BOUNDARY )
        );
        body->setCollisionBitmask(
            Utils::CreateMask(
                core::CategoryBits::ENEMY, 
                core::CategoryBits::HERO, 
                core::CategoryBits::PROJECTILE, 
                core::CategoryBits::PLATFORM
            )
        );
        body->setContactTestBitmask(
            Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
        );

        this->addComponent(body);
        this->setContentSize(size);
    }
};

#endif // BORDER_HPP