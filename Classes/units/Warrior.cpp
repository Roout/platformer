#include "Warrior.hpp"
#include "Player.hpp"

#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Weapon.hpp"
#include "../Influence.hpp"
#include "../Movement.hpp"

#include "cocos2d.h"

namespace Enemies {

Warrior::Warrior(size_t id
    , const char* dragonBonesName
    , const cocos2d::Size& contentSize
)
    : Bot{ id, dragonBonesName }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width * 0.75f, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}


bool Warrior::init() {
    if (!Bot::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(80.f);
    return true;
}

void Warrior::update(float dt) {
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

/// Unique to warrior
void Warrior::AttachNavigator(Path&& path) {
    // process a default paths waypoints to avoid jumping:
    // align them with own Y-axis position
    for (auto& point: path.m_waypoints) {
        point.y = getPosition().y;
    }
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    Patrol();
}

void Warrior::Pursue(Unit * target) noexcept {
    if(!IsDead() && target && !target->IsDead()) {
        const auto shift { floorf(
            target->GetHitBox().width / 2.f // shift to bottom left\right corner
            + m_weapons[WeaponClass::MELEE]->GetRange() / 2.f // shift by weapon length (not 1.0f to be able to reach the target by attack!)
            + m_contentSize.width / 2.f     // shift by size where the weapon's attack will be created
        )};
        // possible destinations (bottom middle of this unit)
        const float xTargets[2] = { 
            target->getPositionX() + shift,
            target->getPositionX() - shift,
        };
        // choose the closest target in out influence field!
        const float xDistances[2] = {
            fabs(xTargets[0] - getPositionX()),
            fabs(xTargets[1] - getPositionX())
        };
        const float xShift { floorf(m_physicsBodySize.width * 0.4f) };
        // -xShift for left-bottom corner of this unit 
        // +xShift for right-bottom corner of this unit 
        // So if influence contains either of these corners than the unit won't fall down for sure!
        const bool acceptable[2] = {
            m_influence->ContainsX(xTargets[0] - xShift) || 
            m_influence->ContainsX(xTargets[0] + xShift),

            m_influence->ContainsX(xTargets[1] - xShift) || 
            m_influence->ContainsX(xTargets[1] + xShift)
        };
        // find the closest point from the ones where unit won't fall down
        int choosenIndex { -1 };
        float xDistance { xDistances[0] };
        for (int i = 0; i < 2; i++) {
            if (acceptable[i] && xDistances[i] <= xDistance) {
                choosenIndex = i;
                xDistance = xDistances[i];
            }
        }
        auto destination = getPosition();
        if (choosenIndex != -1) { // every target lead to falling down or other shit
            destination.x = xTargets[choosenIndex];
        }
        // move to target along X-axis;
        // Pass own Y-axis coordinate to not move along Y-axis
        m_navigator->VisitCustomPoint(destination);
    }
}

void Warrior::Patrol() noexcept {
    m_navigator->FollowPath();
}

/// Bot interface
void Warrior::OnEnemyIntrusion() {
    m_detectEnemy = true;
    auto target = getParent()->getChildByName<Unit *>(core::EntityNames::PLAYER);
    Pursue(target);
}

void Warrior::OnEnemyLeave() {
    m_detectEnemy = false;
    Patrol();
}

void Warrior::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_health <= 0) {
        m_currentState = State::DEAD;
    } 
    else if (m_weapons[WeaponClass::MELEE]->IsPreparing()) {
        m_currentState = State::ATTACK;
    }
    else if (m_weapons[WeaponClass::MELEE]->IsAttacking()) {
        m_currentState = State::ATTACK;
    }
    else if (m_weapons[WeaponClass::MELEE]->IsReloading()) {
        m_currentState = State::IDLE;
    }
    else if (m_detectEnemy) {
        m_currentState = State::PURSUIT;
    }
    else {
        m_currentState = State::PATROL;
    }
}

void Warrior::UpdatePosition(const float dt) noexcept {
    assert(!IsDead());

    auto initiateAttack { 
        m_weapons[WeaponClass::MELEE]->IsAttacking() || 
        m_weapons[WeaponClass::MELEE]->IsPreparing() 
    };
    if (m_weapons[WeaponClass::MELEE]->IsReloading()) {
        // stop
        Stop(Movement::Axis::XY);
    }
    else if (!initiateAttack) {
        if (m_detectEnemy) { // update target
            auto target = getParent()->getChildByName<Unit *>(core::EntityNames::PLAYER);
            Pursue(target);
        }
        // update
        m_navigator->Update(dt);
        m_movement->Update();
    }
}

void Warrior::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        const auto isOneTimeAttack { 
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

void Warrior::OnDeath() {
    removeComponent(getPhysicsBody());
    getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Warrior::AddPhysicsBody() {
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

void Warrior::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::IDLE),    GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::PURSUIT), GetStateName(State::PURSUIT)),
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEAD),    GetStateName(State::DEAD))
    });
}

} // namespace Enemies