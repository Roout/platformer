#include "BanditBoss.hpp"
#include "Player.hpp"

#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Influence.hpp"
#include "../Movement.hpp"
#include "../PhysicsHelper.hpp"

#include "../scenes/LevelScene.hpp"

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
    m_health = MAX_HEALTH;
}


bool BanditBoss::init() {
    if (!Bot::init()) {
        return false; 
    }
    m_movement->SetJumpHeight(80.f, LevelScene::GRAVITY);
    m_movement->SetMaxSpeed(280.f);
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
void BanditBoss::AttachNavigator(Path&&) {}

/// Bot interface
void BanditBoss::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void BanditBoss::OnEnemyLeave() {
    m_detectEnemy = false;
}

void BanditBoss::LaunchFireballs() {
    assert(m_weapons[FIRECLOUD_ATTACK]->IsReady() 
        && !this->IsDead() 
        && "You don't check confition beforehand"
    );
    auto projectilePosition = [this]() -> cocos2d::Rect {
        const auto attackRange { m_weapons[FIREBALL_ATTACK]->GetRange() * 0.25f };

        auto position = this->getPosition();
        if(m_side == Side::RIGHT) {
            position.x += m_contentSize.width / 2.f;
        }
        else {
            position.x -= m_contentSize.width / 2.f + attackRange;
        }
        // shift a little bit higher to avoid immediate collision with the ground
        position.y += m_contentSize.height * 0.2f;
        cocos2d::Rect attackedArea {
            position,
            cocos2d::Size{ m_weapons[FIREBALL_ATTACK]->GetRange() * 2.f, m_contentSize.height * 1.05f } // a little bigger than the designed size
        };
        return attackedArea;
    };
    auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity({ this->IsLookingLeft()? -400.f: 400.f, 0.f });
    };
    m_weapons[FIREBALL_ATTACK]->LaunchAttack(
        std::move(projectilePosition), 
        std::move(pushProjectile)
    );
}

void BanditBoss::LaunchFirecloud() {
    assert(m_weapons[FIRECLOUD_ATTACK]->IsReady() 
        && !this->IsDead() 
        && "You don't check confition beforehand"
    );

    auto projectilePosition = [this]() -> cocos2d::Rect {
        const auto attackRange { m_weapons[FIRECLOUD_ATTACK]->GetRange()};
        auto position = this->getPosition();
        position.y += m_contentSize.height;
        return { position, cocos2d::Size{ attackRange, m_contentSize.height * 0.35f } };
    };
    auto pushProjectile = [](cocos2d::PhysicsBody* body) {
        cocos2d::Vec2 impulse { 0.f, body->getMass() * 190.f };
        body->applyImpulse(impulse);
    };
    // start attack with weapon
    m_weapons[FIRECLOUD_ATTACK]->LaunchAttack(
        std::move(projectilePosition), 
        std::move(pushProjectile)
    );
}

void BanditBoss::LaunchSweepAttack() {
    auto projectilePosition = [this]() -> cocos2d::Rect {
        const auto attackRange { m_weapons[SWEEP_ATTACK]->GetRange()};
        const cocos2d::Size area { attackRange, attackRange };

        auto position = this->getPosition();
        if (this->IsLookingLeft()) {
            position.x -= m_contentSize.width / 2.f + area.width;
        }
        else {
            position.x += m_contentSize.width / 2.f;
        }
        position.y -= m_contentSize.height / 3.f;

        return { position, area };
    };
    auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
        body->setVelocity(this->getPhysicsBody()->getVelocity());
    };
    // create projectile - area where the chains aredealing damage during jump
    m_weapons[SWEEP_ATTACK]->LaunchAttack(
        std::move(projectilePosition), 
        std::move(pushProjectile)
    );
    // jump
    m_movement->ResetForce();
    m_movement->Push(IsLookingLeft()? -1.f: 1.f, 1.f);
}

// FIREBALLS
bool BanditBoss::CanLaunchFireballs() const noexcept {
    const auto player = this->getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    if(player 
        && !player->IsDead()
        && m_weapons[FIREBALL_ATTACK]->IsReady()
        && m_detectEnemy
    ) {
       return true;
    }

    return false;
}

// FIRECLOUD
bool BanditBoss::CanLaunchFirecloud() const noexcept {
    const auto player = this->getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    if(player 
        && !player->IsDead()
        && m_weapons[FIRECLOUD_ATTACK]->IsReady()
        && this->m_health <= MAX_HEALTH / 2
    ) {
        return true;
    }

    return false;
}

// JUMP + CHAINS
bool BanditBoss::CanLaunchSweepAttack() const noexcept {
    const auto player = this->getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    if(player 
        && !player->IsDead()
        && m_weapons[SWEEP_ATTACK]->IsReady()
        && this->IsOnGround() // can't jump in the air
    ) {
        cocos2d::Size aggroSize { m_contentSize.height * 2.f, m_contentSize.height };
        const cocos2d::Rect left { 
            this->getPosition() - cocos2d::Size {aggroSize.width, 0.f}, 
            aggroSize
        };
        const cocos2d::Rect right { 
            this->getPosition(), 
            aggroSize
        };

        const auto playerSize { player->getContentSize() };
        const cocos2d::Rect boundingBox {
            player->getPosition() - cocos2d::Vec2{ playerSize.width / 2.f, 0.f }, 
            playerSize
        };
        return (left.intersectsRect(boundingBox) || right.intersectsRect(boundingBox));
    }

    return false;
}

void BanditBoss::TryAttack() {
    const auto target = this->getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    const auto canBeInterrupted = (m_currentState == State::WALK || m_currentState == State::IDLE);
    if(target && !this->IsDead() && canBeInterrupted) {
        if(this->CanLaunchSweepAttack()) {
            // if player is in range and attack_3 can be performed -> perform jump attack
            this->LookAt(target->getPosition());
            this->MoveAlong(0.f, 0.f);
            this->LaunchSweepAttack();
        }
        else if(this->CanLaunchFireballs()) {
            // fireballs
            this->LookAt(target->getPosition());
            this->MoveAlong(0.f, 0.f);
            this->LaunchFireballs();
        }
        else if(this->CanLaunchFirecloud()) {
            this->LookAt(target->getPosition());
            this->MoveAlong(0.f, 0.f);
            this->LaunchFirecloud();
        }
        else if(m_detectEnemy) {
            const auto player = target;
            bool playerIsNear { false };
            if(player && !player->IsDead()) {
                const auto bossX = this->getPositionX();
                const auto playerX = player->getPositionX();
                playerIsNear = std::fabs(bossX - playerX) <= m_contentSize.width / 2.f;
            }

            /// Move towards player:
            if(!playerIsNear) {
                this->LookAt(target->getPosition());
                this->MoveAlong(this->IsLookingLeft()? -1.f: 1.f, 0.f);
            }
        }
        else {
            this->LookAt(target->getPosition());
            this->MoveAlong(0.f, 0.f);
        }
    }
}

static inline bool IsBusy(Weapon * weapon) noexcept {
    assert(weapon);
    return weapon->IsPreparing() || weapon->IsAttacking();
}

void BanditBoss::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    const auto velocity { this->IsDead()? cocos2d::Vec2{ 0.f, 0.f } : this->getPhysicsBody()->getVelocity() };
    static constexpr float EPS { 0.001f };

    if(m_health <= 0) {
        m_currentState = State::DEAD;
    } 
    else if(IsBusy(m_weapons[WeaponClass::FIREBALL_ATTACK])) {
        m_currentState = State::FIREBALL_ATTACK;
    }
    else if(IsBusy(m_weapons[WeaponClass::FIRECLOUD_ATTACK])) {
        m_currentState = State::FIRECLOUD_ATTACK;
    }
    else if(IsBusy(m_weapons[WeaponClass::SWEEP_ATTACK])) {
        m_currentState = State::SWEEP_ATTACK;
    }
    else if(helper::IsEqual(velocity.x, 0.f, EPS)) {
        m_currentState = State::IDLE;
    } 
    else {
        m_currentState = State::WALK;
    }

}

void BanditBoss::UpdatePosition(const float dt) noexcept {
    m_movement->Update(dt);
}

void BanditBoss::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        const auto isInfinity { m_currentState == State::IDLE || m_currentState == State::WALK };
        const auto repeatTimes { isInfinity ? dragonBones::Animator::INFINITY_LOOP: 1 };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(this->IsDead()) {
            this->OnDeath();
        }
    }
}

void BanditBoss::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this]() {
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
        Utils::CreateMask(core::CategoryBits::BOUNDARY, core::CategoryBits::PLATFORM )
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
        Utils::CreateMask(core::CategoryBits::BOUNDARY, core::CategoryBits::PLATFORM)
    );
}

void BanditBoss::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    std::string prefix = m_dragonBonesName + "/" + m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(chachedArmatureName));
    m_animator->setScale(0.2f); // TODO: introduce multi-resolution scaling
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::FIREBALL_ATTACK),     GetStateName(State::FIREBALL_ATTACK)),
        std::make_pair(Utils::EnumCast(State::FIRECLOUD_ATTACK),    GetStateName(State::FIRECLOUD_ATTACK)),
        std::make_pair(Utils::EnumCast(State::SWEEP_ATTACK),        GetStateName(State::SWEEP_ATTACK)),
        std::make_pair(Utils::EnumCast(State::IDLE),                GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::WALK),                "move"),
        std::make_pair(Utils::EnumCast(State::DEAD),                GetStateName(State::DEAD))
    });
    this->addChild(m_animator);
}

void BanditBoss::AddWeapons() {
    {
        const auto damage { 15.f };
        const auto range { 60.f };
        const auto animDuration = m_animator->GetDuration(Utils::EnumCast(State::FIREBALL_ATTACK));
        const auto attackDuration { 0.4f * animDuration };
        const auto preparationTime { animDuration - attackDuration };
        const auto reloadTime { 2.f };
        m_weapons[WeaponClass::FIREBALL_ATTACK] = new BossFireball(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
    {
        const auto damage { 0.f }; // doesn't matter
        const auto range { 130.f }; 
        const auto animDuration = m_animator->GetDuration(Utils::EnumCast(State::FIRECLOUD_ATTACK));
        const auto attackDuration { 0.4f * animDuration };
        const auto preparationTime { animDuration - attackDuration };
        const auto reloadTime { 5.f };
        m_weapons[WeaponClass::FIRECLOUD_ATTACK] = new BossFireCloud(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
    {
        const auto damage { 30.f }; // doesn't matter
        const auto range { 100.f }; 
        const auto animDuration = m_animator->GetDuration(Utils::EnumCast(State::SWEEP_ATTACK));
        const auto attackDuration { 0.4f * animDuration };
        const auto preparationTime { animDuration - attackDuration };
        const auto reloadTime { 2.f };
        m_weapons[WeaponClass::SWEEP_ATTACK] = new BossChain(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
}

} // namespace Enemies