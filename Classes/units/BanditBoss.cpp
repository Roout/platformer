#include "BanditBoss.hpp"

#include "../Player.hpp"
#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Influence.hpp"
#include "../Movement.hpp"

#include "cocos2d.h"

namespace Enemies {

BanditBoss* BanditBoss::create(size_t id, const cocos2d::Size& contentSize ) {
    auto pRet { new (std::nothrow) BanditBoss(id, core::EntityNames::BOSS, contentSize) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

BanditBoss::BanditBoss(size_t id, const char* dragonBonesName, const cocos2d::Size& contentSize)
    : Bot{ id, dragonBonesName }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width * 0.75f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}


bool BanditBoss::init() {
    if (!Bot::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(80.f);
    return true;
}

void BanditBoss::update(float dt) {
    // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapons(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

/// Unique to warrior
void BanditBoss::AttachNavigator(Path&& path) {
   
}


/// Bot interface
void BanditBoss::OnEnemyIntrusion() {
    m_detectEnemy = true;

}

void BanditBoss::OnEnemyLeave() {
    m_detectEnemy = false;
}

void BanditBoss::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if( m_health <= 0 ) {
        m_currentState = State::DEAD;
    } 

}

void BanditBoss::UpdatePosition(const float dt) noexcept {

}

void BanditBoss::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        const auto isOneTimeAttack { 
            m_currentState == State::ATTACK || 
            m_currentState == State::DEAD
        };
        const auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(this->IsDead()) {
            this->OnDeath();
        }
    }
}

void BanditBoss::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
}

void BanditBoss::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    const auto body { this->getPhysicsBody() };
    // body->setMass(25.f);
    body->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLATFORM 
        )
    );

    const auto hitBoxTag { Utils::EnumCast(core::CategoryBits::HITBOX_SENSOR) };
    const auto hitBoxSensor { body->getShape(hitBoxTag) };
    hitBoxSensor->setCollisionBitmask(0);
    hitBoxSensor->setCategoryBitmask(hitBoxTag);
    hitBoxSensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::HITBOX_SENSOR
        )
    );

    const auto groundSensorTag { Utils::EnumCast(core::CategoryBits::GROUND_SENSOR) };
    const auto groundSensor { body->getShape(groundSensorTag) };
    groundSensor->setCollisionBitmask(0);
    groundSensor->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::GROUND_SENSOR));
    groundSensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLATFORM
        )
    );
}

void BanditBoss::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    std::string prefix = m_dragonBonesName + "/" + m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(chachedArmatureName));
    m_animator->setScale(0.2f); // TODO: introduce multi-resolution scaling
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::IDLE),    GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::PURSUIT), GetStateName(State::PURSUIT)),
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEAD),    GetStateName(State::DEAD))
    });
    this->addChild(m_animator);
}

void BanditBoss::AddWeapons() {
  
}

} // namespace Enemies