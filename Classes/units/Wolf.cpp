#include "Wolf.hpp"

#include "../Weapon.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Core.hpp"
#include "../Movement.hpp"

#include <memory>

namespace Enemies {

Wolf* Wolf::create(size_t id, const cocos2d::Size& contentSize) {
    auto pRet { new (std::nothrow) Wolf(id, core::EntityNames::WOLF, contentSize) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Wolf::init() {
    if (!Warrior::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(PATROL_SPEED);
    return true;
}

Wolf::Wolf(size_t id, const char * name, const cocos2d::Size& contentSize)
    : Warrior{ id, name, contentSize }
{
    m_physicsBodySize = cocos2d::Size { m_contentSize.width * 0.9f,  m_contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}

void Wolf::AddWeapons() {
    const auto damage { 20.f };
    const auto range { 15.f };
    const auto attackDuration { 0.2f };
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };
    const auto reloadTime { 0.4f };
    m_weapons[WeaponClass::MELEE] = new Maw(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void Wolf::Attack() {
    if (m_weapons[WeaponClass::MELEE]->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]() -> cocos2d::Rect {
            const auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
            const cocos2d::Size mawSize { attackRange, attackRange * 2.f };

            auto position = this->getPosition();
            if (this->IsLookingLeft()) {
                position.x -= m_hitBoxSize.width / 2.f + mawSize.width;
            }
            else {
                position.x += m_hitBoxSize.width / 2.f;
            }
            position.y += m_hitBoxSize.height / 3.f;

            return { position, mawSize };
        };
        auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
            body->setVelocity(this->getPhysicsBody()->getVelocity());
        };
        m_weapons[WeaponClass::MELEE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

void Wolf::OnEnemyIntrusion() {
    Warrior::OnEnemyIntrusion();
    m_movement->SetMaxSpeed(PURSUE_SPEED);
}

void Wolf::OnEnemyLeave() {
    Warrior::OnEnemyLeave();
    m_movement->SetMaxSpeed(PATROL_SPEED);
}

bool Wolf::NeedAttack() const noexcept {
    bool attackIsReady {
        !this->IsDead() && 
        m_detectEnemy && 
        m_weapons[WeaponClass::MELEE]->IsReady()
    };
    bool enemyIsClose = false;
    if (attackIsReady) {
        const auto target = this->getParent()->getChildByName<const Unit*>(core::EntityNames::PLAYER);
        // use some simple algorithm to determine whether a player is close enough to the target
        // to perform an attack
        if(target && !target->IsDead()) {
            // TODO: this is code replication!!!! (see above Wasp::Attack())
            // calc position of the maw:
            const auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
            const cocos2d::Size mawSize { attackRange, attackRange * 1.6f };

            auto position = this->getPosition();
            if (this->IsLookingLeft()) {
                position.x -= m_hitBoxSize.width / 2.f + mawSize.width;
            }
            else {
                position.x += m_hitBoxSize.width / 2.f;
            }
            position.y += m_hitBoxSize.height / 3.f;

            const auto radius = m_weapons[WeaponClass::MELEE]->GetRange() * 0.75f;
            const auto targetHitbox = target->GetHitBox();
            const cocos2d::Rect lhs { 
                target->getPosition() - cocos2d::Vec2{ targetHitbox.width / 2.f, 0.f },
                targetHitbox
            };
            const cocos2d::Rect rhs { position, mawSize };
            enemyIsClose = lhs.intersectsRect(rhs);
        }
    }
    return attackIsReady && enemyIsClose;
}

} // namespace Enemies