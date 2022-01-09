#include "Slime.hpp"

#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"
#include "Movement.hpp"

#include "configs/JsonUnits.hpp"

#include <memory>

namespace Enemies {


Slime* Slime::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Slime *slime) 
{
    auto pRet { new (std::nothrow) Slime(id, contentSize, slime) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Slime::init() {
    if (!Bot::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(m_slime->maxSpeed);
    m_health = m_slime->health;
    return true;
}

Slime::Slime(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Slime *slime
)
    : Bot { id, core::EntityNames::SLIME }
    , m_slime { slime }
{
    m_contentSize = contentSize;
    m_physicsBodySize = contentSize;
    m_hitBoxSize = contentSize;
}

void Slime::update(float dt) {
    // update components
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

void Slime::AttachNavigator(Path&& path) {
    // process a default paths waypoints to avoid jumping:
    // align them with own Y-axis position
    for (auto& point: path.m_waypoints) {
        point.y = getPosition().y;
    }
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    Patrol();
}

void Slime::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void Slime::OnEnemyLeave() {
    m_detectEnemy = false;
    Patrol();
}

void Slime::Patrol() noexcept {
    m_navigator->FollowPath();
}

void Slime::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsPreparing()) {
        /// TODO: fix this approach which leads to misundestanding
        /// This force to continue the animation which was played before 
        /// Needed to choose the time when the projectile will be created
        m_currentState = State::ATTACK;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::ATTACK;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsReloading()) {
        m_currentState = State::IDLE;
    }
    else {
        m_currentState = State::PATROL;
    }
}

void Slime::UpdatePosition(const float dt) noexcept {
    assert(!IsDead());
    if (!m_detectEnemy) {
        // update
        m_navigator->Update(dt);
        m_movement->Update();
    }
    else {
        Stop(Movement::Axis::XY);
    }
}

void Slime::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        const auto isOneTimeAttack { 
            m_currentState == State::PREPARE_ATTACK ||
            m_currentState == State::ATTACK || 
            m_currentState == State::DEAD
        };
        const auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (IsDead()) {
            OnDeath();
        }
    }
}

void Slime::OnDeath() {
    removeComponent(getPhysicsBody());
    getChildByName("health")->removeFromParent();
    m_animator->EndWith([this]() {
        runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Slime::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    const auto body { getPhysicsBody() };
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

bool Slime::NeedAttack() const noexcept {
    assert(!IsDead());
    return m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

void Slime::AddAnimator() {
    Unit::AddAnimator();
    m_animator->setScale(0.35f); // TODO: introduce multi-resolution scaling
    m_animator->InitializeAnimations({
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::ATTACK),  GetStateName(State::ATTACK)), 
        std::make_pair(Utils::EnumCast(State::IDLE),    GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEAD),    GetStateName(State::DEAD))
    });
}

// =============  WEAPON STUFF ================== //

void Slime::AddWeapons() {
    
    const auto duration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto preparationTime { duration * 0.6f }; /// TODO: update animation!
    const auto attackDuration { duration - preparationTime };
    
    auto genPos = [this]() -> cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
        cocos2d::Size waterballSize { attackRange, floorf(attackRange * 0.8f) };

        auto position = getPosition();
        if (IsLookingLeft()) {
            position.x -= m_contentSize.width / 2.f ;
        }
        else {
            position.x += m_contentSize.width / 2.f;
        }
        position.y += floorf(m_contentSize.height * 0.3f);

        return { position, waterballSize };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        const auto& velocity = m_slime->weapons.spell.projectile.velocity;
        body->setVelocity({ IsLookingLeft()? -velocity[0]: velocity[0], velocity[1] });
    };

    const auto& spell = m_slime->weapons.spell;
    auto& weapon = m_weapons[WeaponClass::RANGE];
    weapon.reset(new SlimeShot(spell.projectile.damage
        , spell.range
        , preparationTime
        , attackDuration
        , spell.cooldown));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}

void Slime::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::RANGE]->IsReady());

    m_weapons[WeaponClass::RANGE]->LaunchAttack();
}

}// namespace Enemies