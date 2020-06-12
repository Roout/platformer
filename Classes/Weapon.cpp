#include "Weapon.hpp"
#include <type_traits> // std::invoke
#include "cocos2d.h"
#include "ProjectileView.hpp"
#include "SizeDeducer.hpp"

void Sword::Attack(
    PhysicWorld * const world,
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
    // define callback
    // Requirements: 
    // 1. Must nullify lifetime of the projectile on collision
    // 2. Must deduct health
    auto callback = [damage = this->GetDamage()](core::Entity* const rhs) {
        rhs->RecieveDamage(damage);
        cocos2d::log("Sword projectile collide with some entity and expect to deal: %d damage.", damage);

    };
    auto proj = std::make_unique<Projectile>(world, projectilePosition, size, direction, speed, callback);
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
    PhysicWorld * const world,
    const cocos2d::Vec2& position,
    const cocos2d::Size& size,
    const cocos2d::Vec2& direction,
    const float xAxisSpeed,
    PhysicWorld::OnCollision weaponCallback
) :
    m_world { world }, 
    m_lifeTime { 0.15f }
{    
    const auto callback = [this, weaponCallback](core::Entity* entity) {
        // do the job known to sword
        std::invoke(weaponCallback, entity);
        // do the job known to this projectile:
        // end it's lifetime after collision!
        this->Collapse();
    };
    m_body = m_world->Create<KinematicBody>(callback, position, size);
    m_body->EmplaceFixture(this, core::CategoryName::UNDEFINED);
    m_body->SetDirection(direction);
    // to allow the body going up, it must have some jump time:
    if( direction.y > 0.f) m_body->Jump();
    m_body->SetXAxisSpeed(xAxisSpeed);
    m_body->SetMask(
        CreateMask(CategoryBits::PROJECTILE),
        CreateMask(CategoryBits::ENEMY, CategoryBits::BOUNDARY, CategoryBits::PLATFORM) 
    );
    cocos2d::log("Projectile was created with body");

    m_view = ProjectileView::create(this);
    auto map = cocos2d::Director::getInstance()->getRunningScene()->getChildByName("Map");
    map->addChild(m_view);
}

Projectile::~Projectile() {
    auto map = cocos2d::Director::getInstance()->getRunningScene()->getChildByName("Map");
    map->removeChild(m_view);
    m_world->Erase(m_body);

    cocos2d::log("Projectile was destroyed");
}