#include "BoulderPusher.hpp"

#include "Player.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Weapon.hpp"
#include "UnitMovement.hpp"

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
    m_contentSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
    m_hitBoxSize = m_physicsBodySize;
}

bool BoulderPusher::init() {
    if( !Bot::init() ) {
        return false; 
    }
    return true;
}

void BoulderPusher::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    // this->UpdateWeapon(dt);
    this->UpdateCurses(dt);
    // this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

void BoulderPusher::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void BoulderPusher::OnEnemyLeave() {
    m_detectEnemy = false;
}

void BoulderPusher::Attack() {
    if(m_weapon->IsReady() && !this->IsDead()) {
        // TODO: isn't implemented yet
    }
}

void BoulderPusher::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if( m_health <= 0 ) {
        m_currentState = State::DEAD;
    }
    // else if( m_weapon->IsPreparing() ) {
    //     m_currentState = State::PREPARE_ATTACK;
    // }
    // else if( m_weapon->IsAttacking() ) {
    //     m_currentState = State::ATTACK;
    // }
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
    body->setMass(25.f);
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
    m_animator->setScale(0.3f); // override scale
    m_animator->InitializeAnimations({
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::ATTACK),          GetStateName(State::ATTACK)), 
        std::make_pair(Utils::EnumCast(State::IDLE),            GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::DEAD),            GetStateName(State::DEAD))
    });
}

void BoulderPusher::AddWeapon() {
    // TODO: isn't implemented yet
}

bool BoulderPusher::NeedAttack() const noexcept {
    return !this->IsDead() && m_detectEnemy && m_weapon->IsReady();
}

}