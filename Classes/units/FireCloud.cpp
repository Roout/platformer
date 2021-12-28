#include "FireCloud.hpp"
#include "Player.hpp"

#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Movement.hpp"

#include "../configs/JsonUnits.hpp"

#include "cocos2d.h"

namespace Enemies {

FireCloud* FireCloud::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::UnitsFirecloud *model) 
{
    auto pRet { new (std::nothrow) FireCloud(id, contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

FireCloud::FireCloud(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::UnitsFirecloud *model
)
    : Bot{ id, core::EntityNames::FIRECLOUD }
    , m_model { model }
{
    assert(model);
    m_contentSize = cocos2d::Size { contentSize.width, contentSize.height * 2.f };
    m_physicsBodySize = m_contentSize;
    m_hitBoxSize = m_physicsBodySize;
}

bool FireCloud::init() {
    if (!Bot::init() ) {
        return false; 
    }

    if (auto healthBar = getChildByName("health"); healthBar) {
        healthBar->removeFromParent();
    }

    m_health = m_model->health; 
    m_lifetime = m_model->lifetime;
    m_shells = m_model->shellRefillCount;

    return true;
}

void FireCloud::update(float dt) {
    cocos2d::Node::update(dt);
    // custom updates
    UpdateDebugLabel();
    if (!IsDead()) {
        UpdateWeapons(dt);
        UpdatePosition(dt);
        // TODO: remove cuz it's undestructable
        // UpdateCurses(dt);
        TryAttack();
    }
    UpdateState(dt);
    UpdateAnimation(); 
}

/// Bot interface

void FireCloud::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void FireCloud::OnEnemyLeave() {
    m_detectEnemy = false;
}

void FireCloud::UpdatePosition(const float dt) noexcept {
    m_movement->Update();
}

void FireCloud::UpdateWeapons(const float dt) noexcept {
    Unit::UpdateWeapons(dt);
    if (m_shells <= 0.f) {
        m_shellRenewTimer -= dt;
        if (m_shellRenewTimer <= 0.f) {
            m_shells = m_model->shellRefillCount;
        }
    }
}

void FireCloud::TryAttack() {
    if (NeedAttack()) { // attack if possible
        Attack();
    } 
}

void FireCloud::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_currentState == State::UNDEFINED) {
        m_currentState = State::INIT;
    }
    else if (m_currentState == State::LATE && m_lifetime <= 0.f) {
        m_currentState = State::DEAD;
    }
    else if (m_currentState == State::LATE && m_lifetime > 0.f) {
        m_lifetime -= dt;
    }
    else if (m_finished && m_currentState != State::DEAD) {
        m_currentState = static_cast<State>(Utils::EnumCast(m_currentState) + 1);
        m_finished = false;
        // stop comming up
        if (m_currentState == State::LATE) {
            Stop(Movement::Axis::Y);
            assert(getPhysicsBody() && "Cloud doesn't have physics body");

            m_movement->SetMaxSpeed(m_model->maxSpeed);
            using Move = Movement::Direction;
            m_movement->Push(IsLookingLeft()? Move::LEFT: Move::RIGHT);
        }
    }
}

void FireCloud::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        auto isOneTimeAttack { m_currentState != State::LATE };
        auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (m_currentState == State::DEAD) {
            OnDeath();
        }
    }
}

void FireCloud::OnDeath() {
    m_animator->EndWith([this]() {
        removeComponent(getPhysicsBody());
        runAction(cocos2d::RemoveSelf::create(true));
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

    addComponent(body);
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
    m_animator->EndWith([this]() {
       m_finished = true;
    });
    addChild(m_animator);
}

void FireCloud::AddWeapons() {
    const auto preparationTime { 0.f }; 
    const auto attackDuration { 0.1f };

    auto genPos = [this]()->cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
        cocos2d::Size fireballSize { attackRange, attackRange * 1.4f };

        auto position = getPosition();
        position.y -= m_contentSize.height * 0.1f;
        position.x = static_cast<float>(
            cocos2d::RandomHelper::random_int(
                static_cast<int>(position.x - m_contentSize.width / 3.f), 
                static_cast<int>(position.x + m_contentSize.width / 3.f)
            )
        );

        return { position, fireballSize };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        const auto& velocity = m_model->weapons.fireball.projectile.velocity;
        body->setVelocity({ IsLookingLeft()? -velocity[0]: velocity[0], -velocity[1] });
    };

    const auto& fireball = m_model->weapons.fireball;
    auto& weapon = m_weapons[WeaponClass::RANGE];
    weapon.reset(new CloudFireball(fireball.projectile.damage
        , fireball.range
        , preparationTime
        , attackDuration
        , fireball.cooldown));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}

void FireCloud::Attack() {
    if (--m_shells <= 0) {
        m_shellRenewTimer = m_model->shellRefillCooldown;
    }
    m_weapons[WeaponClass::RANGE]->LaunchAttack();
}

bool FireCloud::NeedAttack() const noexcept {
    assert(!IsDead());
    return (m_shells 
        && m_weapons[WeaponClass::RANGE]->IsReady() 
        && m_currentState == State::LATE);
}

} // namespace Enemies