#include "Archer.hpp"

#include "Player.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Weapon.hpp"

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
    Bot{ id, "archer" }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
}


bool Archer::init() {
    if( !Bot::init() ) {
        return false; 
    }
    return true;
}

void Archer::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
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

    if( m_health <= 0 ) {
        m_currentState = State::DEATH;
    }
    else if( m_weapon->IsAttacking() || m_detectEnemy ) {
        m_currentState = State::ATTACK;
    }
    else {
        m_currentState = State::IDLE;
    }
}

void Archer::UpdateAnimation() {
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

void Archer::AddPhysicsBody() {
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

void Archer::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::IDLE),    GetStateName(State::IDLE)), /// TODO: change
        std::make_pair(Utils::EnumCast(State::DEATH),   GetStateName(State::DEATH))
    });
}

void Archer::AddWeapon() {
    const auto damage { 5.f };
    const auto range { 40.f };
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto reloadTime { 0.5f };
    m_weapon = std::make_unique<Bow>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void Archer::Attack() {
    if(m_weapon->IsReady() && !this->IsDead()) {
        const auto attackRange { m_weapon->GetRange() };
        const cocos2d::Size arrowSize { attackRange, attackRange / 4.f };

        cocos2d::Vec2 velocity { 600.f, 0.f };
        auto position = this->getPosition();
        if(m_side == Side::RIGHT) {
            position.x += this->getContentSize().width / 2.f;
        }
        else {
            position.x -= this->getContentSize().width / 2.f + arrowSize.width;
            velocity.x *= -1.f;
        }
        position.y += this->getContentSize().height / 2.f;

        const cocos2d::Rect attackedArea {position, arrowSize };
        m_weapon->LaunchAttack(attackedArea, velocity);
    }
}

bool Archer::NeedAttack() const noexcept {
    return !this->IsDead() && m_detectEnemy && m_weapon->IsReady();
}

} // namespace Enemies