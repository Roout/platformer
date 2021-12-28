#include "Spearman.hpp"

#include "../Weapon.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Core.hpp"
#include "../Movement.hpp"

#include "../configs/JsonUnits.hpp"

#include <memory>

namespace Enemies {

Spearman* Spearman::Spearman::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Spearman *spearman) 
{
    auto pRet { new (std::nothrow) Spearman(id, contentSize, spearman) };
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
    m_movement->SetMaxSpeed(m_spearman->maxSpeed);
    m_health = m_spearman->health;
    return true;
}

Spearman::Spearman(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Spearman *spearman
)
    : Warrior{ id, core::EntityNames::SPEARMAN, contentSize }
    , m_spearman { spearman }
{
    assert(spearman);
}

void Spearman::AddWeapons() {
    const auto attackDuration { 0.2f };
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };
    
    auto genPos = [this]() -> cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
        cocos2d::Size spearSize { attackRange, attackRange / 4.f };

        auto position = getPosition();
        if (IsLookingLeft()) {
            position.x -= m_contentSize.width / 2.f + spearSize.width;
        }
        else {
            position.x += m_contentSize.width / 2.f;
        }
        position.y += m_contentSize.height / 3.f - spearSize.height / 2.f;

        return { position, spearSize };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity(getPhysicsBody()->getVelocity());
    };

    const auto& spear = m_spearman->weapons.spear;
    auto& weapon = m_weapons[WeaponClass::MELEE];
    weapon.reset(new Spear(spear.damage
        , spear.range
        , preparationTime
        , attackDuration
        , spear.cooldown));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}

void Spearman::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::MELEE]->IsReady());

    m_weapons[WeaponClass::MELEE]->LaunchAttack();
}

}// namespace Enemies