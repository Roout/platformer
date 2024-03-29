#include "BoulderPusher.hpp"
#include "Player.hpp"
#include "Core.hpp"

#include "components/DragonBonesAnimator.hpp"
#include "components/Weapon.hpp"
#include "components/Movement.hpp"

#include "configs/JsonUnits.hpp"

#include "cocos2d.h"

namespace Enemies {

BoulderPusher* BoulderPusher::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::BoulderPusher *model
) {
    auto pRet { new (std::nothrow) BoulderPusher(id, contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

BoulderPusher::BoulderPusher(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::BoulderPusher *model
)
    : Bot{ id, core::EntityNames::BOULDER_PUSHER }
    , m_model { model }
{
    assert(model);
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size{ contentSize.width * 0.75f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
    m_health = m_model->health;
}

bool BoulderPusher::init() {
    if (!Bot::init()) {
        return false; 
    }
    return true;
}

void BoulderPusher::update(float dt) {
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

void BoulderPusher::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void BoulderPusher::OnEnemyLeave() {
    m_detectEnemy = false;
}

void BoulderPusher::UpdateState(const float dt) noexcept {
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

void BoulderPusher::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        auto isOneTimeAttack { 
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

void BoulderPusher::OnDeath() {
    removeComponent(getPhysicsBody());
    getChildByName("health")->removeFromParent();
    m_animator->EndWith([this]() {
        runAction(cocos2d::RemoveSelf::create(true));
    });
}

void BoulderPusher::AddPhysicsBody() {
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

void BoulderPusher::AddAnimator() {
    Unit::AddAnimator();
    m_animator->setScale(0.15f); // override scale
    m_animator->InitializeAnimations({
        /// TODO: mismatch, update animation!
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::ATTACK),          GetStateName(State::IDLE)), 
        std::make_pair(Utils::EnumCast(State::IDLE),            GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::DEAD),            GetStateName(State::DEAD))
    });
}

void BoulderPusher::AddWeapons() {
    const auto& legs = m_model->weapons.legs;
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    const auto preparationTime { 0.4f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) - preparationTime };

    auto genPos = [this]() -> cocos2d::Rect {
        auto radius { m_weapons[WeaponClass::RANGE]->GetRange() };
        cocos2d::Size stoneSize { radius * 2.f, radius * 2.f };

        auto position = getPosition();
        if (IsLookingLeft()) {
            position.x -= m_contentSize.width / 2.f + stoneSize.width;
        }
        else {
            position.x += m_contentSize.width / 2.f;
        }
        return { position, stoneSize };
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        const auto& legs = m_model->weapons.legs;
        cocos2d::Vec2 impulse { body->getMass() * legs.projectile.impulse[0], 0.f };
        if (IsLookingLeft()) {
            impulse.x *= -1.f;
        }
        body->applyImpulse(impulse);
        body->setAngularVelocity(impulse.x > 0.f? -legs.projectile.angular[0]: legs.projectile.angular[0]);
    };

    auto& weapon = m_weapons[WeaponClass::RANGE];
    weapon.reset(new Legs(legs.projectile.damage
        , legs.range
        , preparationTime
        , attackDuration
        , legs.cooldown));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}

bool BoulderPusher::NeedAttack() const noexcept {
    assert(!IsDead());
    return m_detectEnemy && m_weapons[WeaponClass::RANGE]->IsReady();
}

void BoulderPusher::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::RANGE]->IsReady());

    m_weapons[WeaponClass::RANGE]->LaunchAttack();
}

} // namespace Enemies