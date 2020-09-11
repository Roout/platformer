#include "Weapon.hpp"
#include "Projectile.hpp"
#include "Utils.hpp"
#include "Player.hpp"
#include "Core.hpp"

/**
 * TODO: 
 * - remove run-time dependency on UI. It shouldn't be attached to the scene here,
 * this is model!
 * - remove code repetition: everything except mask is the same!
 */

void Sword::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");

    const auto proj = Projectile::create(m_projectile.size, m_velocity, this->GetDamage());
    proj->setPosition(m_projectile.origin);
    const auto mask {
        Utils::CreateMask(
            core::CategoryBits::ENEMY, 
            core::CategoryBits::BARREL, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE 
        )
    };
    proj->SetContactTestBitmask(mask);
    map->addChild(proj); 
}

void Axe::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(m_projectile.size, m_velocity, this->GetDamage());
    proj->setPosition(m_projectile.origin);
    const auto mask {
        Utils::CreateMask(
            core::CategoryBits::HERO, 
            core::CategoryBits::BARREL, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE 
        )
    };
    proj->SetContactTestBitmask(mask);
    map->addChild(proj);
}


void Bow::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(m_projectile.size, m_velocity, this->GetDamage());
    proj->setPosition(m_projectile.origin);
    const auto mask {
        Utils::CreateMask(
            core::CategoryBits::HERO, 
            core::CategoryBits::BARREL, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE,
            core::CategoryBits::PLATFORM
        )
    };
    proj->SetContactTestBitmask(mask);
    proj->SetLifetime(3.f);
    map->addChild(proj);
}
