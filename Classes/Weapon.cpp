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

    auto proj = std::make_unique<Projectile>(position, size, direction, speed);
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
    m_body->setVelocity(velocity * speed);
    m_body->setDynamic(false);
    m_body->setVelocityLimit(speed);
    m_body->setGravityEnable(true);
    m_body->setRotationEnable(false);
    m_body->setCategoryBitmask(
        Utils::CreateMask(
            core::CategoryBits::PROJECTILE
        )
    );
    const auto interactWith { 
        Utils::CreateMask(
            core::CategoryBits::ENEMY, 
            /// TODO: add this when unit will be inherit as HERO or ENEMY and define categoty it collide with
            // core::CategoryBits::HERO,  
            core::CategoryBits::BARREL, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE 
        )
    };
    m_body->setCollisionBitmask(interactWith);
    m_body->setContactTestBitmask(interactWith);

    cocos2d::log("Projectile was created");

    m_view = ProjectileView::create(this);
    /**
     * Anchor point is choosen same as unit it's attached to.
     */
    m_view->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    m_view->setPosition(position);
    m_view->setContentSize(size);
    m_view->addComponent(m_body);

    /**
     * These 'runtime queries' is hard to be erased. 
     */
    auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    auto map = runningScene->getChildByName("Map");
    auto player = map->getChildByName("Player");

    player->addChild(m_view);
}

Projectile::~Projectile() {
    m_view->removeFromParentAndCleanup(true);
    cocos2d::log("Projectile was destroyed");
}