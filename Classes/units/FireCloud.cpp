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
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width * 0.875f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}

bool FireCloud::init() {
    if (!Bot::init() ) {
        return false; 
    }

    m_health = 1'000'000'000; // some big value to make cloud indestructable
    // set up lifetime of the cloud
    constexpr float CLOUD_LIFETIME { 10.f };
    m_lifetime = CLOUD_LIFETIME;

    return true;
}

void FireCloud::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapons(dt);
    // TODO: remove cuz it's undestructable
    // this->UpdateCurses(dt);
    // TODO: implement own attack!
    // this->TryAttack();
    this->UpdateState(dt);
    if(m_currentState != State::DEAD) {
        this->UpdateAnimation(); 
    }
}

/// Bot interface

void FireCloud::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void FireCloud::OnEnemyLeave() {
    m_detectEnemy = false;
}

void FireCloud::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if(m_currentState == State::UNDEFINED) {
        m_currentState = State::INIT;
    }
    else if(m_currentState == State::LATE && m_lifetime <= 0.f) {
        m_currentState = State::DEAD;
        this->OnDeath();
    }
    else if(m_currentState == State::LATE && m_lifetime >= 0.f) {
        m_lifetime -= dt;
    }
    else if(m_finished) {
        m_currentState = static_cast<State>(Utils::EnumCast(m_currentState) + 1);
        m_finished = false;
    }
}

void FireCloud::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        assert(m_currentState != State::DEAD && "Logic error: DEATH state shouldn't be reached!");

        auto isOneTimeAttack { m_currentState != State::LATE };
        auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
    }
}

void FireCloud::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
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
    
    this->addComponent(body);
}

void FireCloud::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::INIT),    GetStateName(State::INIT)), 
        std::make_pair(Utils::EnumCast(State::EARLY),   GetStateName(State::EARLY)), 
        std::make_pair(Utils::EnumCast(State::MID),     GetStateName(State::MID)), 
        std::make_pair(Utils::EnumCast(State::LATE),    GetStateName(State::LATE))
    });
    m_animator->EndWith([this](){
       this->m_finished = true;
    });
}

void FireCloud::AddWeapons() {
    const auto damage { 15.f };
    const auto range { 50.f };
    const auto preparationTime { 0.f }; 
    const auto attackDuration { 0.5f };
    const auto reloadTime { 0.5f };
    m_weapons[WeaponClass::RANGE] = new CloudFireball(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void FireCloud::Attack() {
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
            body->setVelocity({ this->IsLookingLeft()? -300.f: 300.f, -600.f });
        };
        m_weapons[WeaponClass::RANGE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

bool FireCloud::NeedAttack() const noexcept {
    return !this->IsDead() && m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

} // namespace Enemies