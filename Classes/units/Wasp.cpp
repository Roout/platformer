#include "Wasp.hpp"

#include "../Weapon.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Core.hpp"
#include "../Movement.hpp"

#include <memory>

namespace Enemies {

Wasp* Wasp::Wasp::create(size_t id, const cocos2d::Size& contentSize) {
    auto pRet { new (std::nothrow) Wasp(id, core::EntityNames::WASP, contentSize) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Wasp::init() {
    if (!Warrior::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(35.f);
    const auto body = this->getPhysicsBody();
    body->setGravityEnable(false);
    return true;
}

Wasp::Wasp(size_t id, const char * name, const cocos2d::Size& contentSize)
    : Warrior{ id, name, contentSize }
{
}

void Wasp::AddWeapons() {
    const auto damage { 10.f };
    const auto range { 20.f };
    const auto attackDuration { 0.2f };
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };
    const auto reloadTime { 0.6f };
    m_weapons[WeaponClass::MELEE] = new Maw(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void Wasp::Attack() {
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