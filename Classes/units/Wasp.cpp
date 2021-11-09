#include "Wasp.hpp"

#include "../Weapon.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Core.hpp"
#include "../Movement.hpp"
#include "../Influence.hpp"

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
    if (!Warrior::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(35.f);
    return true;
}

Wasp::Wasp(size_t id, const char * name, const cocos2d::Size& contentSize)
    : Warrior{ id, name, contentSize }
{
    m_physicsBodySize = cocos2d::Size { m_contentSize.width * 0.5f,  m_contentSize.height * 0.8f};
    m_hitBoxSize = cocos2d::Size { m_contentSize.width * 0.6f, m_contentSize.height * 0.9f };
}

void Wasp::AddWeapons() {
    const auto damage { 10.f };
    const auto range { 15.f };
    const auto attackDuration { 0.2f };
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - attackDuration };
    const auto reloadTime { 0.6f };
    m_weapons[WeaponClass::MELEE].reset(new Maw(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime));
}

void Wasp::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::MELEE]->IsReady());

    auto projectilePosition = [this]() -> cocos2d::Rect {
        const auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
        const cocos2d::Size stingSize { attackRange, attackRange / 4.f };

        auto position = getPosition();
        if (IsLookingLeft()) {
            position.x -= m_hitBoxSize.width / 2.f + stingSize.width;
        }
        else {
            position.x += m_hitBoxSize.width / 2.f;
        }
        position.y += m_hitBoxSize.height / 8.f;

        return { position, stingSize };
    };
    auto pushProjectile = [](cocos2d::PhysicsBody* body) {
        // body->setVelocity(getPhysicsBody()->getVelocity());
    };
    m_weapons[WeaponClass::MELEE]->LaunchAttack(
        std::move(projectilePosition), 
        std::move(pushProjectile)
    );
}

void Wasp::Pursue(Unit * target) noexcept {
    assert(target);
    if (!this->IsDead() && target && !target->IsDead()) {
        m_navigator->VisitCustomPoint(
            target->getPosition() + cocos2d::Vec2{0.f, target->GetHitBox().height / 2.f}
        );
    }
}

bool Wasp::NeedAttack() const noexcept {
    assert(!IsDead());
    bool attackIsReady { m_detectEnemy 
        && m_weapons[WeaponClass::MELEE]->IsReady()
    };
    if (!attackIsReady) return false;
    
    bool enemyIsClose = false;
    const auto target = this->getParent()->getChildByName<const Unit*>(core::EntityNames::PLAYER);
    // use some simple algorithm to determine whether a player is close enough to the target
    // to perform an attack
    if (target && !target->IsDead()) {
        // TODO: this is code replication!!!! (see above Wasp::Attack())
        // calc position of the stinger:
        const auto attackRange { m_weapons[WeaponClass::MELEE]->GetRange() };
        const cocos2d::Size stingSize { attackRange, attackRange / 4.4f };
        auto position = this->getPosition();
        if (this->IsLookingLeft()) {
            position.x -= m_hitBoxSize.width / 2.f + stingSize.width;
        }
        else {
            position.x += m_hitBoxSize.width / 2.f;
        }
        position.y += m_hitBoxSize.height / 8.f;

        const auto radius = m_weapons[WeaponClass::MELEE]->GetRange() * 0.75f;
        const auto targetHitbox = target->GetHitBox();
        const cocos2d::Rect lhs { 
            target->getPosition() - cocos2d::Vec2{ targetHitbox.width / 2.f, 0.f },
            targetHitbox
        };
        const cocos2d::Rect rhs { position, stingSize };
        enemyIsClose = lhs.intersectsRect(rhs);
    }
    return enemyIsClose;
}

void Wasp::AttachNavigator(Path&& path) {
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    m_navigator->FollowPath();
    m_navigator->SetPrecision(15.f);
}

void Wasp::MoveAlong(Movement::Direction dir) noexcept {
    m_movement->Move(dir);
}

void Wasp::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.1f, 0.1f), 
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    body->setDynamic(true);
    body->setGravityEnable(false);
    body->setRotationEnable(false);
    
    body->setContactTestBitmask(0);
    body->setCollisionBitmask(0);
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));

    const auto hitBoxShape = cocos2d::PhysicsShapeBox::create(
        m_hitBoxSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT,
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    const auto hitBoxTag { Utils::EnumCast(core::CategoryBits::HITBOX_SENSOR) };
    hitBoxShape->setSensor(true);
    hitBoxShape->setTag(hitBoxTag);
    hitBoxShape->setCollisionBitmask(0);
    hitBoxShape->setCategoryBitmask(hitBoxTag);
    hitBoxShape->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::HITBOX_SENSOR
        )
    );
    body->addShape(hitBoxShape, false);
    
    this->addComponent(body);
}

void Wasp::OnEnemyIntrusion() {
    Warrior::OnEnemyIntrusion();
    m_movement->SetMaxSpeed(70.f);
}

void Wasp::OnEnemyLeave() {
    Warrior::OnEnemyLeave();
    m_movement->SetMaxSpeed(35.f);
}

} // namespace Enemies