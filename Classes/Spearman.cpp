#include "Spearman.hpp"

#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"
#include "UnitMovement.hpp"

#include <memory>

namespace Enemies {

Spearman* Spearman::Spearman::create(size_t id) {
    auto pRet { new (std::nothrow) Spearman(id, core::EntityNames::SPEARMAN) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Spearman::init() {
    if (!Warrior::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(150.f);
    return true;
}

Spearman::Spearman(size_t id, const char * name) :
    Warrior{ id, name }
{
    m_contentSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
    m_hitBoxSize = m_physicsBodySize;
}

void Spearman::AddWeapons() {
    const auto damage { 10.f };
    const auto range { 100.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto reloadTime { 0.8f };
    m_weapons[WeaponClass::MELEE] = new Spear(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void Spearman::Attack() {
    if(m_weapons[WeaponClass::MELEE]->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]() -> cocos2d::Rect {
            const auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
            const cocos2d::Size spearSize { attackRange, attackRange / 4.f };

            auto position = this->getPosition();
            if (this->IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f + spearSize.width;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += m_contentSize.height / 2.f - spearSize.height / 2.f;

            return { position, spearSize };
        };
        auto pushProjectile = [this](cocos2d::PhysicsBody* body){
            body->setVelocity(this->getPhysicsBody()->getVelocity());
        };
        m_weapons[WeaponClass::MELEE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

}// namespace Enemies