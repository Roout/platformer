#include "Spearman.hpp"

#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"
#include "Movement.hpp"

#include <memory>

namespace Enemies {

Spearman* Spearman::Spearman::create(size_t id, const cocos2d::Size& contentSize) {
    auto pRet { new (std::nothrow) Spearman(id, core::EntityNames::SPEARMAN, contentSize) };
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
    m_movement->SetMaxSpeed(75.f);
    return true;
}

Spearman::Spearman(size_t id, const char * name, const cocos2d::Size& contentSize)
    : Warrior{ id, name, contentSize }
{
}

void Spearman::AddWeapons() {
    const auto damage { 10.f };
    const auto range { 50.f };
    const auto attackDuration { 0.2f };
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };
    const auto reloadTime { 1.4f };
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
            position.y += m_contentSize.height / 3.f - spearSize.height / 2.f;

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