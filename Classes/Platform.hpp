#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "Core.hpp"
#include "Utils.hpp"
#include "cocos2d.h"

class Platform final : public cocos2d::Node {
public:
    static Platform* create(const cocos2d::Size & size) {
        auto pRet = new (std::nothrow) Platform(size);
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

    Platform( const cocos2d::Size& size ) {
        const cocos2d::PhysicsMaterial material { 1.f, 0.f, 0.f };
        const auto body = cocos2d::PhysicsBody::createBox({floorf(size.width * 0.93f), size.height}, material);
        body->setDynamic(false);
        body->setCategoryBitmask(
            Utils::CreateMask(
                core::CategoryBits::PLATFORM
            )
        );
        body->setCollisionBitmask(
            Utils::CreateMask(
                core::CategoryBits::HERO,
                core::CategoryBits::ENEMY
            )
        );
        body->setContactTestBitmask(
            Utils::CreateMask(
                core::CategoryBits::GROUND_SENSOR,
                core::CategoryBits::HERO,
                core::CategoryBits::ENEMY,
                core::CategoryBits::PROJECTILE
            )
        );
        this->addComponent(body);
        this->setContentSize(size);
    }
};

#endif // PLATFORM_HPP