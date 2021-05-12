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
    if (!Warrior::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(35.f);
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

void Wasp::Pursue(Unit * target) noexcept {
    if (!this->IsDead() && target && !target->IsDead()) {
        // min distance required for the attack
        const auto minAttackDistance { floorf(
            target->GetHitBox().width / 2.f // shift to bottom left\right corner
            + m_weapons[WeaponClass::MELEE]->GetRange() * 0.75f   // shift by weapon length (not 1.0f to be able to reach the target by attack!)
            + m_contentSize.width / 2.f     // shift by size where the weapon's attack will be created
        )};
        // possible destinations (bottom middle of this unit)
        const float xTargets[2] = { 
            target->getPositionX() + minAttackDistance,
            target->getPositionX() - minAttackDistance,
        };

        const auto deltaHeight = target->GetHitBox().height - m_hitBoxSize.height;
        CCASSERT(deltaHeight >= 0.f, "Height mismatch");

        const float yTargets[2] = { 
            // max Y
            target->getPositionY() + floorf(deltaHeight * 0.75f) /* delta hitbox sizes along Y (of both units) */,
            // min Y
            target->getPositionY() - floorf(m_hitBoxSize.height * 0.75f) /* { my hitbox Y size } * 0.75 */,
        };
        // choose the closest target in our influence field!
        const cocos2d::Vec2 targets[4] = {
            cocos2d::Vec2 { xTargets[0], yTargets[0] }
            , { xTargets[0], yTargets[1] }
            , { xTargets[1], yTargets[0] }
            , { xTargets[1], yTargets[1] }
        };
        float distanceSquared[4]; 
        {
            size_t targetIndex = 0;
            for (auto&d: distanceSquared) {
                d = (targets[targetIndex++] - this->getPosition()).lengthSquared();
            }
        }

        // find the closest point from the ones where unit won't fall down
        int choosenIndex { -1 };
        auto distance { distanceSquared[0] };
        for(int i = 0; i < 4; i++) {
            if(m_influence->Contains(targets[i]) && distanceSquared[i] <= distance) {
                choosenIndex = i;
                distance = distanceSquared[i];
            }
        }
        auto destination = this->getPosition();
        if(choosenIndex != -1) { // every target lead to falling down or other shit
            destination = targets[choosenIndex]; // target->getPosition(); 
        }
        // move to target along X-axis;
        // Pass own Y-axis coordinate to not move along Y-axis
        m_navigator->VisitCustomPoint(destination);
    }
}

void Wasp::AttachNavigator(Path&& path) {
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    m_navigator->FollowPath();
    m_navigator->SetPrecision(15.f);
}

void Wasp::MoveAlong(float x, float y) noexcept {
    m_movement->Move(x, y);
}

void Wasp::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.1f, 0.1f), 
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    // body->setMass(25.f);
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