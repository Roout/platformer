#include "Weapon.hpp"
#include <type_traits> // std::invoke
#include "cocos2d.h"
#include "ProjectileView.hpp"
#include "SizeDeducer.hpp"
#include "Utils.hpp"

void Sword::Attack(
    const cocos2d::Vec2& position,
    const cocos2d::Vec2& direction
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

    /// TODO: Update base on direction!
    auto projectilePosition { position };
    if( direction.x < 0.f) {
        projectilePosition.x -= size.width;
    }
    
    auto proj = std::make_unique<Projectile>(projectilePosition, size, direction, speed);
    m_projectiles.emplace_back(std::move(proj));

    this->ForceReload();
}

void Sword::Update(const float dt) noexcept {
    // update weapon reload time
    Weapon::Update(dt);
    // update projectiles
    this->UpdateProjectiles(dt);
}



void Sword::UpdateProjectiles(const float dt) noexcept {
    for(auto& projectile: m_projectiles) {
        projectile->Update(dt);
    }

    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(), [](const auto& projectile) {
            return !projectile->IsExist();
        }),
        m_projectiles.end()
    );
}


Projectile::Projectile (
    const cocos2d::Vec2& position,
    const cocos2d::Size& size,
    const cocos2d::Vec2& velocity,
    const float speed
) : 
    m_lifeTime { 0.15f }
{    
    m_body = cocos2d::PhysicsBody::createBox(size);
    m_body->setVelocity(velocity);
    m_body->setVelocityLimit(speed);
    m_body->setGravityEnable(true);
    m_body->setRotationEnable(false);
    m_body->setPositionOffset(position);
    m_body->setCategoryBitmask(
        Utils::CreateMask(
            core::CategoryBits::PROJECTILE
        )
    );
    m_body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::ENEMY, 
            core::CategoryBits::HERO, 
            core::CategoryBits::BARREL, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE 
        )
    );
    cocos2d::log("Projectile was created with body");

    m_view = ProjectileView::create(this);
    auto map = cocos2d::Director::getInstance()->getRunningScene()->getChildByName("Map");
    map->addChild(m_view);
}

Projectile::~Projectile() {
    /// TODO: clean up this shit. It should be destroyed in right order by itself.
    auto map = cocos2d::Director::getInstance()->getRunningScene()->getChildByName("Map");
    map->removeChild(m_view);
    cocos2d::log("Projectile was destroyed");

    m_body->removeFromWorld();
}