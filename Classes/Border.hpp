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
        const cocos2d::PhysicsMaterial material { 1.f, 0.f, 0.1f };
        const auto body = cocos2d::PhysicsBody::createBox(size, material);
        body->setDynamic(false);
        body->setCategoryBitmask(
            Utils::CreateMask(core::CategoryBits::BOUNDARY)
        );
        body->setCollisionBitmask(
            Utils::CreateMask(
                core::CategoryBits::ENEMY, 
                core::CategoryBits::HERO
            )
        );
        body->setContactTestBitmask(
            Utils::CreateMask(
                core::CategoryBits::GROUND_SENSOR
                , core::CategoryBits::ENEMY_PROJECTILE
                , core::CategoryBits::PLAYER_PROJECTILE
            )
        );

        this->addComponent(body);
        this->setContentSize(size);
    }
};

#endif // BORDER_HPP