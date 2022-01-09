#include "AxWarrior.hpp"

#include "Weapon.hpp"
#include "Influence.hpp"
#include "Movement.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"

#include "configs/JsonUnits.hpp"

namespace Enemies {

AxWarrior* AxWarrior::AxWarrior::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::AxWarrior *model) 
{
    auto pRet { new (std::nothrow) AxWarrior(id, contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool AxWarrior::init() {
    if (!Warrior::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(m_model->maxSpeed);
    m_health = m_model->health;
    return true;
}

AxWarrior::AxWarrior(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::AxWarrior *model
)
    : Warrior { id, core::EntityNames::WARRIOR, contentSize }
    , m_model { model }
{
    assert(model);
}


void AxWarrior::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::MELEE]);
    assert(m_weapons[WeaponClass::MELEE]->IsReady());

    m_weapons[WeaponClass::MELEE]->LaunchAttack();
}

void AxWarrior::AddWeapons() {
    float attackDuration { 0.15f };
    float preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };

    auto genPos = [this]() -> cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };

        auto position = getPosition();
        if (m_side == Side::RIGHT) {
            position.x += m_contentSize.width / 2.f;
        }
        else {
            position.x -= m_contentSize.width / 2.f + attackRange;
        }
        // shift a little bit higher to avoid immediate collision with the ground
        position.y += m_contentSize.height * 0.05f;
        cocos2d::Rect attackedArea {
            position,
            cocos2d::Size{ attackRange, m_contentSize.height * 1.05f } // a little bigger than the designed size
        };
        return attackedArea;
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity(getPhysicsBody()->getVelocity());
    };

	const auto& axe = m_model->weapons.axe;
    auto& weapon = m_weapons[WeaponClass::MELEE];
    weapon.reset(new Axe(axe.damage, axe.range, preparationTime, attackDuration, axe.cooldown));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}


} // namespace Enemies