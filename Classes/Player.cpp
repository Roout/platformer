#include "Player.hpp"
#include "SmoothFollower.hpp"
#include "Utils.hpp"
#include "Core.hpp"
#include "SizeDeducer.hpp"
#include "Weapon.hpp"

Player* Player::create(const cocos2d::Size& size) {
    auto pRet { new (std::nothrow) Player(size) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player(const cocos2d::Size& sz) :
    Unit { sz, "mc" }
{
    // Create weapon
    const int damage { 25 };
    const int range { SizeDeducer::GetInstance().GetAdjustedSize(60) };
    const float reloadTime { m_maxAttackTime };
    m_weapon = std::make_unique<Sword>( damage, range, reloadTime );
}


bool Player::init() {
    if( !Unit::init()) {
        return false; 
    }
    m_follower = std::make_unique<SmoothFollower>(this);

    const auto body { this->getPhysicsBody() };
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::HERO)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
  //          core::CategoryBits::ENEMY, 
            core::CategoryBits::BOUNDARY, 
  //          core::CategoryBits::PROJECTILE, 
            core::CategoryBits::PLATFORM 
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::PROJECTILE,
            core::CategoryBits::TRAP,
            core::CategoryBits::PLATFORM
        )
    );
    const auto sensor { 
        body->getShape(Utils::EnumCast(
            core::CategoryBits::GROUND_SENSOR)
        ) 
    };
    sensor->setCollisionBitmask(0);
    sensor->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
    );
    sensor->setContactTestBitmask(
        Utils::CreateMask(
    //        core::CategoryBits::ENEMY,
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );
    return true;
}

void Player::setPosition(const cocos2d::Vec2& position) {
    Node::setPosition(position.x, position.y);
    m_follower->Reset();
}

void Player::update(float dt) {
    Unit::update(dt);
    m_follower->UpdateMapPosition(dt);
}
