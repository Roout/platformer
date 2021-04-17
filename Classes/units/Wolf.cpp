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
    m_movement->SetMaxSpeed(200.f);
    return true;
}

Wolf::Wolf(size_t id, const char * name, const cocos2d::Size& contentSize)
    : Warrior{ id, name, contentSize }
{
    m_physicsBodySize = m_contentSize;
    m_hitBoxSize = m_contentSize;
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
                position.x -= m_contentSize.width / 2.f + mawSize.width;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += m_contentSize.height / 3.f;

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

} // namespace Enemies