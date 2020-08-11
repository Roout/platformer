#include "Weapon.hpp"
#include "Projectile.hpp"
#include "Utils.hpp"
#include "Player.hpp"
#include "Core.hpp"

void Sword::Attack(
    const cocos2d::Rect& area,
    const cocos2d::Vec2& velocity
) noexcept {
   
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto map = runningScene->getChildByName("Map");

    const auto proj = Projectile::create(area.size, velocity, this->GetDamage());
    proj->setPosition(area.origin);
    /// TODO: this solution isn't correct when sword is equiped by the AI
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

    this->ForceReload();
}

void Axe::Attack(
    const cocos2d::Rect& area,
    const cocos2d::Vec2& velocity
) noexcept {

    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto map = runningScene->getChildByName("Map");
    
    const auto proj = Projectile::create(area.size, velocity, this->GetDamage());
    proj->setPosition(area.origin);
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

    this->ForceReload();
}