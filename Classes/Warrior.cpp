#include "Warrior.hpp"

#include "Player.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Weapon.hpp"

#include "cocos2d.h"

namespace Enemies {

Warrior* Warrior::create(size_t id) {
    auto pRet { new (std::nothrow) Warrior(id) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Warrior::Warrior(size_t id) :
    Bot{ id, "warrior" }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
}


bool Warrior::init() {
    if( !Bot::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(130.f);
    return true;
}

void Warrior::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

/// Unique to warrior
void Warrior::AttachNavigator(path::Path&& path) {
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    this->Patrol();
}

void Warrior::Pursue(Unit * target) noexcept {
    m_navigator->Pursue(target);
}

void Warrior::Patrol() noexcept {
    m_navigator->Patrol();
}

/// Bot interface
void Warrior::OnEnemyIntrusion() {
    m_detectEnemy = true;
    auto target = dynamic_cast<Unit*>(this->getParent()->getChildByName(Player::NAME));
    this->Pursue(target);
}

void Warrior::OnEnemyLeave() {
    m_detectEnemy = false;
    this->Patrol();
}


void Warrior::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if( m_health <= 0 ) {
        m_currentState = State::DEATH;
    }
    else if( m_weapon->IsAttacking() ) {
        m_currentState = State::ATTACK;
    }
    else if( m_detectEnemy ) {
        m_currentState = State::PURSUIT;
    }
    else {
        m_currentState = State::PATROL;
    }
}

void Warrior::UpdatePosition(const float dt) noexcept {
    if(m_currentState != State::ATTACK ) {
        m_navigator->Navigate(dt);  // update direction/target if needed
        m_movement->Update(dt);     // apply forces
    }
}

void Warrior::UpdateAnimation() {
    if( this->IsDead() ) {
        // emit particles
        const auto emitter = cocos2d::ParticleSystemQuad::create("particle_texture.plist");
        emitter->setAutoRemoveOnFinish(true);
        /// TODO: adjust for the multiresolution
        emitter->setScale(0.4f);
        emitter->setPositionType(cocos2d::ParticleSystem::PositionType::RELATIVE);
        emitter->setPosition(this->getPosition());
        this->getParent()->addChild(emitter, 9);
        // remove physics body
        this->removeComponent(this->getPhysicsBody());
        // remove from screen
        this->runAction(cocos2d::RemoveSelf::create());
    } 
    else if(m_currentState != m_previousState) {
        auto repeatTimes { m_currentState == State::ATTACK? 1 : dragonBones::Animator::INFINITY_LOOP };
        m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
    }
}

void Warrior::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    auto body { this->getPhysicsBody() };
    body->setMass(2000.f);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::ENEMY)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PLATFORM 
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP,
            core::CategoryBits::PLATFORM,
            core::CategoryBits::PROJECTILE
        )
    );
    const auto sensor { 
        body->getShape(Utils::EnumCast(
            core::CategoryBits::GROUND_SENSOR)
        ) 
    };
    sensor->setCollisionBitmask(0);
    sensor->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
    );
    sensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );
}

void Warrior::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::PURSUIT), GetStateName(State::PURSUIT)),
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEATH),   GetStateName(State::DEATH))
    });
}

void Warrior::AddWeapon() {
    const auto damage { 25.f };
    const auto range { 60.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto reloadTime { 0.5f };
    m_weapon = std::make_unique<Axe>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

} // namespace Enemies