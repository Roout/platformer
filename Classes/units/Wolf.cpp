#include "Wolf.hpp"
#include "Core.hpp"

#include "components/Weapon.hpp"
#include "components/DragonBonesAnimator.hpp"
#include "components/Movement.hpp"

#include "configs/JsonUnits.hpp"

#include <memory>

namespace Enemies {

Wolf* Wolf::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Wolf *model
) {
    auto pRet { new (std::nothrow) Wolf(id, contentSize, model) };
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
    m_movement->SetMaxSpeed(m_model->idleSpeed);
    return true;
}

Wolf::Wolf(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Wolf *model
)
    : Warrior{ id, core::EntityNames::WOLF, contentSize }
    , m_model { model }
{
    assert(model);
    m_physicsBodySize = cocos2d::Size { m_contentSize.width * 0.9f,  m_contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
    m_health = m_model->health;
}

void Wolf::AddWeapons() {
    const auto& maw = m_model->weapons.maw;

    const auto attackDuration { 0.2f };
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };

    auto genPos = [this]() -> cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
        cocos2d::Size mawSize { attackRange, attackRange * 2.f };

        auto position = getPosition();
        if (IsLookingLeft()) {
            position.x -= m_hitBoxSize.width / 2.f + mawSize.width;
        }
        else {
            position.x += m_hitBoxSize.width / 2.f;
        }
        position.y += m_hitBoxSize.height / 3.f;

        return { position, mawSize };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity(getPhysicsBody()->getVelocity());
    };

    auto& weapon = m_weapons[WeaponClass::MELEE];
    weapon.reset(new Maw(maw.damage
        , maw.range
        , preparationTime
        , attackDuration
        , maw.cooldown));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}

void Wolf::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::MELEE]->IsReady());

    m_weapons[WeaponClass::MELEE]->LaunchAttack();
}

void Wolf::OnEnemyIntrusion() {
    Warrior::OnEnemyIntrusion();
    m_movement->SetMaxSpeed(m_model->alertSpeed);
}

void Wolf::OnEnemyLeave() {
    Warrior::OnEnemyLeave();
    m_movement->SetMaxSpeed(m_model->idleSpeed);
}

bool Wolf::NeedAttack() const noexcept {
    assert(!IsDead());

    bool attackIsReady {
        m_detectEnemy && 
        m_weapons[WeaponClass::MELEE]->IsReady()
    };
    bool enemyIsClose = false;
    if (!attackIsReady) {
        const auto target = getParent()->getChildByName<const Unit*>(core::EntityNames::PLAYER);
        // use some simple algorithm to determine whether a player is close enough to the target
        // to perform an attack
        if(target && !target->IsDead()) {
            // calc position of the maw:
            const auto radius = m_weapons[WeaponClass::MELEE]->GetRange() * 0.75f;
            const auto targetHitbox = target->GetHitBox();
            const cocos2d::Rect lhs { 
                target->getPosition() - cocos2d::Vec2{ targetHitbox.width / 2.f, 0.f },
                targetHitbox
            };
            const cocos2d::Rect rhs { // check attack in both directions
                getPosition() - cocos2d::Vec2 { m_contentSize.width / 2.f + radius, -m_hitBoxSize.height / 3.f },
                cocos2d::Size { m_contentSize.width + 2.f * radius, 0.f }
            };
            enemyIsClose = lhs.intersectsRect(rhs);
        }
    }
    return attackIsReady && enemyIsClose;
}

} // namespace Enemies