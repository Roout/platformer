#include "Cannon.hpp"
#include "Player.hpp"

#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Movement.hpp"

#include "cocos2d.h"

namespace Enemies {

Cannon* Cannon::create(size_t id, const cocos2d::Size& contentSize, float scale) {
    auto pRet { new (std::nothrow) Cannon(id, contentSize, scale) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Cannon::Cannon(size_t id, const cocos2d::Size& contentSize, float scale)
    : Bot{ id, core::EntityNames::CANNON }
    , m_scale { scale }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width * 0.875f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}

bool Cannon::init() {
    if (!Bot::init() ) {
        return false; 
    }  
    return true;
}

void Cannon::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    UpdateDebugLabel();
    if (!IsDead()) {
        UpdateWeapons(dt);
        TryAttack();
        UpdateCurses(dt);
    }
    UpdateState(dt);
    UpdateAnimation(); 
}

void Cannon::onEnter() {
    cocos2d::Node::onEnter();
    
    if(this->IsLookingLeft()) {
        m_animator->setPositionX(-m_contentSize.width / 2.f);
    }
    else {
        m_animator->setPositionX( m_contentSize.width / 2.f);
    }
}

/// Bot interface
void Cannon::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void Cannon::OnEnemyLeave() {
    m_detectEnemy = false;
}

void Cannon::UpdateState(const float dt) noexcept {
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

void Cannon::UpdateAnimation() {
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

void Cannon::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Cannon::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.0f, 0.0f), 
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    body->setDynamic(false);
    body->setGravityEnable(false);

    const auto hitBoxShape = cocos2d::PhysicsShapeBox::create(
        m_hitBoxSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT,
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    hitBoxShape->setSensor(true);
    hitBoxShape->setCollisionBitmask(0);
    hitBoxShape->setCategoryBitmask(Utils::EnumCast(core::CategoryBits::HITBOX_SENSOR));
    hitBoxShape->setContactTestBitmask(
        Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE, core::CategoryBits::HITBOX_SENSOR)
    );

    // change masks for physics body
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
    body->setContactTestBitmask(0);
    body->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY));
    body->addShape(hitBoxShape, false);
    
    this->addComponent(body);
}

void Cannon::AddAnimator() {
    Unit::AddAnimator();
    m_animator->setScale(m_scale); 
    m_animator->InitializeAnimations({
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK),  GetStateName(State::ATTACK)),
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::ATTACK),          GetStateName(State::IDLE)), 
        std::make_pair(Utils::EnumCast(State::IDLE),            GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::DEAD),            GetStateName(State::DEAD))
    });
}

void Cannon::AddWeapons() {
    const auto damage { 10.f };
    const auto range { 50.f };
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) }; /// TODO: update animation!
    const auto attackDuration { 0.1f };
    const auto reloadTime { 0.3f };
    
    auto genPos = [this]()->cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
        cocos2d::Size stake { attackRange, floorf(attackRange / 8.5f) };

        auto position = this->getPosition();
        if (IsLookingLeft()) {
            position.x -= m_contentSize.width / 2.f + stake.width;
        }
        else {
            position.x += m_contentSize.width / 2.f;
        }
        position.y += m_contentSize.height / 2.f - stake.height / 2.f;

        return { position, stake };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity({ IsLookingLeft()? -450.f: 450.f, 0.f });
    };

    m_weapons[WeaponClass::RANGE].reset(new Stake(
        damage, range, preparationTime, attackDuration, reloadTime));
    m_weapons[WeaponClass::RANGE]->AddPositionGenerator(std::move(genPos));
    m_weapons[WeaponClass::RANGE]->AddVelocityGenerator(std::move(genVel));
}

void Cannon::TryAttack() {
    assert(!IsDead());

    const auto target = getParent()->getChildByName(core::EntityNames::PLAYER);
    if (target && NeedAttack()) { // attack if possible
        Stop(Movement::Axis::XY);
        Attack();
    } 
}

void Cannon::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::RANGE]->IsReady());

    m_weapons[WeaponClass::RANGE]->LaunchAttack();
}

bool Cannon::NeedAttack() const noexcept {
    return m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

} // namespace Enemies