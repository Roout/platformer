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
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::BARREL
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::ENEMY_PROJECTILE 
            , core::CategoryBits::HITBOX_SENSOR 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj); 
}

void Axe::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(m_projectile.size, m_velocity, this->GetDamage());
    proj->setPosition(m_projectile.origin);
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::BARREL
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj);
}

void Spear::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(m_projectile.size, m_velocity, this->GetDamage());
    proj->setPosition(m_projectile.origin);
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::BARREL
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj);
}

void Bow::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create("archer/library/arrow.png", m_velocity, this->GetDamage());
    proj->setPosition(m_projectile.origin);
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::BARREL
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::PLATFORM
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    proj->SetLifetime(3.f);
    map->addChild(proj, 100); /// TODO: clean up this mess with Z-order!
}
