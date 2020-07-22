#include "Weapon.hpp"
#include "Projectile.hpp"
#include "SizeDeducer.hpp"
#include "Utils.hpp"

void Sword::Attack(
    const cocos2d::Vec2& position,
    const cocos2d::Vec2& velocity
) noexcept {
    static constexpr float width { 60.f };
    static constexpr float height { 146.f };
    // define size of projectile:
    // Size will be close to character's size.
    /// TODO: change size of the sword attack to something more than random magic values.
    const cocos2d::Size size { 
        SizeDeducer::GetInstance().GetAdjustedSize(width), 
        SizeDeducer::GetInstance().GetAdjustedSize(height) 
    };
    
    // define attack speed
    const auto speed { SizeDeducer::GetInstance().GetAdjustedSize(250.f) };

    auto proj = Projectile::create(size, velocity * speed, this->GetDamage());
    proj->setPosition(position);
    /**
     * These 'runtime queries' is hard to be erased. 
     */
    auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    auto map = runningScene->getChildByName("Map");
    auto player = map->getChildByName("Player");

    player->addChild(proj);

    this->ForceReload();
}