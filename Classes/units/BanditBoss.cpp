#include "BanditBoss.hpp"
#include "Player.hpp"

#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Influence.hpp"
#include "../Movement.hpp"
#include "../Dash.hpp"
#include "../PhysicsHelper.hpp"

#include "../scenes/LevelScene.hpp"

#include "../configs/JsonUnits.hpp"

#include "cocos2d.h"

namespace {
/**
 * Can't introduce this function to the weapon class because 
 * can't define what is being busy mean for different units so it's 
 * using internal linkage
 */
inline bool IsBusy(const std::unique_ptr<Weapon>& weapon) noexcept {
    assert(weapon);
    return (weapon->IsPreparing() || weapon->IsAttacking());
}

} // namespace {

namespace Enemies {


BanditBoss* BanditBoss::create(size_t id
    , const cocos2d::Size& contentSize 
    , const json_models::BanditBoss *model) 
{
    auto pRet { new (std::nothrow) BanditBoss(id, contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

BanditBoss::BanditBoss(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::BanditBoss *model
)
    : Bot{ id, core::EntityNames::BOSS }
    , m_model { model }
{
    assert(model);
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width * 0.75f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
    m_health = m_model->health;
}


bool BanditBoss::init() {
    if (!Bot::init()) {
        return false; 
    }
    // Defines how high can the body jump
    // Note, in formula: G = -H / (2*t*t), G and t are already defined base on player
    // so changing `jumpHeight` will just tweak the result
    m_movement->SetJumpHeight(m_model->jumpHeight, LevelScene::GRAVITY);
    m_movement->SetMaxSpeed(m_model->defaultSpeed);

    const auto& dash = m_model->weapons.dash;
    m_dash = Dash::create(dash.cooldown
        , m_model->defaultSpeed
        , dash.velocity[0]);
    addComponent(m_dash);
    
    return true;
}

void BanditBoss::update(float dt) {
    cocos2d::Node::update(dt);
    // custom updates
    UpdateDebugLabel();
    if (!IsDead()) {
        UpdateWeapons(dt);
        UpdatePosition(dt); 
        TryAttack();
        UpdateCurses(dt);
    }
    UpdateState(dt);
    UpdateAnimation(); 
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
    assert(m_weapons[FIREBALL_ATTACK]->IsReady() 
        && !IsDead() 
        && "You don't check condition beforehand");

    m_weapons[FIREBALL_ATTACK]->LaunchAttack();
}

void BanditBoss::LaunchFirecloud() {
    assert(m_weapons[FIRECLOUD_ATTACK]->IsReady() 
        && !IsDead() 
        && "You don't check condition beforehand"
    );

    // start attack with weapon
    m_weapons[FIRECLOUD_ATTACK]->LaunchAttack();
}

void BanditBoss::LaunchSweepAttack() {
    assert(m_weapons[SWEEP_ATTACK]->IsReady() 
        && !IsDead() 
        && "You don't check condition beforehand"
    );
    
    // create projectile - area where the chains aredealing damage during jump
    m_weapons[SWEEP_ATTACK]->LaunchAttack();
    // jump
    using Move = Movement::Direction;
    
    Stop(Movement::Axis::XY);
    m_movement->Push(Move::UP);
    m_movement->Push(IsLookingLeft()? Move::LEFT: Move::RIGHT);
}

void BanditBoss::LaunchDash() {
    const auto duration { m_animator->GetDuration(Utils::EnumCast(State::DASH)) };
    m_dash->Initiate(duration);
}

// FIREBALLS
bool BanditBoss::CanLaunchFireballs() const noexcept {
    const auto player = getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    if (player 
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
    const auto player = getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    if (player 
        && !player->IsDead()
        && m_weapons[FIRECLOUD_ATTACK]->IsReady()
        && m_health <= m_model->health / 2
    ) {
        return true;
    }

    return false;
}

// JUMP + CHAINS
bool BanditBoss::CanLaunchSweepAttack() const noexcept {
    const auto player = getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    if (player 
        && !player->IsDead()
        && m_weapons[SWEEP_ATTACK]->IsReady()
        && IsOnGround() // can't jump in the air
    ) {
        cocos2d::Size aggroSize { m_contentSize.height * 2.f, m_contentSize.height };
        const cocos2d::Rect left { 
           getPosition() - cocos2d::Size {aggroSize.width, 0.f}, 
            aggroSize
        };
        const cocos2d::Rect right { getPosition(), aggroSize };

        const auto playerSize { player->getContentSize() };
        const cocos2d::Rect boundingBox {
            player->getPosition() - cocos2d::Vec2{ playerSize.width / 2.f, 0.f }, 
            playerSize
        };
        return (left.intersectsRect(boundingBox) || right.intersectsRect(boundingBox));
    }

    return false;
}

/**
 * Check whether DASH can be launched.
 * Consider:
 * 1. No cooldown
 * 2. Player exist and is alive (user make sure that it's true)
 * 3. health <= 50%? only after [finishing fire cloud call]
 * 4. health > 50%? player is quite far from the boss
 * 5. No other attacks performed
 */
bool BanditBoss::CanLaunchDash() const noexcept {
    bool canDash = !m_dash->IsOnCooldown() 
        && std::none_of(m_weapons.cbegin(), m_weapons.cend(), [](const std::unique_ptr<Weapon>& weapon) {
            return (weapon && IsBusy(weapon));
    });

    if (canDash) {
        // health <= 50%? only after [finishing fire cloud call]
        bool needDash = (m_health <= m_model->health / 2 
            && m_previousState == State::FIRECLOUD_ATTACK 
            && m_currentState != m_previousState
        );
        auto player = getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
        float bossX = getPositionX();
        float playerX = player->getPositionX();
        float duration { m_animator->GetDuration(Utils::EnumCast(State::DASH)) };
        float dashDistance = duration * m_model->weapons.dash.velocity[0];
        // health > 50%? player is quite far from the boss
        needDash = needDash || (m_health > m_model->health / 2
            && m_health <= static_cast<int>(m_model->health * 0.8f)
            && std::fabs(bossX - playerX) >= dashDistance * 0.75f // slice down the distance of the dash
        );
        return needDash;
    }
    return false;
}

void BanditBoss::LaunchBasicAttack() {
    assert(m_weapons[BASIC_ATTACK]->IsReady() 
        && !IsDead() 
        && "You don't check condition beforehand"
    );
    // create projectile - area where the chains are dealing damage
    m_weapons[BASIC_ATTACK]->LaunchAttack();
}

/**
 * Check whether BASIC_ATTACK can be launched.
 * Consider:
 * 1. Weapon is ready
 * 2. Sweep attack is on cd
 * 3. Player is close enough to attack him
 * 4. No other attacks performed
 */
bool BanditBoss::CanLaunchBasicAttack() const noexcept {
    const auto player =getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    assert(player && !IsDead() && "Expected to be called only satisfying those conditions");
    if (m_weapons[BASIC_ATTACK]->IsReady()) {
        const auto attackRange { m_weapons[BASIC_ATTACK]->GetRange()};
        const cocos2d::Rect aggroArea { 
           getPosition() - cocos2d::Vec2 { attackRange + m_contentSize.width / 2.f, 0.f }, 
            cocos2d::Size { 2 * attackRange + m_contentSize.width, m_contentSize.height } // aggro area size
        };
        const auto playerSize { player->getContentSize() };
        const cocos2d::Rect boundingBox {
            // substract half cuz it has anchor point bottom-middle
            player->getPosition() - cocos2d::Vec2{ playerSize.width / 2.f, 0.f }, 
            playerSize
        };
        return aggroArea.intersectsRect(boundingBox);
    }

    return false;
}

void BanditBoss::TryAttack() {
    assert(!IsDead());
    const auto target = getParent()->getChildByName<Unit*>(core::EntityNames::PLAYER);
    const auto canBeInterrupted = (m_currentState == State::WALK 
        || m_currentState == State::IDLE
        || m_currentState == State::BASIC_WALK
    );
    if (target && canBeInterrupted) {
        if (CanLaunchDash()) {
           LookAt(target->getPosition());
            Stop(Movement::Axis::XY);
           LaunchDash();
        }
        else if (CanLaunchSweepAttack()) {
            // if player is in range and SWEEP can be performed -> perform jump attack
           LookAt(target->getPosition());
            Stop(Movement::Axis::XY);
           LaunchSweepAttack();
        }
        else if (CanLaunchBasicAttack()) {
            // if player is in range and SWING can be performed -> invoke basic attack
           LookAt(target->getPosition());
            Stop(Movement::Axis::XY);
           LaunchBasicAttack();
        }
        else if (CanLaunchFireballs()) {
            // fireballs
           LookAt(target->getPosition());
            Stop(Movement::Axis::XY);
           LaunchFireballs();
        }
        else if (CanLaunchFirecloud()) {
           LookAt(target->getPosition());
            Stop(Movement::Axis::XY);
           LaunchFirecloud();
        }
        else if (m_detectEnemy) {
            const auto player = target;
            bool playerIsNear { false };
            if (!player->IsDead()) {
                const auto bossX =getPositionX();
                const auto playerX = player->getPositionX();
                playerIsNear = std::fabs(bossX - playerX) <= m_contentSize.width / 2.f;
            }

            /// Move towards player:
            if (!playerIsNear) {
               LookAt(target->getPosition());
                using Move = Movement::Direction;
                MoveAlong(IsLookingLeft()? Move::LEFT: Move::RIGHT);
            }
        }
        else {
           LookAt(target->getPosition());
            Stop(Movement::Axis::XY);
        }
    }
}

void BanditBoss::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    const auto velocity { IsDead()? cocos2d::Vec2{ 0.f, 0.f } : getPhysicsBody()->getVelocity() };
    static constexpr float EPS { 0.001f };

    if (m_health <= 0) {
        m_currentState = State::DEAD;
    } 
    else if (IsBusy(m_weapons[WeaponClass::FIREBALL_ATTACK])) {
        m_currentState = State::FIREBALL_ATTACK;
    }
    else if (IsBusy(m_weapons[WeaponClass::FIRECLOUD_ATTACK])) {
        m_currentState = State::FIRECLOUD_ATTACK;
    }
    else if (IsBusy(m_weapons[WeaponClass::SWEEP_ATTACK])) {
        m_currentState = State::SWEEP_ATTACK;
    }
    else if (IsBusy(m_weapons[WeaponClass::BASIC_ATTACK])) {
        m_currentState = State::BASIC_ATTACK;
    }
    else if (m_dash->IsRunning()) {
        m_currentState = State::DASH;
    }
    else if (helper::IsEqual(velocity.x, 0.f, EPS)) {
        m_currentState = State::IDLE;
    }
    else if (m_health > m_model->health / 2) {
        m_movement->SetMaxSpeed(m_model->defaultSpeed);
        m_currentState = State::BASIC_WALK;
    }
    else {
        m_movement->SetMaxSpeed(m_model->enhancedSpeed);
        m_currentState = State::WALK;
    }
}

void BanditBoss::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        bool isInfinity { 
            m_currentState == State::IDLE 
            || m_currentState == State::WALK 
            || m_currentState == State::BASIC_WALK 
        };
        const auto repeatTimes { isInfinity ? dragonBones::Animator::INFINITY_LOOP: 1 };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (IsDead()) {
           OnDeath();
        }
    }
}

void BanditBoss::OnDeath() {
    removeComponent(getPhysicsBody());
    getChildByName("health")->removeFromParent();
    m_animator->EndWith([this]() {
       runAction(cocos2d::RemoveSelf::create(true));
    });
}

void BanditBoss::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    const auto body { getPhysicsBody() };
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
    // TODO: introduce multi-resolution scaling
    m_animator->setScale(0.2f); 

    std::initializer_list<std::pair<size_t, std::string>> animations {
        std::make_pair(Utils::EnumCast(State::FIREBALL_ATTACK),     GetStateName(State::FIREBALL_ATTACK)),
        std::make_pair(Utils::EnumCast(State::FIRECLOUD_ATTACK),    GetStateName(State::FIRECLOUD_ATTACK)),
        std::make_pair(Utils::EnumCast(State::SWEEP_ATTACK),        GetStateName(State::SWEEP_ATTACK)),
        std::make_pair(Utils::EnumCast(State::BASIC_ATTACK),        GetStateName(State::BASIC_ATTACK)),
        std::make_pair(Utils::EnumCast(State::BASIC_WALK),          GetStateName(State::BASIC_WALK)),
        std::make_pair(Utils::EnumCast(State::DASH),                GetStateName(State::DASH)),
        std::make_pair(Utils::EnumCast(State::IDLE),                GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::WALK),                "move"),
        std::make_pair(Utils::EnumCast(State::DEAD),                GetStateName(State::DEAD))
    };
    m_animator->InitializeAnimations(animations);
    addChild(m_animator);
}

void BanditBoss::AddWeapons() {
    {
        const auto& firebook = m_model->weapons.firebook;
        float animDuration = m_animator->GetDuration(Utils::EnumCast(State::FIREBALL_ATTACK));
        float attackDuration { 0.4f * animDuration };
        float preparationTime { animDuration - attackDuration };

        auto genPos = [this]() -> cocos2d::Rect {
            const auto attackRange { m_weapons[FIREBALL_ATTACK]->GetRange() * 0.25f };

            auto position = getPosition();
            if (m_side == Side::RIGHT) {
                position.x += m_contentSize.width / 2.f;
            }
            else {
                position.x -= m_contentSize.width / 2.f + attackRange;
            }
            // shift a little bit higher to avoid immediate collision with the ground
            position.y += m_contentSize.height * 0.2f;
            cocos2d::Rect attackedArea { position, cocos2d::Size { 
                m_weapons[FIREBALL_ATTACK]->GetRange() * 2.f
                , m_contentSize.height * 1.05f } // a little bigger than the designed size
            };
            return attackedArea;
        };
        auto genVel = [this](cocos2d::PhysicsBody* body) {
            float xSpeed = m_model->weapons.firebook.projectile.velocity[0];
            body->setVelocity({ IsLookingLeft()? -xSpeed: xSpeed, 0.f });
        };

        auto& weapon = m_weapons[WeaponClass::FIREBALL_ATTACK];
        weapon.reset(new BossFireball(firebook.projectile.damage
            , firebook.range
            , preparationTime
            , attackDuration
            , firebook.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
    {
        const auto& firecloud = m_model->weapons.firecloud;
        float animDuration = m_animator->GetDuration(Utils::EnumCast(State::FIRECLOUD_ATTACK));
        float attackDuration { 0.4f * animDuration };
        float preparationTime { animDuration - attackDuration };

        auto genPos = [this]() -> cocos2d::Rect {
            auto attackRange { m_weapons[FIRECLOUD_ATTACK]->GetRange()};
            auto position = getPosition();
            position.y += m_contentSize.height;
            return { position, cocos2d::Size{ attackRange, m_contentSize.height * 0.35f } };
        };
        auto genVel = [dx = firecloud.impulse[0], dy = firecloud.impulse[1]](cocos2d::PhysicsBody* body) {
            cocos2d::Vec2 impulse { dx, body->getMass() * dy };
            body->applyImpulse(impulse);
        };

        auto& weapon = m_weapons[WeaponClass::FIRECLOUD_ATTACK];
        weapon.reset(new BossFireCloud(
            0.f, firecloud.range, preparationTime, attackDuration, firecloud.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
    {
        const auto& chainSweeper = m_model->weapons.chainSweep;
        float animDuration = m_animator->GetDuration(Utils::EnumCast(State::SWEEP_ATTACK));
        float attackDuration { 0.4f * animDuration };
        float preparationTime { animDuration - attackDuration };
        
        auto genPos = [this]() -> cocos2d::Rect {
            auto attackRange { m_weapons[SWEEP_ATTACK]->GetRange()};
            cocos2d::Size area { attackRange, attackRange };

            auto position = getPosition();
            if (IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f + area.width;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y -= m_contentSize.height / 3.f;

            return { position, area };
        };
        auto genVel = [this](cocos2d::PhysicsBody* body) {
            body->setVelocity(getPhysicsBody()->getVelocity());
        };
        
        auto& weapon = m_weapons[WeaponClass::SWEEP_ATTACK];
        weapon.reset(new BossChainSweep(chainSweeper.projectile.damage
            , chainSweeper.range
            , preparationTime
            , attackDuration
            , chainSweeper.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
    {
        const auto& chainSwing = m_model->weapons.chainSwing;
        float animDuration = m_animator->GetDuration(Utils::EnumCast(State::BASIC_ATTACK));
        float attackDuration { 0.8f * animDuration };
        float preparationTime { animDuration - attackDuration };

        auto genPos = [this]() -> cocos2d::Rect {
            auto attackRange { m_weapons[BASIC_ATTACK]->GetRange()};
            cocos2d::Size area { attackRange, attackRange / 4.f };

            auto position = getPosition();
            if (IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f + area.width;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += m_contentSize.height / 4.f;

            return { position, area };
        };
        auto genVel = [this](cocos2d::PhysicsBody* body) {
            body->setVelocity(getPhysicsBody()->getVelocity());
        };

        auto& weapon = m_weapons[WeaponClass::BASIC_ATTACK];
        weapon.reset(new BossChainSwing(chainSwing.projectile.damage
            , chainSwing.range
            , preparationTime
            , attackDuration
            , chainSwing.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
}

} // namespace Enemies