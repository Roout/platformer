#include "Weapon.hpp"
#include <type_traits> // std::invoke
#include "cocos2d.h"

void Sword::Attack(
    PhysicWorld * const world,
    const cocos2d::Vec2& position,
    const cocos2d::Vec2& direction
) noexcept {
    // define size of projectile:
    // Size will be around the character size.
    // TODO: change size of the sword attack to something more than random magic values.
    const cocos2d::Size size { 20.f, 20.f };
    
    // define attack speed
    const auto speed { 120.f };

    // define callback
    // Requirements: 
    // 1. Must nullify lifetime of the projectile on collision
    // 2. Must deduct health
    auto callback = [damage = this->GetDamage()](core::Entity* const rhs) {
        rhs->RecieveDamage(damage);
        cocos2d::log("Sword projectile collide with some entity and expect to deal: %d damage.", damage);

    };
    auto proj = std::make_unique<Projectile>(world, position, size , direction, speed, callback);
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
    m_body { position, size, this },
    m_lifeTime { 1.f }
{
    m_world->Add( &m_body, [this, &weaponCallback](core::Entity* entity) {
        // do the job known to sword
        std::invoke(weaponCallback, entity);
        // do the job known to this projectile:
        // end it's lifetime after collision!
        this->Collapse();
    });
    
    m_body.SetDirection( { direction.x, 0.f });
    m_body.SetXAxisSpeed(xAxisSpeed);
    m_body.SetMask(
        CreateMask(CategoryBits::PROJECTILE),
        CreateMask(CategoryBits::ENEMY, CategoryBits::BOUNDARY) 
    );
    cocos2d::log("Projectile was created with body");
}

Projectile::~Projectile() {
    m_world->Erase(&m_body);
    cocos2d::log("Projectile was destroyed");
}