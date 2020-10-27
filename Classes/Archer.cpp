#include "Archer.hpp"

#include "Player.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Weapon.hpp"
#include "Movement.hpp"

#include "cocos2d.h"

namespace Enemies {

Archer* Archer::create(size_t id) {
    auto pRet { new (std::nothrow) Archer(id) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Archer::Archer(size_t id) :
    Bot{ id, core::EntityNames::ARCHER }
{
    m_contentSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
    m_hitBoxSize = m_physicsBodySize;
}


bool Archer::init() {
    if (!Bot::init() ) {
        return false; 
    }
    return true;
}

void Archer::update(float dt) {
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

/// Bot interface
void Archer::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void Archer::OnEnemyLeave() {
    m_detectEnemy = false;
}


void Archer::UpdateState(const float dt) noexcept {
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

void Archer::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        auto isOneTimeAttack { 
            m_currentState == State::PREPARE_ATTACK || 
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


void Archer::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Archer::AddPhysicsBody() {
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

void Archer::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK),  GetStateName(State::ATTACK)),
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::ATTACK),          GetStateName(State::IDLE)), 
        std::make_pair(Utils::EnumCast(State::IDLE),            GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::DEAD),            GetStateName(State::DEAD))
    });
}

void Archer::AddWeapons() {
    const auto damage { 5.f };
    const auto range { 100.f };
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) }; /// TODO: update animation!
    const auto attackDuration { 0.1f };
    const auto reloadTime { 0.5f };
    m_weapons[WeaponClass::RANGE] = new Bow(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void Archer::Attack() {
    if(m_weapons[WeaponClass::RANGE]->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]()->cocos2d::Rect {
            const auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
            const cocos2d::Size arrowSize { attackRange, floorf(attackRange / 8.5f) };

            auto position = this->getPosition();
            if (this->IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f + arrowSize.width;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += m_contentSize.height / 2.f;

            return {position, arrowSize};
        };
        auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
            cocos2d::Vec2 velocity { 600.f, 0.f };
            if (this->IsLookingLeft()) {
                velocity.x *= -1.f;
            }
            body->setVelocity(velocity);
        };
        m_weapons[WeaponClass::RANGE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

bool Archer::NeedAttack() const noexcept {
    return !this->IsDead() && m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

} // namespace Enemies