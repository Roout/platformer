#include "FireCloud.hpp"

#include "../Player.hpp"
#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Movement.hpp"

#include "cocos2d.h"

namespace Enemies {

FireCloud* FireCloud::create(size_t id, const cocos2d::Size& contentSize) {
    auto pRet { new (std::nothrow) FireCloud(id, contentSize) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

FireCloud::FireCloud(size_t id, const cocos2d::Size& contentSize)
    : Bot{ id, core::EntityNames::FIRECLOUD }
{
    m_contentSize = cocos2d::Size { contentSize.width, contentSize.height * 2.f };
    m_physicsBodySize = m_contentSize;
    m_hitBoxSize = m_physicsBodySize;
}

bool FireCloud::init() {
    if (!Bot::init() ) {
        return false; 
    }

    if(auto healthBar = this->getChildByName("health"); healthBar) {
        healthBar->removeFromParent();
    }

    m_health = 100; // some big value to make cloud indestructable
    // set up lifetime of the cloud
    m_lifetime = CLOUD_LIFETIME;

    return true;
}

void FireCloud::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapons(dt);
    this->UpdatePosition(dt);
    // TODO: remove cuz it's undestructable
    // this->UpdateCurses(dt);
    this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

/// Bot interface

void FireCloud::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void FireCloud::OnEnemyLeave() {
    m_detectEnemy = false;
}

void FireCloud::UpdatePosition(const float dt) noexcept {
    m_movement->Update(dt);
}

void FireCloud::UpdateWeapons(const float dt) noexcept {
    Unit::UpdateWeapons(dt);
    if(m_shells <= 0.f) {
        m_shellRenewTimer -= dt;
        if(m_shellRenewTimer <= 0.f) {
            m_shells = MAX_SHELL_COUNT;
        }
    }
}

void FireCloud::TryAttack() {
    if (this->NeedAttack()) { // attack if possible
        this->Attack();
    } 
}

void FireCloud::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if(m_currentState == State::UNDEFINED) {
        m_currentState = State::INIT;
    }
    else if(m_currentState == State::LATE && m_lifetime <= 0.f) {
        m_currentState = State::DEAD;
    }
    else if(m_currentState == State::LATE && m_lifetime > 0.f) {
        m_lifetime -= dt;
    }
    else if(m_finished && m_currentState != State::DEAD) {
        m_currentState = static_cast<State>(Utils::EnumCast(m_currentState) + 1);
        m_finished = false;
        // stop comming up
        if(m_currentState == State::LATE) {
            m_movement->ResetForceY();
            assert(this->getPhysicsBody() && "Cloud doesn't have physics body");
            cocos2d::Vec2 impulse { 1.f, 0.f };
            if (this->IsLookingLeft()) {
                impulse.x *= -1.f;
            }
            m_movement->SetMaxSpeed(CLOUD_SPEED);
            m_movement->Push(impulse.x, impulse.y);
        }
    }
}

void FireCloud::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        auto isOneTimeAttack { m_currentState != State::LATE };
        auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(m_currentState == State::DEAD) {
            this->OnDeath();
        }
    }
}

void FireCloud::OnDeath() {
    m_animator->EndWith([this](){
        this->removeComponent(this->getPhysicsBody());
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
}

void FireCloud::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.1f, 0.1f), 
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    body->setDynamic(true);
    body->setGravityEnable(false);
    body->setRotationEnable(false);
    
    body->setCollisionBitmask(0);
    body->setCategoryBitmask(0);
    body->setContactTestBitmask(0);

    this->addComponent(body);
}

void FireCloud::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    std::string prefix = "boss/boss_cloud";
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(chachedArmatureName));
    m_animator->setScale(0.27f);
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::INIT),    GetStateName(State::INIT)), 
        std::make_pair(Utils::EnumCast(State::EARLY),   GetStateName(State::EARLY)), 
        std::make_pair(Utils::EnumCast(State::MID),     GetStateName(State::MID)), 
        std::make_pair(Utils::EnumCast(State::LATE),    GetStateName(State::LATE)),
        std::make_pair(Utils::EnumCast(State::DEAD),    GetStateName(State::LATE))
    });
    m_animator->EndWith([this](){
       this->m_finished = true;
    });
    this->addChild(m_animator);
}

void FireCloud::AddWeapons() {
    const auto damage { 15.f };
    const auto range { 115.f };
    const auto preparationTime { 0.f }; 
    const auto attackDuration { 0.1f };
    const auto reloadTime { 0.1f };
    m_weapons[WeaponClass::RANGE] = new CloudFireball(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void FireCloud::Attack() {
    m_shells--;
    if(m_shells <= 0) m_shellRenewTimer = FireCloud::MAX_SHELL_DELAY;

    auto projectilePosition = [this]()->cocos2d::Rect {
        const auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
        const cocos2d::Size fireballSize { attackRange, attackRange * 1.4f };

        auto position = this->getPosition();
        position.y -= m_contentSize.height * 0.1f;
        position.x = static_cast<float>(
            cocos2d::RandomHelper::random_int(
                static_cast<int>(position.x - m_contentSize.width / 3.f), 
                static_cast<int>(position.x + m_contentSize.width / 3.f)
            )
        );

        return { position, fireballSize };
    };
    auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity({ this->IsLookingLeft()? -300.f: 300.f, -600.f });
    };
    m_weapons[WeaponClass::RANGE]->LaunchAttack(
        std::move(projectilePosition), 
        std::move(pushProjectile)
    );
}

bool FireCloud::NeedAttack() const noexcept {
    return (!this->IsDead() 
        && m_shells 
        && m_weapons[WeaponClass::RANGE]->IsReady() 
        && m_currentState == State::LATE
    );
}

} // namespace Enemies