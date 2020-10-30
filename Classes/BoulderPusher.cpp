#include "BoulderPusher.hpp"

#include "Player.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Weapon.hpp"
#include "Movement.hpp"

#include "cocos2d.h"

namespace Enemies {

BoulderPusher* BoulderPusher::create(size_t id) {
    auto pRet { new (std::nothrow) BoulderPusher(id) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

BoulderPusher::BoulderPusher(size_t id):
    Bot{ id, core::EntityNames::BOULDER_PUSHER }
{
    m_contentSize = cocos2d::Size{ 40.f, 68.f };
    m_physicsBodySize = cocos2d::Size{ 30.f, m_contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}

bool BoulderPusher::init() {
    if (!Bot::init()) {
        return false; 
    }
    return true;
}

void BoulderPusher::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapons(dt);
    this->UpdateCurses(dt);
    this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

void BoulderPusher::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void BoulderPusher::OnEnemyLeave() {
    m_detectEnemy = false;
}

void BoulderPusher::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if(m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else if(m_weapons[WeaponClass::RANGE]->IsPreparing()) {
        m_currentState = State::PREPARE_ATTACK;
    }
    else if(m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::ATTACK;
    }
    else {
        m_currentState = State::IDLE;
    }
}

void BoulderPusher::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        auto isOneTimeAttack { 
            m_currentState == State::ATTACK || 
            m_currentState == State::DEAD
        };
        auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(this->IsDead()) {
            this->OnDeath();
        }
    }
}

void BoulderPusher::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
}

void BoulderPusher::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    const auto body { this->getPhysicsBody() };
    // body->setMass(25.f);
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
    body->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLATFORM 
        )
    );

    /// Hit box sensor:
    const auto hitBoxTag { Utils::EnumCast(core::CategoryBits::HITBOX_SENSOR) };
    const auto hitBoxSensor { body->getShape(hitBoxTag)};
    hitBoxSensor->setCollisionBitmask(0);
    hitBoxSensor->setCategoryBitmask(hitBoxTag);
    hitBoxSensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::HITBOX_SENSOR
        )
    );

    /// Ground sensor:
    const auto groundSensorTag { Utils::EnumCast(core::CategoryBits::GROUND_SENSOR) };
    const auto groundSensor { body->getShape(groundSensorTag) };
    groundSensor->setCollisionBitmask(0);
    groundSensor->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::GROUND_SENSOR));
    groundSensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );
}

void BoulderPusher::AddAnimator() {
    Unit::AddAnimator();
    m_animator->setScale(0.15f); // override scale
    m_animator->InitializeAnimations({
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::ATTACK),          GetStateName(State::IDLE)), 
        std::make_pair(Utils::EnumCast(State::IDLE),            GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::DEAD),            GetStateName(State::DEAD))
    });
}

void BoulderPusher::AddWeapons() {
    const auto damage { 5.f };
    const auto range { 18.f };
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    const auto preparationTime { 0.4f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - preparationTime };
    const auto reloadTime { 1.5f };
    m_weapons[WeaponClass::RANGE] = new Legs(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

bool BoulderPusher::NeedAttack() const noexcept {
    return !this->IsDead() && m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

void BoulderPusher::Attack() {
    if(m_weapons[WeaponClass::RANGE]->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]() -> cocos2d::Rect {
            const auto radius { m_weapons[WeaponClass::RANGE]->GetRange() };
            const cocos2d::Size stoneSize { radius * 2.f, radius * 2.f };

            auto position = this->getPosition();
            if(this->IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f + stoneSize.width;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            return { position, stoneSize };
        };
        auto pushProjectile = [isLookingLeft = this->IsLookingLeft()](cocos2d::PhysicsBody* body) {
            cocos2d::Vec2 impulse { body->getMass() * 200.f, 0.f };
            if (isLookingLeft) {
                impulse.x *= -1.f;
            }
            body->applyImpulse(impulse);
            body->setAngularVelocity(impulse.x > 0.f? -10.f: 10.f);
        };
        m_weapons[WeaponClass::RANGE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

}