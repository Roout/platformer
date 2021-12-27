#include "Archer.hpp"

#include "Player.hpp"
#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Movement.hpp"
#include "../configs/JsonUnits.hpp"

#include "cocos2d.h"

namespace Enemies {

Archer* Archer::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Archer * model) 
{
    auto pRet { new (std::nothrow) Archer(id, contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Archer::Archer(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Archer * model
)
    : Bot{ id, core::EntityNames::ARCHER }
    , m_model { model }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width * 0.875f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}

bool Archer::init() {
    if (!Bot::init() ) {
        return false; 
    }
    return true;
}

void Archer::update(float dt) {
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

/// Bot interface
void Archer::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void Archer::OnEnemyLeave() {
    m_detectEnemy = false;
}

void Archer::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsPreparing()) {
        m_currentState = State::PREPARE_ATTACK;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::ATTACK;
    }
    else {
        m_currentState = State::IDLE;
    }
}

void Archer::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        auto isOneTimeAttack { 
            m_currentState == State::PREPARE_ATTACK || 
            m_currentState == State::ATTACK || 
            m_currentState == State::DEAD
        };
        auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (IsDead()) {
            OnDeath();
        }
    }
}

void Archer::OnDeath() {
    removeComponent(getPhysicsBody());
    getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Archer::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    const auto body { getPhysicsBody() };
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
    const auto& bow = m_model->weapons.bow;
    float damage { bow.projectile.damage };
    float range { bow.range };
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    float preparationTime { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) }; /// TODO: update animation!
    float attackDuration { 0.1f };
    float reloadTime { bow.cooldown };
    
    auto genPos = [this]()->cocos2d::Rect {
        float attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
        cocos2d::Size arrowSize { attackRange, floorf(attackRange / 8.5f) };

        auto position = getPosition();
        if (IsLookingLeft()) {
            position.x -= m_contentSize.width / 2.f + arrowSize.width;
        }
        else {
            position.x += m_contentSize.width / 2.f;
        }
        position.y += m_contentSize.height / 2.f;

        return { position, arrowSize };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        const auto& velocity = m_model->weapons.bow.projectile.velocity;
        body->setVelocity({ IsLookingLeft()? -velocity[0]: velocity[0], velocity[1] });
    };

    auto& weapon = m_weapons[WeaponClass::RANGE];
    weapon.reset(new Bow(
        damage, range, preparationTime, attackDuration, reloadTime));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}

void Archer::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::RANGE]->IsReady());

    m_weapons[WeaponClass::RANGE]->LaunchAttack();
}

bool Archer::NeedAttack() const noexcept {
    return !IsDead() && m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

} // namespace Enemies